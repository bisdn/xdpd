/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PACKET_OPERATIONS_DPDK_
#define _PACKET_OPERATIONS_DPDK_

#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "../dpdk_datapacket.h"

/**
* @file packet_operations.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Inline implementation for DPDK
*/


ROFL_BEGIN_DECLS

static inline rofl_result_t pkt_push(datapacket_t* pkt, uint8_t* push_point, unsigned int offset, unsigned int num_of_bytes){
	datapacket_dpdk_t *pkt_state = (datapacket_dpdk_t*)(pkt->platform_state);
	if(push_point)
		return push_datapacket_point(pkt_state,push_point,num_of_bytes);
	else
		return push_datapacket_offset(pkt_state, offset, num_of_bytes);
}

static inline rofl_result_t pkt_pop(datapacket_t* pkt, uint8_t* pop_point, unsigned int offset, unsigned int num_of_bytes){
	datapacket_dpdk_t *pkt_state = (datapacket_dpdk_t*)(pkt->platform_state);
	if(pop_point)
		return pop_datapacket_point(pkt_state, pop_point,num_of_bytes);
	else
		return pop_datapacket_offset(pkt_state, offset, num_of_bytes);
}


ROFL_END_DECLS

#endif //_PACKET_OPERATIONS_DPDK_
