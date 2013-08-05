#include "processing.h"

/*
* Processes RX in a specific port. The function will process up to MAX_BURST_SIZE 
*/
inline static process_port_rx(switch_port_t* port, datapacket_t* pkt, dpdk_pkt_platform_state_t* pstate, datapacketx86* pkt_x86){

	unsigned int i, burst_len;
	of_switch_t* sw = port->attached_sw;
	struct rte_mbuf* mbuf;

	//Read a burst
	//XXX	

	//Process them 
	for(i=0;i<burst_len;<++i){
		if(unlikely(sw == NULL))
			continue;

		//set mbuf pointer
		pstate->rte_buf = mbuf;
	
		//Parse	
		pkt_x86->init(X86_DATAPACKET_BUFFERED_IN_NIC, mbuf->buf_addr, mbuf->buf_len);
		pkt_x86->classify();

		//Send to process
		of_process_packets_pipeline(sw, pkt);
	}	
}

void process_packets_port(core_task_list_t* task_list){

	unsigned int i;
	switch_port_t* port;
	bool* keep_on = &task_list->keep_on;

	//Parsing and pipeline extra state
	datapacket_t pkt = datapacket_t;
	datapacketx86 pkt_x86();
	dpdk_port_platform_state_t pstate;

	//Init values and assign
	pstate->datapacket = &pkt;
	pkt->platform_state = (platform_port_state_t*)&pstate;

	while(likely(*keep_on)){
	
		for(i=0;i<task->num_of_ports;++i){
			port = task_list->port_list[i];
			if(likely(port)){
				//Process RX&pipeline 
				process_port_rx(port, &pkt, &pstate, &pkt_x86);

				//Process TX
				//XXX
			}
		}
	}
	
	
}


