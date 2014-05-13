/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _DPDK_DATAPACKET_H_
#define _DPDK_DATAPACKET_H_

#include "../config.h"
#include "../util/time_measurements.h"

#include <rte_config.h> 
#include <rte_common.h> 
#include <rte_eal.h> 
#include <rte_mbuf.h> 

#include "packet_classifiers/c_pktclassifier/c_pktclassifier.h"

/**
* @brief Datapacket abstraction for dpdk (GNU/Linux)
*
* @ingroup fm_gnu_linux_dpdk_io
*
* @ingroup fm_gnu_linux_dpdk_io
*/

typedef struct datapacket_dpdk{
	
	//General data of the packet
	uint64_t buffer_id;		//Unique "non-reusable" buffer id
	uint64_t internal_buffer_id;	//IO subsystem buffer ID

	//Incomming packet information
	uint32_t in_port;
	uint32_t in_phy_port;
	
	//Output queue
	uint32_t output_queue;

	//Checksum flags
	bool ipv4_recalc_checksum;
	bool tcp_recalc_checksum;
	bool udp_recalc_checksum;
	bool icmpv4_recalc_checksum;

	//Temporary store for pkt_in information
	uint8_t pktin_table_id;
	of_packet_in_reason_t pktin_reason;
	uint16_t pktin_send_len;
	
	//Time profiling
	TM_PKT_STATE;
	
	//Header packet classification
	struct classify_state* headers;
	
	// Pointer to the buffer
	struct rte_mbuf* mbuf;
	
	// True if the packet is stored in the bufferpool
	// used to know when to free the slot
	bool packet_in_bufferpool;
	
}datapacket_dpdk_t;

//C++ extern C
ROFL_BEGIN_DECLS

/// Datapacket dpdk functions

//Constructor&destructor
datapacket_dpdk_t* create_datapacket_dpdk(datapacket_t* pkt);
void destroy_datapacket_dpdk(datapacket_dpdk_t *dpkt);

//Return the pointer to the buffer
static inline uint8_t* get_buffer_dpdk(datapacket_dpdk_t *dpkt){
	return (uint8_t *)rte_pktmbuf_mtod(dpkt->mbuf, uint8_t*);
}

static inline size_t get_buffer_length_dpdk(datapacket_dpdk_t *dpkt){
	return rte_pktmbuf_pkt_len(dpkt->mbuf);
}

//Init & reset (inline)
static inline rofl_result_t init_datapacket_dpdk(datapacket_dpdk_t *dpkt, struct rte_mbuf* mbuf, of_switch_t* sw, uint32_t in_port, uint32_t in_phy_port, bool classify, bool packet_is_in_bufferpool){
	
#ifdef DEBUG
	if( unlikely(NULL == rte_pktmbuf_mtod(mbuf, uint8_t*) ) )
		return ROFL_FAILURE;
#endif

	//Fill the structure
	dpkt->in_port = in_port;
	dpkt->in_phy_port = in_phy_port;
	dpkt->output_queue = 0;
	dpkt->mbuf = mbuf;
	dpkt->packet_in_bufferpool = packet_is_in_bufferpool;

	//Classify the packet
	if(likely(classify))
		classify_packet(dpkt->headers, get_buffer_dpdk(dpkt), get_buffer_length_dpdk(dpkt), in_port, in_phy_port);

	return ROFL_SUCCESS;
}
static inline void reset_datapacket_dpdk(datapacket_dpdk_t *dpkt){
	reset_classifier(dpkt->headers);
}

// Push & Pop raw operations. To be used ONLY by classifiers
rofl_result_t push_datapacket_offset(datapacket_dpdk_t *dpkt, unsigned int offset, unsigned int num_of_bytes);
rofl_result_t pop_datapacket_offset(datapacket_dpdk_t *dpkt, unsigned int offset, unsigned int num_of_bytes);

rofl_result_t push_datapacket_point(datapacket_dpdk_t *dpkt, uint8_t* push_point, unsigned int num_of_bytes);
rofl_result_t pop_datapacket_point(datapacket_dpdk_t *dpkt, uint8_t* pop_point, unsigned int num_of_bytes);

//TODO dump() ?

//C++ extern C
ROFL_END_DECLS

#endif //_DPDK_DATAPACKET_H_
