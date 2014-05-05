#include "tx.h"

//Make sure pipeline-imp are BEFORE _pp.h
//so that functions can be inlined
#include "../pipeline-imp/packet.h"
#include "../pipeline-imp/rte_atomic_operations.h"
#include "../pipeline-imp/lock.h"

//Now include pp headers
#include <rofl/datapath/pipeline/openflow/of_switch_pp.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline_pp.h>



void xdpd::gnu_linux_dpdk::tx_pkt_vlink(switch_port_t* vlink, datapacket_t* pkt){
	switch_port_t* vlink_pair = (switch_port_t*)vlink->platform_port_state;
	of_switch_t* sw;

	assert(vlink->type == PORT_TYPE_VIRTUAL);
	
	if( likely( vlink_pair!= NULL) ){
		sw = vlink_pair->attached_sw;
		if( likely(sw != NULL) ){	
			of_process_packet_pipeline(sw, pkt);
			return;
		}
	}

	//The vlink is being destroyed; drop the packet
	platform_packet_drop(pkt); //!!! circular dependency
}




