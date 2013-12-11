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

#ifdef C_PACKET_CLASSIFIER
	#include "packet_classifiers/c_pktclassifier/c_pktclassifier.h"
#else
	#include "packet_classifiers/cpp_pktclassifier/cpp_pktclassifier.h"
#endif

#if 0
/*
* Binds datapacket dpdk's state (mbuf...)
*/
typedef struct dpdk_pkt_platform_state{

	xdpd::gnu_linux::datapacketx86* pktx86;
	struct rte_mbuf* mbuf;

}dpdk_pkt_platform_state_t;
#endif

//buffering status
typedef enum{
	DPDK_DATAPACKET_BUFFER_IS_EMPTY,
	DPDK_DATAPACKET_BUFFERED_IN_NIC,
	DPDK_DATAPACKET_BUFFERED_IN_USER_SPACE
}dpdk_buffering_status_t;

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
	of_switch_t* lsw;
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
	
	//Time profiling
	TM_PKT_STATE;
	
	//Header packet classification
	struct classify_state* headers;
	
	// Pointer to the buffer
	struct rte_mbuf* mbuf;
	
	//Status of this buffer
	dpdk_buffering_status_t buffering_status;
	
}datapacket_dpdk_t;

//C++ extern C
ROFL_BEGIN_DECLS

/// Datapacket dpdk functions

//Constructor&destructor
datapacket_dpdk_t* create_datapacket_dpdk(void);
void destroy_datapacket_dpdk(datapacket_dpdk_t *dpkt);

// Init & reset
rofl_result_t init_datapacket_dpdk(datapacket_dpdk_t *dpkt, struct rte_mbuf* mbuf, of_switch_t* sw, uint32_t in_port, uint32_t in_phy_port, bool classify, bool copy_packet_to_internal_buffer);
void reset_datapacket_dpdk(datapacket_dpdk_t *dpkt);

//Return the pointer to the buffer
inline uint8_t* get_buffer_dpdk(datapacket_dpdk_t *dpkt){
	return (uint8_t *)rte_pktmbuf_mtod(dpkt->mbuf, uint8_t*);
}

inline size_t get_buffer_length_dpdk(datapacket_dpdk_t *dpkt){
	return rte_pktmbuf_pkt_len(dpkt->mbuf);
}

inline dpdk_buffering_status_t get_buffering_status_dpdk(datapacket_dpdk_t *dpkt){
	return dpkt->buffering_status;
}

// Push & Pop raw operations. To be used ONLY by classifiers
rofl_result_t push_datapacket_offset(datapacket_dpdk_t *dpkt, unsigned int offset, unsigned int num_of_bytes);
rofl_result_t pop_datapacket_offset(datapacket_dpdk_t *dpkt, unsigned int offset, unsigned int num_of_bytes);

rofl_result_t push_datapacket_point(datapacket_dpdk_t *dpkt, uint8_t* push_point, unsigned int num_of_bytes);
rofl_result_t pop_datapacket_point(datapacket_dpdk_t *dpkt, uint8_t* pop_point, unsigned int num_of_bytes);

//NOTE dump() ?

//C++ extern C
ROFL_END_DECLS

#endif //_DPDK_DATAPACKET_H_
