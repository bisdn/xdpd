/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OF10_TRANSLATION_UTILS_H
#define OF10_TRANSLATION_UTILS_H

#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/messages/cofmsg_features.h>
#include <rofl/common/openflow/messages/cofmsg_flow_mod.h>
#include <rofl/platform/unix/csyslog.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_action.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_match.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_instruction.h>

#include "../openflow_switch.h"

#include <rofl/common/openflow/experimental/matches/pppoe_matches.h>
#include <rofl/common/openflow/experimental/actions/pppoe_actions.h>

/**
* @file of10_translation_utils.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief OpenFlow 1.0 translation utils
*/

using namespace rofl;

namespace xdpd {

/**
* @brief Translates ROFL OF1.0 objects to AFA-compliant structures and vice-versa
* @ingroup cmm_of
*/
class of10_translation_utils{


public:
	/**
	* Port speeds
	*/
	static uint32_t get_port_speed_kb(port_features_t features);
	/**
	* Maps a of1x_flow_entry from an OF1.0 Header
	*/
	static of1x_flow_entry_t* of1x_map_flow_entry(cofctl *ctl,  cofmsg_flow_mod *msg, openflow_switch* sw);

	/**
	* Maps a of1x_match from an OF1.0 Header
	*/
	static void of10_map_flow_entry_matches(cofctl *ctl, cofmatch const& match, openflow_switch* sw, of1x_flow_entry* entry);



	/**
	* Maps a of1x_action from an OF1.0 Header
	*/
	static void of1x_map_flow_entry_actions(cofctl *ctl, openflow_switch* sw, cofaclist& actions, of1x_action_group_t *apply_actions, of1x_write_actions_t *write_actions);


	/**
	* Maps a of1x_match from an OF1.0 Header
	*/
	static void of1x_map_reverse_flow_entry_matches(of1x_match_t* m, cofmatch& match);

	static void of1x_map_reverse_flow_entry_instructions(of1x_instruction_group_t* group, cofinlist& instructions, uint16_t pipeline_miss_send_len);

	/**
	*
	*/
	static void of1x_map_reverse_flow_entry_instruction(of1x_instruction_t* inst, cofinst& instruction, uint16_t pipeline_miss_send_len);

	/**
	*
	*/
	static void of1x_map_reverse_flow_entry_action(of1x_packet_action_t* of1x_action, cofaction& action, uint16_t pipeline_miss_send_len);

	/**
	*
	*/
	static void of1x_map_reverse_packet_matches(of1x_packet_matches_t* packet_matches, cofmatch& match);
	
	/**
	 * Actions supported by switch
	 */
	static uint32_t get_supported_actions(of1x_switch_t* lsw);
	
	/**
	 * Translate special port numbers from 1.0 to 1.X
	 */
	static uint64_t get_out_port(uint16_t port);
	
	/**
	 * Reverse translate special port numbers from 1.0 to 1.X
	 */
	static uint32_t get_out_port_reverse(uint64_t port);
};

}// namespace rofl

#endif /* OF10_TRANSLATION_UTILS_H_ */


