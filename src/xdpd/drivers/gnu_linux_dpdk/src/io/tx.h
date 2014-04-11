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
transmit_port_queue_tx_burst(unsigned int port_id, unsigned int queue_id){
	
	unsigned int ret, len;
	struct rte_mbuf* tmp[IO_IFACE_MAX_PKT_BURST];

	//Dequeue a burst from the TX ring	
	len = rte_ring_mc_dequeue_burst(port_tx_lcore_queue[port_id][queue_id], (void **)tmp, IO_IFACE_MAX_PKT_BURST);      

	ROFL_DEBUG(DRIVER_NAME"[io][%s(%u)] Trying to transmit burst on port queue_id %u of length %u\n", port_mapping[port_id]->name,  port_id, queue_id, len);

	//Send burst
	ret = rte_eth_tx_burst(port_id, queue_id, tmp, len);
	//XXX port_statistics[port].tx += ret;
	
	ROFL_DEBUG(DRIVER_NAME"[io][%s(%u)] +++ Transmited %u pkts, on queue_id %u\n", port_mapping[port_id]->name, port_id, ret, queue_id);

	if (unlikely(ret < len)) {
		//XXX port_statistics[port].dropped += (n - ret);
		do {
			rte_pktmbuf_free(tmp[ret]);
		} while (++ret < len);
	}
}

inline void
flush_port_queue_tx_burst(switch_port_t* port, unsigned int port_id, struct mbuf_burst* queue, unsigned int queue_id){
	unsigned ret;

	if(unlikely((port->up == false)) || queue->len == 0){
		return;
	}

	ROFL_DEBUG(DRIVER_NAME"[io][%s(%u)] Trying to flush burst(enqueue in lcore ring) on port queue_id %u of length: %u\n", port->name,  port_id, queue_id, queue->len);
		
	//Enqueue to the lcore (if it'd we us, we could probably call to transmit directly)
	ret = rte_ring_mp_enqueue_burst(port_tx_lcore_queue[port_id][queue_id], (void **)queue->burst, queue->len);
	//XXX port_statistics[port].tx += ret;
	
	ROFL_DEBUG(DRIVER_NAME"[io][%s(%u)] --- Flushed %u pkts, on queue id %u\n", port->name, port_id, ret, queue_id);

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
	core_tasks_t* tasks = &processing_core_tasks[rte_lcore_id()];
	
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
		flush_port_queue_tx_burst(port, port_id, pkt_burst, queue_id);
		return;
	}

	pkt_burst->len = len;

	return;
}

void tx_pkt_vlink(switch_port_t* vlink, datapacket_t* pkt);

}// namespace xdpd::gnu_linux_dpdk 
}// namespace xdpd

#endif //_TX_H_
