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

#include <iostream>

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
process_port_rx(unsigned int core_id, switch_port_t* port, struct rte_mbuf** pkts_burst, datapacket_t* pkt, datapacket_dpdk_t* pkt_state){
	
	unsigned int i, burst_len = 0;
	of_switch_t* sw = port->attached_sw;
	struct rte_mbuf* mbuf;
	datapacket_dpdk_t* pkt_dpdk = pkt_state;

	if(unlikely(port->drop_received)) //Ignore if port is marked as "drop received"
		return;
		
	//Read a burst
#ifdef GNU_LINUX_DPDK_ENABLE_NF	
	if(port->type == PORT_TYPE_NF_SHMEM) 
	{
		//DPDK NF port - pkts received through an rte_ring
		nf_port_state_dpdk *port_state = (nf_port_state_dpdk_t*)port->platform_port_state;
		burst_len = rte_ring_mc_dequeue_burst(port_state->to_xdpd_queue, (void **)pkts_burst, IO_IFACE_MAX_PKT_BURST);
	}
	else if(port->type == PORT_TYPE_NF_EXTERNAL)
	{		
		//KNI NF port - pkts received through a KNI interface
		nf_port_state_kni *port_state = (nf_port_state_kni_t*)port->platform_port_state;
		assert(port_state->kni != NULL);
		
		burst_len = rte_kni_rx_burst(port_state->kni, pkts_burst, IO_IFACE_MAX_PKT_BURST);
			
#if DEBUG
		if(burst_len != 0)
		{
			ROFL_DEBUG_VERBOSE(DRIVER_NAME"[io] Read burst from %s (%u pkts)\n", port->name, burst_len);
	
			for(i=0;i<burst_len;i++)
			{
				unsigned char *tmp = rte_pktmbuf_mtod(pkts_burst[i],unsigned char *);
				unsigned int tmp_len = rte_pktmbuf_pkt_len(pkts_burst[i]);	 
				ROFL_DEBUG_VERBOSE("#%d length: %d\n",i,tmp_len);	
				ROFL_DEBUG_VERBOSE("#%d %x:%x:%x:%x:%x:%x->%x:%x:%x:%x:%x:%x\n",i,tmp[6],tmp[7],tmp[8],tmp[9],tmp[10],tmp[11],tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
			}	
		}
#endif
	}else
#endif
	{
		//Physical port - pkts received through an ethernet port
		unsigned int port_id = ((dpdk_port_state_t*)port->platform_port_state)->port_id;
		burst_len = rte_eth_rx_burst(port_id, 0, pkts_burst, IO_IFACE_MAX_PKT_BURST);
	}

	//ROFL_DEBUG_VERBOSE(DRIVER_NAME"[io] Read burst from %s (%u pkts)\n", port->name, burst_len);

	//Prefetch
	if( burst_len )
		rte_prefetch0(rte_pktmbuf_mtod(pkts_burst[0], void *));

	//Process them 
	for(i=0;i<burst_len;++i){
		mbuf = pkts_burst[i];		

#ifdef DEBUG	
		if(unlikely(sw == NULL)){
			rte_pktmbuf_free(mbuf);
			continue;
		}
#endif

		//set mbuf pointer in the state so that it can be recovered afterwards when going
		//out from the pipeline
		pkt_state->mbuf = mbuf;

		//Increment port RX statistics
		port->stats.rx_packets++;
		port->stats.rx_bytes += mbuf->pkt.pkt_len;

		//We only support nb_segs == 1. TODO: can it be that NICs send us pkts with more than one segment?
		assert(mbuf->pkt.nb_segs == 1);

		//tmp_port is used to avoid to repeat code for both kinds of port
		//(note that the port_mapping used is different
		switch_port_t *tmp_port;
#ifdef GNU_LINUX_DPDK_ENABLE_NF	
		if(port->type == PORT_TYPE_NF_SHMEM) {
			tmp_port = port;
		}
		else if(port->type == PORT_TYPE_NF_EXTERNAL) {
			tmp_port=port;
		}else
#endif
		{
			tmp_port = phy_port_mapping[mbuf->pkt.in_port];
		}

		if(unlikely(!tmp_port)){
			//Not attached
			rte_pktmbuf_free(mbuf);
			continue;
		}

		//Init&classify	
		init_datapacket_dpdk(pkt_dpdk, mbuf, sw, tmp_port->of_port_num, 0, true, false);
	
		//Prefetch next pkt
		if( (i+1) < burst_len )
			rte_prefetch0(rte_pktmbuf_mtod(pkts_burst[i+1], void *));

		//Send to process
		of_process_packet_pipeline(core_id, sw, pkt);
	}	
}

}// namespace xdpd::gnu_linux_dpdk 
}// namespace xdpd

#endif //_RX_H_
