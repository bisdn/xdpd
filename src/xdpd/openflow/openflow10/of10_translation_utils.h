/*
 * of10_translation_utils.h
 *
 *  Created on: 06.09.2013
 *      Author: andreas
 */


#ifndef OF10_TRANSLATION_UTILS_H
#define OF10_TRANSLATION_UTILS_H

#include <rofl/common/openflow/cofctl.h>
#include <rofl/common/openflow/messages/cofmsg_features.h>
#include <rofl/common/openflow/messages/cofmsg_flow_mod.h>
#include <rofl/platform/unix/csyslog.h>
#include <rofl/datapath/pipeline/openflow/openflow12/of12_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_flow_entry.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_action.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_match.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_instruction.h>

#include "../openflow_switch.h"

#include <rofl/common/openflow/experimental/matches/pppoe_matches.h>
#include <rofl/common/openflow/experimental/actions/pppoe_actions.h>

/**
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*/

namespace rofl {

class of10_translation_utils : public csyslog{


public:
	/**
	* Port speeds
	*/
	static uint32_t get_port_speed_kb(port_features_t features);
	/**
	* Maps a of12_flow_entry from an OF1.0 Header
	*/
	static of12_flow_entry_t* of12_map_flow_entry(cofctl *ctl,  cofmsg_flow_mod *msg, openflow_switch* sw);

	/**
	* Maps a of12_match from an OF1.0 Header
	*/
	static void of10_map_flow_entry_matches(cofctl *ctl, cofmatch const& match, openflow_switch* sw, of12_flow_entry* entry);



	/**
	* Maps a of12_action from an OF1.0 Header
	*/
	static void of12_map_flow_entry_actions(cofctl *ctl, openflow_switch* sw, cofaclist& actions, of12_action_group_t *apply_actions, of12_write_actions_t *write_actions);


	/**
	* Maps a of12_match from an OF1.0 Header
	*/
	static void of12_map_reverse_flow_entry_matches(of12_match_t* m, cofmatch& match);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_instructions(of12_instruction_group_t* group, cofinlist& instructions);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_instruction(of12_instruction_t* inst, cofinst& instruction);

	/**
	*
	*/
	static void of12_map_reverse_flow_entry_action(of12_packet_action_t* of12_action, cofaction& action);

	/**
	*
	*/
	static void of12_map_reverse_packet_matches(of12_packet_matches_t* packet_matches, cofmatch& match);
};

}// namespace rofl

#endif /* OF10_TRANSLATION_UTILS_H_ */


