#include "flow_entry_snapshot.h"

//Translators for matches and actions
#include "../../openflow/openflow10/of10_translation_utils.h"
#include "../../openflow/openflow12/of12_translation_utils.h"
#include "../../openflow/openflow13/of13_translation_utils.h"

using namespace xdpd;

flow_entry_snapshot::flow_entry_snapshot(of_version_t ver, uint16_t miss_send_len, of1x_stats_single_flow_msg_t* flow_it){

		rofl::openflow::cofinstructions inst;
		rofl::openflow::cofactions actions;
		rofl::openflow::cofmatch match;
		
		//Add general info
		set_version(ver);
		set_priority(flow_it->priority);
		set_cookie(flow_it->cookie);
		set_idle_timeout(flow_it->idle_timeout);
		set_hard_timeout(flow_it->hard_timeout);
		set_duration_sec(flow_it->duration_sec);
		set_duration_nsec(flow_it->duration_nsec);
		set_packet_count(flow_it->packet_count);
		set_byte_count(flow_it->byte_count);
	
		//Translate match&instructions
		switch(ver){
			case OF_VERSION_10:	
				of10_translation_utils::of1x_map_reverse_flow_entry_matches(flow_it->matches, match);
				of10_translation_utils::of1x_map_reverse_flow_entry_actions(flow_it->inst_grp, actions, miss_send_len); 

				set_match() = match;
				set_actions() = actions;
				break;
			case OF_VERSION_12:
				of12_translation_utils::of12_map_reverse_flow_entry_matches(flow_it->matches, match);
				of12_translation_utils::of12_map_reverse_flow_entry_instructions(flow_it->inst_grp,  inst);
				set_match() = match;
				set_instructions() = inst;
				break;
			case OF_VERSION_13:
				of13_translation_utils::of13_map_reverse_flow_entry_matches(flow_it->matches, match);
				of13_translation_utils::of13_map_reverse_flow_entry_instructions(flow_it->inst_grp,  inst);
				set_match() = match;
				set_instructions() = inst;
				break;
			default:
				assert(0);
				break;
		}	
}

rofl_result_t flow_entry_snapshot::map_flow_stats_msg(of_version_t ver, uint16_t miss_send_len, of1x_stats_flow_msg_t* hal_flows, std::list<flow_entry_snapshot>& flows ){

	of1x_stats_single_flow_msg_t* flow_it;

	try{ 
		//Translate flows
		for(flow_it = hal_flows->flows_head;flow_it;flow_it=flow_it->next){
			flows.push_back(flow_entry_snapshot(ver, miss_send_len, flow_it));
		}
	}catch(...){
		assert(0);
		return ROFL_FAILURE;
	}

	return ROFL_SUCCESS;
}
