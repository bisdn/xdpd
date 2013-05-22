#include <rofl/datapath/afa/openflow/openflow12/of12_fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_pipeline.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_flow_entry.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_statistics.h>
#include "../../../io/bufferpool.h"
#include "../../../io/datapacket_storage.h"
#include "../../../io/datapacketx86.h"
#include "../../../io/ports/ioport.h"
#include "../../../ls_internal_state.h"

#include <rofl/datapath/pipeline/openflow/openflow12/openflow12.h>

//FIXME move this definition out of here
#define OF12P_NO_BUFFER	0xffffffff

/*
* Checks wheather the action group contains at least an action output
*/
static inline bool action_group_of12_packet_in_contains_output(of12_action_group_t* action_group){

	of12_packet_action_t* action;
	
	for(action=action_group->head;action;action = action->next){
		if(action->type == OF12_AT_OUTPUT)
			return true; 
	}
	
	return false;
}

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
	
	switch_port_t* port = physical_switch_get_port_by_num(dpid,port_num);
	ioport* ioport_instance;

	if(!port)
		return AFA_FAILURE;

	ioport_instance = (ioport*)port->platform_port_state;	

	//Set flag
	if(ioport_instance->set_drop_received_config(drop_received) != ROFL_SUCCESS )
		return AFA_FAILURE;
	
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
	switch_port_t* port = physical_switch_get_port_by_num(dpid,port_num);
	ioport* ioport_instance;

	if(!port)
		return AFA_FAILURE;

	ioport_instance = (ioport*)port->platform_port_state;	

	//Set flag
	if(ioport_instance->set_forward_config(forward) != ROFL_SUCCESS )
		return AFA_FAILURE;
	
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
	
	switch_port_t* port = physical_switch_get_port_by_num(dpid,port_num);
	ioport* ioport_instance;

	if(!port)
		return AFA_FAILURE;

	ioport_instance = (ioport*)port->platform_port_state;	

	//Set flag
	if(ioport_instance->set_generate_packet_in_config(generate_packet_in) != ROFL_SUCCESS )
		return AFA_FAILURE;
	
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

	switch_port_t* port = physical_switch_get_port_by_num(dpid,port_num);
	ioport* ioport_instance;

	if(!port)
		return AFA_FAILURE;

	ioport_instance = (ioport*)port->platform_port_state;	

	//Set flag
	if(ioport_instance->set_advertise_config(advertise) != ROFL_SUCCESS )
		return AFA_FAILURE;
	
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

	of_switch_t* lsw;

	//Recover switch 
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);

	//Check switch and port
	if(!lsw || lsw->of_ver != OF_VERSION_12) {
		//TODO: log this... should never happen
		assert(0);
		return AFA_FAILURE;
	}	

	//Simply store the new config
	((of12_switch_t*)lsw)->pipeline->capabilities = flags;
	((of12_switch_t*)lsw)->pipeline->miss_send_len = miss_send_len;

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

	of12_switch_t* lsw;
	unsigned int i;

	//Recover switch 
	lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);

	//Check switch and port
	if(!lsw || 
		lsw->of_ver != OF_VERSION_12 || 
		( (table_id != OF12_FLOW_TABLE_ALL) && (table_id >= lsw->pipeline->num_of_tables) )
	) {
		//TODO: log this... should never happen
		assert(0);
		return AFA_FAILURE;
	}	

	//Simply store the new config
	if( table_id == OF12_FLOW_TABLE_ALL ){
		for( i=0; i < lsw->pipeline->num_of_tables; i++){
			lsw->pipeline->tables[i].default_action = config;
		}
	}else{
		lsw->pipeline->tables[table_id].default_action = config;
	}

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
	of_switch_t* lsw;
	datapacket_t* pkt;

	//Recover port	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);

	//Check switch and port
	if(!lsw || lsw->of_ver != OF_VERSION_12) {
		//TODO: log this... should never happen
		assert(0);
		return AFA_FAILURE;
	}	
	
	//Avoid DoS. Check whether the action list contains an action ouput, otherwise drop, since the packet will never be freed
	if(!action_group_of12_packet_in_contains_output(action_group)){
		//FIXME: free action_group??
		return AFA_FAILURE; /*TODO add specific error */
	}
	
	//Recover pkt buffer if is stored. Otherwise pick a free buffer
	if(buffer_id != OF12P_NO_BUFFER){
	
		//Retrieve the packet
		pkt = datapacket_storage_get_packet_wrapper(((struct logical_switch_internals*)lsw->platform_state)->store_handle, buffer_id);

		//Buffer has expired
		if(!pkt){
			return AFA_FAILURE; /* TODO: add specific error */
		}
	}else{
		//Retrieve a free buffer	
		pkt = bufferpool::get_free_buffer();
		
		//Initialize the packet and copy
		((datapacketx86*)pkt->platform_state)->init(buffer, buffer_size, lsw, in_port, 0, true);
	}

	//Reclassify the packet
	((datapacketx86*)pkt->platform_state)->headers->classify();

	ROFL_DEBUG_VERBOSE("Getting packet out [%p]\n",pkt);	
	
	//Instruct pipeline to process actions. This may reinject the packet	
	of12_process_packet_out_pipeline(lsw, pkt, action_group);
	
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

	of12_switch_t* lsw;
	rofl_of12_fm_result_t result;

	//Recover port	
	lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);


	if(table_id >= lsw->pipeline->num_of_tables)
		return AFA_FAILURE;

	//TODO: enhance error codes. Contain invalid matches (pipeline enhancement) 
	if( (result = of12_add_flow_entry_table(lsw->pipeline, table_id, flow_entry, check_overlap, reset_counts)) != ROFL_OF12_FM_SUCCESS){

		if(result == ROFL_OF12_FM_OVERLAP)
			return AFA_FM_OVERLAP_FAILURE;
		
		return AFA_FAILURE;
	}

	if(buffer_id != OF12P_NO_BUFFER){
	
		datapacket_t* pkt = datapacket_storage_get_packet_wrapper(((struct logical_switch_internals*)lsw->platform_state)->store_handle, buffer_id);
	
		if(!pkt){
			assert(0);
			return AFA_FAILURE; //TODO: return really failure?
		}

		of_process_packet_pipeline((of_switch_t*)lsw,pkt);
	}


	//FIXME: delete this
	of12_dump_table(&lsw->pipeline->tables[table_id]);
	
	
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

	of12_switch_t* lsw;

	//Recover port	
	lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);


	if(table_id >= lsw->pipeline->num_of_tables)
		return AFA_FAILURE;

	if(of12_modify_flow_entry_table(lsw->pipeline, table_id, flow_entry, strictness, reset_counts) != ROFL_SUCCESS)
		return AFA_FAILURE;
	
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

	of12_switch_t* lsw;
	unsigned int i;

	//Recover port	
	lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);


	if(table_id >= lsw->pipeline->num_of_tables && table_id != OF12_FLOW_TABLE_ALL)
		return AFA_FAILURE;


	if(table_id == OF12_FLOW_TABLE_ALL){
		//Single table
		for(i = 0; i<lsw->pipeline->num_of_tables; i++){
			if(of12_remove_flow_entry_table(lsw->pipeline, i, flow_entry, strictness, out_port, out_group) != ROFL_SUCCESS)
			return AFA_FAILURE;
		}	
	}else{
		//Single table
		if(of12_remove_flow_entry_table(lsw->pipeline, table_id, flow_entry, strictness, out_port, out_group) != ROFL_SUCCESS)
			return AFA_FAILURE;
	}
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

	of12_switch_t* lsw;

	//Recover port	
	lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);


	if(table_id >= lsw->pipeline->num_of_tables && table_id != OF12_FLOW_TABLE_ALL)
		return NULL; 


	return of12_get_flow_stats(lsw->pipeline, table_id, cookie, cookie_mask, out_port, out_group, matchs);
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

	of12_switch_t* lsw;

	//Recover port	
	lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);


	if(table_id >= lsw->pipeline->num_of_tables && table_id != OF12_FLOW_TABLE_ALL)
		return NULL; 


	return of12_get_flow_aggregate_stats(lsw->pipeline, table_id, cookie, cookie_mask, out_port, out_group, matchs);
} 
/**
 * @name    fwd_module_of12_group_mod_add
 * @brief   Instructs driver to add a new GROUP
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the GROUP
 */
rofl_of12_gm_result_t fwd_module_of12_group_mod_add(uint64_t dpid, of12_group_type_t type, uint32_t id, of12_bucket_list_t *buckets){
	
	of12_switch_t* lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	return of12_group_add(lsw->pipeline->groups, type, id, buckets);
}

/**
 * @name    fwd_module_of12_group_mod_modify
 * @brief   Instructs driver to modify the GROUP with identification ID
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to modify the GROUP
 */
rofl_of12_gm_result_t fwd_module_of12_group_mod_modify(uint64_t dpid, of12_group_type_t type, uint32_t id, of12_bucket_list_t *buckets){
	
	of12_switch_t* lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	return of12_group_modify(lsw->pipeline->groups, type, id, buckets);
}

/**
 * @name    fwd_module_of12_group_mod_del
 * @brief   Instructs driver to delete the GROUP with identification ID
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to delete the GROUP
 */
rofl_of12_gm_result_t fwd_module_of12_group_mod_delete(uint64_t dpid, uint32_t id){
	
	of12_switch_t* lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	return of12_group_delete(lsw->pipeline, lsw->pipeline->groups, id);
}

/**
 * @name    fwd_module_of12_group_search
 * @brief   Instructs driver to search the GROUP with identification ID
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to search the GROUP
 */
afa_result_t fwd_module_of12_fetch_group_table(uint64_t dpid, of12_group_table_t *group_table){
	
	of12_switch_t* lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(of12_fetch_group_table(lsw->pipeline,group_table)!=ROFL_SUCCESS)
		return AFA_FAILURE;
	
	return AFA_SUCCESS;
}

/**
 * @name    fwd_module_of12_get_group_stats
 * @brief   Instructs driver to fetch the GROUP statistics
 * @ingroup of12_fwd_module_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch where the GROUP is
 */
of12_stats_group_msg_t * fwd_module_of12_get_group_stats(uint64_t dpid, uint32_t id){
	
	of12_switch_t* lsw = (of12_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	return of12_get_group_stats(lsw->pipeline,id);
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
