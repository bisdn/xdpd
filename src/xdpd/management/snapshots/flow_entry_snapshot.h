/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FLOW_ENTRY_SNAPSHOT_H
#define FLOW_ENTRY_SNAPSHOT_H 

#include <string> 
#include <list> 
#include <rofl.h>
#include <rofl/common/openflow/cofflowstats.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/matching_algorithms/matching_algorithms.h>

/**
* @file flow_entry_snapshot.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief C++ flow entry snapshot (wrapper)
*/

namespace xdpd {

/**
* @brief C++ flow entry snapshot 
* @ingroup cmm_mgmt
*/
class flow_entry_snapshot : public rofl::openflow::cofflow_stats_reply {

public:
	flow_entry_snapshot(of_version_t ver, uint16_t miss_send_len, of1x_stats_single_flow_msg_t*);
	static rofl_result_t map_flow_stats_msg(of_version_t ver, uint16_t miss_send_len, of1x_stats_flow_msg_t* hal_flows, std::list<flow_entry_snapshot>& flows ); 
};


}// namespace xdpd

#endif /* FLOW_ENTRY_SNAPSHOT_H_ */
