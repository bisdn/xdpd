/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OF12_TRANSLATION_UTILS_H
#define OF12_TRANSLATION_UTILS_H 

#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/messages/cofmsg_features.h>
#include <rofl/common/openflow/messages/cofmsg_flow_mod.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_action.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_match.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_instruction.h>

#include "../openflow_switch.h"

#include <rofl/common/openflow/experimental/actions/pppoe_actions.h>
#include <rofl/common/openflow/experimental/actions/gtp_actions.h>
#include <rofl/common/openflow/experimental/actions/capwap_actions.h>
#include <rofl/common/openflow/experimental/actions/wlan_actions.h>
#include <rofl/common/openflow/experimental/actions/gre_actions.h>

/**
* @file of12_translation_utils.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief OpenFlow 1.2 translation utils
*/

using namespace rofl;

namespace xdpd {

/**
* @brief Translates ROFL OF1.2 objects to HAL-compliant structures and vice-versa
* @ingroup cmm_of
*/
class of12_translation_utils{
	

public:
	/**
	* Port speeds
	*/
	static uint32_t get_port_speed_kb(port_features_t features);
	/**
	* Maps a of1x_flow_entry from an OF1.2 Header
	*/
	static of1x_flow_entry_t* of12_map_flow_entry(crofctl *ctl,  rofl::openflow::cofmsg_flow_mod *msg, openflow_switch* sw);

	/**
	* Maps a of1x_match from an OF1.2 Header
	*/
	static void of12_map_flow_entry_matches(crofctl *ctl, rofl::openflow::cofmatch const& match, openflow_switch* sw, of1x_flow_entry* entry);



	/**
	* Maps a of1x_action from an OF1.2 Header
	*/
	static void of12_map_flow_entry_actions(crofctl *ctl, openflow_switch* sw, const rofl::openflow::cofactions& actions, of1x_action_group_t *apply_actions, of1x_write_actions_t *write_actions);


	/**
	* Maps a of1x_match from an OF1.2 Header
	*/
	static void of12_map_reverse_flow_entry_matches(of1x_match_t* m, rofl::openflow::cofmatch& match);
	
	/**
	 * Maps a of12_group_bucket from an OF1.2 Header
	 */
	static void of12_map_bucket_list(crofctl *ctl, openflow_switch* sw, rofl::openflow::cofbuckets& of_buckets, of1x_bucket_list_t* bucket_list);

	/**
	 * Reverse maps a bucket list 
	 */
	static void of12_map_reverse_bucket_list(	rofl::openflow::cofbuckets& of_buckets, of1x_stats_bucket_desc_msg* bucket_list);
	
	/**
	*
	*/
	static void of12_map_reverse_flow_entry_instructions(of1x_instruction_group_t* group, rofl::openflow::cofinstructions& instructions);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_instruction_goto_table(of1x_instruction_t* inst, rofl::openflow::cofinstruction_goto_table& instruction);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_instruction_apply_actions(of1x_instruction_t* inst, rofl::openflow::cofinstruction_apply_actions& instruction);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_instruction_write_actions(of1x_instruction_t* inst, rofl::openflow::cofinstruction_write_actions& instruction);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_instruction_clear_actions(of1x_instruction_t* inst, rofl::openflow::cofinstruction_clear_actions& instruction);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_instruction_write_metadata(of1x_instruction_t* inst, rofl::openflow::cofinstruction_write_metadata& instruction);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_instruction_experimenter(of1x_instruction_t* inst, rofl::openflow::cofinstruction_experimenter& instruction);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_action(of1x_packet_action_t* of1x_action, const rofl::cindex& index, rofl::openflow::cofactions& actions);

	/**
	*
	*/
	static void of12_map_reverse_packet_matches(packet_matches_t* packet_matches, rofl::openflow::cofmatch& match);

	static uint64_t of12_map_bitmap_matches(bitmap128_t* bitmap);

	static uint32_t of12_map_bitmap_actions(bitmap128_t* bitmap);

	static uint32_t of12_map_bitmap_instructions(uint32_t* bitmap);
	
	static uint64_t of12_map_bitmap_set_fields(bitmap128_t* bitmap);
};

}// namespace rofl

#endif /* OF12_TRANSLATION_UTILS_H_ */
