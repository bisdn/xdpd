#include "processing.h"
#include <rofl/common/utils/c_logger.h>
#include <rte_spinlock.h>

#include "assert.h"
#include "../util/compiler_assert.h"

#include "../io/port_state.h"
#include <rofl/datapath/pipeline/openflow/of_switch.h>


//
// Processing state 
//
static unsigned int current_core_index;
static unsigned int max_cores;
static rte_spinlock_t mutex;

/*
* Initialize data structures for processing to work 
*/
rofl_result_t processing_init(void){

	unsigned int i;
	struct rte_config* config;
	enum rte_lcore_role_t role;

	//Cleanup
	memset(core_tasks,0,sizeof(core_tasks_t)*RTE_MAX_LCORE);	

	//Init 
	current_core_index = 0;
	config = rte_eal_get_configuration();
	max_cores = config->lcore_count;
	rte_spinlock_init(&mutex);

	//Define available cores 
	for(i=0; i < max_cores; ++i){
		role = rte_eal_lcore_role(i);
		if(role == ROLE_RTE)
			core_tasks[i].available = true;
	}

	return ROFL_SUCCESS;
}


/*
* Destroy data structures for processing to work 
*/
rofl_result_t processing_destroy(void){

	unsigned int i;

	//All of them to stop
	for(i=0;i<RTE_MAX_LCORE;++i)
		core_tasks[i].stop = true;

	//Join all threads
	
	return ROFL_SUCCESS;
}



//
// Packet processing
//

/*
* Processes RX in a specific port. The function will process up to MAX_BURST_SIZE 
*/
inline static void process_port_rx(switch_port_t* port, struct rte_mbuf** pkts_burst, datapacket_t* pkt, dpdk_pkt_platform_state_t* pkt_state, datapacketx86* pkt_x86){
	
	unsigned int i, burst_len;
	of_switch_t* sw = port->attached_sw;
	struct rte_mbuf* mbuf;
	dpdk_port_state_t* port_state = (dpdk_port_state_t*)port->platform_port_state;

	//Read a burst
	burst_len = rte_eth_rx_burst(port_state->port_id, 0, pkts_burst, IO_IFACE_MAX_PKT_BURST);

	//Process them 
	for(i=0;i<burst_len;++i){
		mbuf = pkts_burst[i];		

		if(unlikely(sw == NULL)){
			rte_pktmbuf_free(mbuf);
			continue;
		}

		//set mbuf pointer in the state so that it can be recovered afterwards when going
		//out from the pipeline
		pkt_state->mbuf = mbuf;

		//XXX: delete from here
		assert(mbuf->pkt.nb_segs == 1);

		//Init&classify	
		pkt_x86->init((uint8_t*)mbuf->buf_addr, mbuf->buf_len, sw, mbuf->pkt.in_port, 0, true, false);

		//Send to process
		of_process_packet_pipeline(sw, pkt);
	}	
}

inline static void process_port_tx(switch_port_t* port){
	//XXX	
}

void processing_process_packets_core(){

	unsigned int i;
	switch_port_t* port;
	struct rte_mbuf* pkts_burst[IO_IFACE_MAX_PKT_BURST];
	core_tasks_t* tasks = &core_tasks[rte_lcore_id()];

	//Parsing and pipeline extra state
	datapacket_t pkt;
	datapacketx86 pkt_x86;
	dpdk_pkt_platform_state_t pkt_state;

	//Init values and assign
	pkt_state.pkt_x86 = &pkt_x86;
	pkt.platform_state = (platform_port_state_t*)&pkt_state;

	//Set flag to active
	tasks->active = true;

	while(likely(tasks->stop == false)){
	
		for(i=0;i<tasks->num_of_ports;++i){
			port = tasks->port_list[i];
			if(likely(port != NULL)){ //This CAN happen while deschedulings
				//Process RX&pipeline 
				process_port_rx(port, pkts_burst, &pkt, &pkt_state, &pkt_x86);

				//Process TX
				process_port_tx(port);
			}
		}
	}
	
	tasks->active = false;
 
}


//
//Port scheduling
//

/*
* Schedule port. Shedule port to an available core (RR)
*/
rofl_result_t processing_schedule_port(switch_port_t* port){

	unsigned int initial_index, *num_of_ports;
	dpdk_port_state_t* port_state = (dpdk_port_state_t*)port->platform_port_state;	

	rte_spinlock_lock(&mutex);

	initial_index = current_core_index;

	do{
		current_core_index++;
		if(current_core_index == max_cores)
			current_core_index = 0; 

		
		if(current_core_index == initial_index+1){
			//All full 
			ROFL_ERR("All cores are full. No available port slots\n");
			assert(0);		
			return ROFL_FAILURE;
		}
	}while( core_tasks[current_core_index].available == false || core_tasks[current_core_index].num_of_ports == MAX_PORTS_PER_CORE);

	num_of_ports = &core_tasks[current_core_index].num_of_ports;

	//Assign port and exit
	if(core_tasks[current_core_index].port_list[*num_of_ports] != NULL){
		ROFL_ERR("Corrupted state on the core task list\n");
		assert(0);
		rte_spinlock_unlock(&mutex);
		return ROFL_FAILURE;
	}

	//Store attachment info (back reference)
	port_state->core_id = current_core_index; 
	port_state->core_port_slot = *num_of_ports;
	
	core_tasks[current_core_index].port_list[*num_of_ports] = port;
	(*num_of_ports)++;
	
	rte_spinlock_unlock(&mutex);
	
	port_state->scheduled = true;
		
	return ROFL_SUCCESS;
}

/*
* Deschedule port to a core 
*/
rofl_result_t processing_deschedule_port(switch_port_t* port){

	unsigned int i;
	dpdk_port_state_t* port_state = (dpdk_port_state_t*)port->platform_port_state;	
	core_tasks_t* core_task = &core_tasks[port_state->core_id];

	if(port_state->scheduled == false){
		ROFL_ERR("Tyring to descheduled an unscheduled port\n");
		assert(0);
		return ROFL_FAILURE;
	}

	rte_spinlock_lock(&mutex);

	//This loop copies from descheduled port, all the rest of the ports
	//one up, so that list of ports is contiguous (0...N-1)

	for(i=(core_task->num_of_ports-1); i > port_state->core_port_slot; i--)
		core_task->port_list[i-1] = core_task->port_list[i];	
	
	//Cleanup the last position
	core_task->num_of_ports--;
	core_task->port_list[core_task->num_of_ports] = NULL;
	
	rte_spinlock_unlock(&mutex);	
	
	port_state->scheduled = false;

	return ROFL_SUCCESS;
}



