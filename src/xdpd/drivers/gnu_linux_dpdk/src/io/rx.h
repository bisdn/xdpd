/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _RX_H_
#define _RX_H_

#include "../config.h"
#include <rofl/common/utils/c_logger.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_spinlock.h>
#include <rte_eal.h> 
#include <rte_mbuf.h> 
#include <rte_ethdev.h> 

#include "assert.h"
#include "dpdk_datapacket.h"

#include "port_state.h"
#include "iface_manager.h"
#include "../processing/processing.h"

//Make sure pipeline-imp are BEFORE _pp.h
//so that functions can be inlined
#include "../pipeline-imp/rte_atomic_operations.h"
#include "../pipeline-imp/lock.h"
#include "../pipeline-imp/packet.h"

//Now include pp headers
#include <rofl/datapath/pipeline/openflow/of_switch_pp.h>

namespace xdpd {
namespace gnu_linux_dpdk {

//
// Packet processing
//


/*
* Processes RX in a specific port. The function will process up to MAX_BURST_SIZE 
*/
inline void
process_port_rx(switch_port_t* port, unsigned int port_id, struct rte_mbuf** pkts_burst, datapacket_t* pkt, datapacket_dpdk_t* pkt_state){
	
	unsigned int i, burst_len;
	of_switch_t* sw = port->attached_sw;
	struct rte_mbuf* mbuf;
	datapacket_dpdk_t* pkt_dpdk = pkt_state;

	if(unlikely(port->drop_received)) //Ignore if port is marked as "drop received"
		return;

	//Read a burst
	burst_len = rte_eth_rx_burst(port_id, 0, pkts_burst, IO_IFACE_MAX_PKT_BURST);

	//XXX: statistics

	//ROFL_DEBUG_VERBOSE(DRIVER_NAME"[io] Read burst from %s (%u pkts)\n", port->name, burst_len);

	//Prefetch
	if( burst_len )
		rte_prefetch0(rte_pktmbuf_mtod(pkts_burst[0], void *));


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

#if DEBUG
		//We only support nb_segs == 1. TODO: can it be that NICs send us pkts with more than one segment?
		assert(mbuf->pkt.nb_segs == 1);

		if(unlikely(!port_mapping[mbuf->pkt.in_port])){
			//Not attached	
			rte_pktmbuf_free(mbuf);
			continue;
		}
#endif

		//Init&classify	
		init_datapacket_dpdk(pkt_dpdk, mbuf, sw, port_mapping[mbuf->pkt.in_port]->of_port_num, 0, true, false);
	
		//Prefetch next pkt
		if( (i+1) < burst_len )
			rte_prefetch0(rte_pktmbuf_mtod(pkts_burst[i+1], void *));

		//Send to process
		of_process_packet_pipeline(sw, pkt);
	}	
}

}// namespace xdpd::gnu_linux_dpdk 
}// namespace xdpd

#endif //_RX_H_
