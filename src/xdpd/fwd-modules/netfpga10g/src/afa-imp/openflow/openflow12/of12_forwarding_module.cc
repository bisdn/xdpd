#include <rofl/datapath/afa/openflow/openflow12/of12_fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_pipeline.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_flow_entry.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_statistics.h>

#include <rofl/datapath/pipeline/openflow/openflow12/openflow12.h>

#define FWD_MOD_NAME "netfpga10g"

//Port config

/**
 * @name    fwd_module_of12_set_port_drop_received_config
 * @brief   Instructs driver to modify port config state 
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 			Datapath ID of the switch 
 * @param port_num		Port number 	
 * @param drop_received		Drop packets received
 */
afa_result_t fwd_module_of12_set_port_drop_received_config(uint64_t dpid, unsigned int port_num, bool drop_received){
	
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);

	return AFA_SUCCESS;
}

/**
 * @name    fwd_module_of12_set_port_forward_config
 * @brief   Instructs driver to modify port config state 
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 			Datapath ID of the switch 
 * @param port_num		Port number 	
 * @param forward		Forward packets
 */
afa_result_t fwd_module_of12_set_port_forward_config(uint64_t dpid, unsigned int port_num, bool forward){
	
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
	return AFA_SUCCESS;
}
/**
 * @name    fwd_module_of12_set_port_generate_packet_in_config
 * @brief   Instructs driver to modify port config state 
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 			Datapath ID of the switch 
 * @param port_num		Port number 	
 * @param generate_packet_in	Generate packet in events for this port 
 */
afa_result_t fwd_module_of12_set_port_generate_packet_in_config(uint64_t dpid, unsigned int port_num, bool generate_packet_in){
	
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
	return AFA_SUCCESS;
}

/**
 * @name    fwd_module_of12_set_port_advertise_config
 * @brief   Instructs driver to modify port advertise flags 
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 			Datapath ID of the switch 
 * @param port_num		Port number 	
 * @param advertise		Bitmap advertised
 */
afa_result_t fwd_module_of12_set_port_advertise_config(uint64_t dpid, unsigned int port_num, uint32_t advertise){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
	return AFA_SUCCESS;
}

/**
 * @name    fwd_module_of12_set_pipeline_config
 * @brief   Instructs driver to process a PACKET_OUT event
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch 
 * @param flags		Capabilities bitmap (OF12_CAP_FLOW_STATS, OF12_CAP_TABLE_STATS, ...)
 * @param miss_send_len	OF MISS_SEND_LEN
 */
afa_result_t fwd_module_of12_set_pipeline_config(uint64_t dpid, unsigned int flags, uint16_t miss_send_len){
	
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);

	return AFA_SUCCESS;
}

/**
 * @name    fwd_module_of12_set_table_config
 * @brief   Instructs driver to set table configuration(default action)
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch
 * @param table_id	Table ID or 0xFF for all 
 * @param miss_send_len Table miss config	
 */
afa_result_t fwd_module_of12_set_table_config(uint64_t dpid, unsigned int table_id, of12_flow_table_miss_config_t config){
	
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);

	return AFA_SUCCESS;
}

/**
 * @name    fwd_module_of12_process_packet_out
 * @brief   Instructs driver to process a PACKET_OUT event
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to process PACKET_OUT
 * @param buffer_id	Buffer ID
 * @param in_port 	Port IN
 * @param action_group 	Action group to apply
 * @param buffer		Pointer to the buffer
 * @param buffer_size	Buffer size
 */
afa_result_t fwd_module_of12_process_packet_out(uint64_t dpid, uint32_t buffer_id, uint32_t in_port, of12_action_group_t* action_group, uint8_t* buffer, uint32_t buffer_size)
{
	
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
	return AFA_SUCCESS;
}

/**
 * @name    fwd_module_of12_process_flow_mod
 * @brief   Instructs driver to process a FLOW_MOD event
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id to install the flowmod
 * @param flow_entry	Flow entry to be installed
 * @param buffer_id	Buffer ID
 * @param out_port 	Port to output
 * @param check_overlap	Check OVERLAP flag
 * @param check_counts	Check RESET_COUNTS flag
 */

afa_result_t fwd_module_of12_process_flow_mod_add(uint64_t dpid, uint8_t table_id, of12_flow_entry_t* flow_entry, uint32_t buffer_id, bool check_overlap, bool reset_counts){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
	return AFA_SUCCESS;
}

/**
 * @name    fwd_module_of12_process_flow_mod_modify
 * @brief   Instructs driver to process a FLOW_MOD modify event
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id from which to modify the flowmod
 * @param flow_entry	Flow entry 
 * @param strictness 	Strictness (STRICT NON-STRICT)
 * @param check_counts	Check RESET_COUNTS flag
 */
afa_result_t fwd_module_of12_process_flow_mod_modify(uint64_t dpid, uint8_t table_id, of12_flow_entry_t* flow_entry, of12_flow_removal_strictness_t strictness, bool reset_counts){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);

	return AFA_SUCCESS;
}


/**
 * @name    fwd_module_of12_process_flow_mod_delete
 * @brief   Instructs driver to process a FLOW_MOD event
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id to install the flowmod
 * @param flow_entry	Flow entry to be installed
 * @param buffer_id	Buffer ID
 * @param out_port 	Out port that entry must include
 * @param out_group 	Out group that entry must include	
 * @param strictness 	Strictness (STRICT NON-STRICT)
 */
afa_result_t fwd_module_of12_process_flow_mod_delete(uint64_t dpid, uint8_t table_id, of12_flow_entry_t* flow_entry, uint32_t buffer_id, uint32_t out_port, uint32_t out_group, of12_flow_removal_strictness_t strictness){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
	return AFA_SUCCESS;
} 

//
// Statistics
//

/**
 * @name    fwd_module_of12_get_flow_stats
 * @brief   Recovers the flow stats given a set of matches 
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id to get the flows of 
 * @param cookie	Cookie to be applied 
 * @param cookie_mask	Mask for the cookie
 * @param out_port 	Out port that entry must include
 * @param out_group 	Out group that entry must include	
 * @param matchs	Matchs
 */
of12_stats_flow_msg_t* fwd_module_of12_get_flow_stats(uint64_t dpid, uint8_t table_id, uint32_t cookie, uint32_t cookie_mask, uint32_t out_port, uint32_t out_group, of12_match_t* matchs){
	//We are not supporting single flow stats 
	return NULL; 
}

 
/**
 * @name    fwd_module_of12_get_flow_aggregate_stats
 * @brief   Recovers the aggregated flow stats given a set of matches 
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id to get the flows of 
 * @param cookie	Cookie to be applied 
 * @param cookie_mask	Mask for the cookie
 * @param out_port 	Out port that entry must include
 * @param out_group 	Out group that entry must include	
 * @param matchs	Matchs
 */
of12_stats_flow_aggregate_msg_t* fwd_module_of12_get_flow_aggregate_stats(uint64_t dpid, uint8_t table_id, uint32_t cookie, uint32_t cookie_mask, uint32_t out_port, uint32_t out_group, of12_match_t* matchs){
	//We are not supporting aggregatestats 
	return NULL; 
} 
/**
 * @name    fwd_module_of12_group_mod_add
 * @brief   Instructs driver to add a new GROUP
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the GROUP
 */
rofl_of12_gm_result_t fwd_module_of12_group_mod_add(uint64_t dpid, of12_group_type_t type, uint32_t id, of12_bucket_list_t *buckets){
	//We are not supporting groups (fast path is 1.0!)	
	return ROFL_OF12_GM_BCOMMAND;
}

/**
 * @name    fwd_module_of12_group_mod_modify
 * @brief   Instructs driver to modify the GROUP with identification ID
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the GROUP
 */
rofl_of12_gm_result_t fwd_module_of12_group_mod_modify(uint64_t dpid, of12_group_type_t type, uint32_t id, of12_bucket_list_t *buckets){
	//We are not supporting groups (fast path is 1.0!)	
	return ROFL_OF12_GM_BCOMMAND;
}

/**
 * @name    fwd_module_of12_group_mod_del
 * @brief   Instructs driver to delete the GROUP with identification ID
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the GROUP
 */
rofl_of12_gm_result_t fwd_module_of12_group_mod_delete(uint64_t dpid, uint32_t id){
	//We are not supporting groups (fast path is 1.0!)	
	return ROFL_OF12_GM_BCOMMAND;
}

/**
 * @name    fwd_module_of12_group_search
 * @brief   Instructs driver to search the GROUP with identification ID
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to search the GROUP
 */
afa_result_t fwd_module_of12_fetch_group_table(uint64_t dpid, of12_group_table_t *group_table){
	
	return AFA_FAILURE;
}
/**
 * @name    fwd_module_of12_get_group_stats
 * @brief   Instructs driver to fetch the GROUP statistics
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch where the GROUP is
 */
of12_stats_group_msg_t * fwd_module_of12_get_group_stats(uint64_t dpid, uint32_t id){
	//We are not supporting groups (fast path is 1.0!)	
	return NULL; 
}

/**
 * @name    fwd_module_of12_get_group_all_stats
 * @brief   Instructs driver to fetch the GROUP statistics from all the groups
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch where the GROUPS are
 */
of12_stats_group_msg_t * fwd_module_of12_get_group_all_stats(uint64_t dpid, uint32_t id){
	
	of12_switch_t* lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
		return of12_get_group_all_stats(lsw->pipeline,id);
}
