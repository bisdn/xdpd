#include "pktin_dispatcher.h"

#include <unistd.h>
#include <fcntl.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_async_events_hooks.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_table.h>
#include <rofl/datapath/hal/openflow/openflow1x/of1x_cmm.h>
#include <rofl/datapath/pipeline/platform/packet.h>
#include <rofl/common/utils/c_logger.h>

#include "../config.h"
#include "../io/bufferpool.h"
#include "../io/datapacketx86.h"
#include "../io/datapacket_storage.h"
#include "../processing/ls_internal_state.h"

using namespace xdpd::gnu_linux;

#define BUCKETS_PER_LS 16
#define DUMMY_BUF_SIZE 128
#define ITERATIONS_PER_ROUND 2

int pktin_not_pipe[2];

//Processes the pkt_ins up to BUCKETS_PER_LS 
static inline void process_sw_of1x_packet_ins(of1x_switch_t* sw){

	int ret;
	unsigned int i;
	datapacket_t* pkt;
	datapacketx86* pkt_x86;
	hal_result_t rv;
	storeid id;
	static char null_buf[DUMMY_BUF_SIZE];
	static unsigned int pending_to_read=0;
	packet_matches_t matches;
	
	//Recover platform state
	switch_platform_state_t* ls_int = (switch_platform_state_t*)sw->platform_state;

	//Try to process up to BUCKETS_PER_LS packet-ins
	for(i=0;i<BUCKETS_PER_LS;++i){
	
		//Recover packet	
		pkt = ls_int->pkt_in_queue->non_blocking_read();
	
		if(!pkt)
			break;
		
		//Recover platform state
		pkt_x86 = (datapacketx86*)pkt->platform_state;
		
		//Store packet in the storage system. Packet is NOT returned to the bufferpool
		id = ls_int->storage->store_packet(pkt);

		if(id == datapacket_storage::ERROR){
			ROFL_DEBUG(DRIVER_NAME"[pkt-in-dispatcher] PKT_IN for packet(%p) could not be stored in the storage. Dropping..\n",pkt);
	
			//Return to the bufferpool
			bufferpool::release_buffer(pkt);
			continue;
		}

		//Fill matches
		fill_packet_matches(pkt, &matches);
		//Process packet in
        	rv = hal_cmm_process_of1x_packet_in(sw->dpid, 
						pkt_x86->pktin_table_id, 	
						pkt_x86->pktin_reason, 	
						pkt_x86->in_port, 
						id, 	
						pkt_x86->get_buffer(), 
						pkt_x86->pktin_send_len,
						pkt_x86->get_buffer_length(),
						&matches
						);

		if( unlikely(rv != HAL_SUCCESS) ){
			ROFL_DEBUG(DRIVER_NAME"[pkt-in-dispatcher] PKT_IN for packet(%p) could not be sent to sw:%s controller. Dropping..\n",pkt,sw->name);
			//Take packet out from the storage
			if( unlikely(ls_int->storage->get_packet(id) != pkt) ){
				ROFL_ERR(DRIVER_NAME"[pkt-in-dispatcher] Storage corruption. get_packet(%u) returned a different pkt pointer (should have been %p)\n", id, pkt);

				assert(0);
			}

			//Return to datapacket_storage
			ls_int->storage->get_packet(id);

			//Return to the bufferpool
			bufferpool::release_buffer(pkt);
		}
		
	}

	//Empty pipe
	pending_to_read += i;

	if( pending_to_read > 0 ){

		if(pending_to_read > DUMMY_BUF_SIZE){
			ret = read(pktin_not_pipe[PKT_IN_PIPE_READ], &null_buf, DUMMY_BUF_SIZE);
		}else{
			ret = read(pktin_not_pipe[PKT_IN_PIPE_READ], &null_buf, pending_to_read);
		}

		if(ret > 0)
			pending_to_read -= ret; 
	}
}

//Initialize pkt in notification pipe

int init_packetin_pipe(){
	
	//Open pipe for output signaling on enqueue	
	int rc = pipe(pktin_not_pipe);
	(void)rc; // todo use the value

	//Set non-blocking read/write in the pipe
	for(unsigned int i=0;i<2;i++){
		int flags = fcntl(pktin_not_pipe[i], F_GETFL, 0);	///get current file status flags
		flags |= O_NONBLOCK;					//turn off blocking flag
		fcntl(pktin_not_pipe[i], F_SETFL, flags);		//set up non-blocking read
	}
	
	return pktin_not_pipe[PKT_IN_PIPE_READ];
}

void destroy_packetin_pipe(){
	close(pktin_not_pipe[PKT_IN_PIPE_WRITE]);
	close(pktin_not_pipe[PKT_IN_PIPE_READ]);
}

/*
* Attempt to process packet ins for all the switches
*/
void process_packet_ins(){

	unsigned int i, j, r, max_switches;
	of_switch_t** logical_switches;
	
	//Retrieve the logical switches list
	logical_switches = physical_switch_get_logical_switches(&max_switches);
	
	for(i=0;i<ITERATIONS_PER_ROUND;++i){
		for(j=0;j<ITERATIONS_PER_ROUND;++j){
			for(r=0; r<max_switches; ++r){
				if(logical_switches[r] != NULL){
					//if( logical_switches[r]->of_ver == OF_VERSION_12 )
					process_sw_of1x_packet_ins((of1x_switch_t*)logical_switches[r]);
				}
			}
		}
	}
}


void drain_packet_ins(of_switch_t* sw){
	
	datapacket_t* pkt;
	switch_platform_state_t* ls_int;
 
	if(!sw)
		assert(0);
		
	//Recover platform state
	ls_int = (switch_platform_state_t*)sw->platform_state;

	//Let the pending pkt_ins to be drained
	while(ls_int->pkt_in_queue->size() != 0){

		pkt = ls_int->pkt_in_queue->non_blocking_read();
	
		//Return to the bufferpool
		if(pkt)
			bufferpool::release_buffer(pkt);
	}
}
