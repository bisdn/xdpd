/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_STATS_
#define _NETFPGA_STATS_


#include <inttypes.h>
#include "../util/compiler_assert.h"
#include "netfpga.h"

/**
* @file stats.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NetFPGA stats manipulation routines 
*/

//Data structures
typedef struct netfpga_flow_entry_stats {
	uint32_t pkt_counter:25;
	uint8_t last_seen:7;
	uint32_t byte_counter;
}netfpga_flow_entry_stats_t;           

COMPILER_ASSERT( stats_alignment, (sizeof(netfpga_flow_entry_stats_t) == 8) );
#define NETFPGA_FLOW_ENTRY_STATS_WORD_LEN ( sizeof(netfpga_flow_entry_stats_t)/4 )
	
//C++ extern C
ROFL_BEGIN_DECLS

/**
* @brief Get the raw statistics for a flow 
*/
rofl_result_t get_raw_stats(netfpga_flow_entry_stats_t* entry, netfpga_flow_entry_stats_t* stats);


//C++ extern C
ROFL_END_DECLS

#endif //NETFPGA_STATS
