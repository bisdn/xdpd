/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _TX_H_
#define _TX_H_

#include "../config.h"
#include <rofl/common/utils/c_logger.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_spinlock.h>
#include <rte_eal.h> 
#include <rte_mbuf.h> 
#include <rte_ethdev.h> 

#include <assert.h>
#include "bufferpool.h"
#include "dpdk_datapacket.h"

#include "port_state.h"
#include "port_manager.h"
#include "../processing/processing.h"

namespace xdpd {
namespace gnu_linux_dpdk {

//
// Packet processing
//


inline void
process_port_queue_tx(switch_port_t* port, unsigned int port_id, struct mbuf_burst* queue, unsigned int queue_id){
	unsigned ret;

	if(unlikely((port->up == false)) || queue->len == 0){
		//static int j=0;
		//j++;
		//if((j%1000000 == 0) && (queue_id == 0))
		//	ROFL_DEBUG(DRIVER_NAME"[io] Auto purge to send burst on port %s(%u) queue %p (queue_id: %u) of length: %u\n", port->name,  port_id, queue, queue_id, queue->len);
		return;
	}

	ROFL_DEBUG(DRIVER_NAME"[io] Trying to send burst on port %s(%u) queue %p (queue_id: %u) of length: %u\n", port->name,  port_id, queue, queue_id, queue->len);
	//Send burst
	ret = rte_eth_tx_burst(port_id, queue_id, queue->burst, queue->len);
	//XXX port_statistics[port].tx += ret;
	
	ROFL_DEBUG(DRIVER_NAME"[io] +++++++++++++++++++++++++++ Transmited %u pkts, on port %s(%u)\n", ret, port->name, port_id);

	if (unlikely(ret < queue->len)) {
		//XXX port_statistics[port].dropped += (n - ret);
		do {
			rte_pktmbuf_free(queue->burst[ret]);
		} while (++ret < queue->len);
	}

	//Reset queue size	
	queue->len = 0;
}

inline void
tx_pkt(switch_port_t* port, unsigned int queue_id, datapacket_t* pkt){

	struct rte_mbuf* mbuf;
	struct mbuf_burst* pkt_burst;
	unsigned int port_id, len;

	//Get mbuf pointer
	mbuf = ((datapacket_dpdk_t*)pkt->platform_state)->mbuf;
	port_id = ((dpdk_port_state_t*)port->platform_port_state)->port_id;

	if(unlikely(!mbuf)){
		assert(0);
		return;
	}
	
	//Recover core task
	core_tasks_t* tasks = &processing_cores[rte_lcore_id()];
	
	//Recover burst container
	pkt_burst = &tasks->all_ports[port_id].tx_queues_burst[queue_id];	
	
	if(unlikely(!pkt_burst)){
		rte_pktmbuf_free(mbuf);
		assert(0);
		return;
	}

	ROFL_DEBUG(DRIVER_NAME"[io] Adding packet %p to queue %p (id: %u)\n", pkt, pkt_burst, rte_lcore_id());

	//Enqueue
	len = pkt_burst->len; 
	pkt_burst->burst[len] = mbuf;
	len++;

	//If burst is full => trigger send
	if (unlikely(len == IO_IFACE_MAX_PKT_BURST) || unlikely(!tasks->active)) { //If buffer is full or mgmt core
		pkt_burst->len = len;
		process_port_queue_tx(port, port_id, pkt_burst, queue_id);
		return;
	}

	pkt_burst->len = len;

	return;
}

void tx_pkt_vlink(switch_port_t* vlink, datapacket_t* pkt);

}// namespace xdpd::gnu_linux_dpdk 
}// namespace xdpd

#endif //_TX_H_
