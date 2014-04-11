#include <rofl.h>
#include <assert.h>
#include <rofl/datapath/hal/openflow/openflow1x/of1x_driver.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_statistics.h>

//Make sure pipeline-imp are BEFORE _pp.h
//so that functions can be inlined
#include "../../../pipeline-imp/rte_atomic_operations.h"
#include "../../../pipeline-imp/lock.h"
#include "../../../pipeline-imp/packet.h"

//Now include pp headers
#include <rofl/datapath/pipeline/openflow/of_switch_pp.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline_pp.h>

#include "../../../config.h"
#include "../../../io/bufferpool.h"
#include "../../../io/dpdk_datapacket.h"
#include "../../../io/datapacket_storage.h"


#include <rte_memcpy.h>

extern rte_mempool *pool_direct;

using namespace xdpd::gnu_linux;

//Port config

/*
* Checks wheather the action group contains at least an action output
*/
static inline bool action_group_of1x_packet_in_contains_output(of1x_action_group_t* action_group){

	of1x_packet_action_t* action;
	
	for(action=action_group->head;action;action = action->next){
		if(action->type == OF1X_AT_OUTPUT)
			return true; 
	}
	
	return false;
}

/**
 * @name    hal_driver_of1x_set_port_drop_received_config
 * @brief   Instructs driver to modify port config state 
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 			Datapath ID of the switch 
 * @param port_num		Port number 	
 * @param drop_received		Drop packets received
 */
hal_result_t hal_driver_of1x_set_port_drop_received_config(uint64_t dpid, unsigned int port_num, bool drop_received){
	
	switch_port_t* port = physical_switch_get_port_by_num(dpid,port_num);

	if(!port)	
		return HAL_FAILURE;
		
	port->drop_received = drop_received;
	
	return HAL_SUCCESS;
}

/**
 * @name    hal_driver_of1x_set_port_no_flood_config
 * @brief   Instructs driver to modify port config state 
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 			Datapath ID of the switch 
 * @param port_num		Port number 	
 * @param no_flood		No flood allowed in port
 */
hal_result_t hal_driver_of1x_set_port_no_flood_config(uint64_t dpid, unsigned int port_num, bool no_flood){
	
	switch_port_t* port = physical_switch_get_port_by_num(dpid,port_num);

	if(!port)	
		return HAL_FAILURE;
		
	port->no_flood = no_flood;
	
	return HAL_SUCCESS;
}

/**
 * @name    hal_driver_of1x_set_port_forward_config
 * @brief   Instructs driver to modify port config state 
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 			Datapath ID of the switch 
 * @param port_num		Port number 	
 * @param forward		Forward packets
 */
hal_result_t hal_driver_of1x_set_port_forward_config(uint64_t dpid, unsigned int port_num, bool forward){
	
	switch_port_t* port = physical_switch_get_port_by_num(dpid,port_num);

	if(!port)	
		return HAL_FAILURE;
		
	port->forward_packets = forward;
	
	return HAL_SUCCESS;
}
/**
 * @name    hal_driver_of1x_set_port_generate_packet_in_config
 * @brief   Instructs driver to modify port config state 
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 			Datapath ID of the switch 
 * @param port_num		Port number 	
 * @param generate_packet_in	Generate packet in events for this port 
 */
hal_result_t hal_driver_of1x_set_port_generate_packet_in_config(uint64_t dpid, unsigned int port_num, bool generate_packet_in){
	
	switch_port_t* port = physical_switch_get_port_by_num(dpid,port_num);

	if(!port)	
		return HAL_FAILURE;
		
	port->of_generate_packet_in = generate_packet_in;
	
	return HAL_SUCCESS;
}

/**
 * @name    hal_driver_of1x_set_port_advertise_config
 * @brief   Instructs driver to modify port advertise flags 
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 			Datapath ID of the switch 
 * @param port_num		Port number 	
 * @param advertise		Bitmap advertised
 */
hal_result_t hal_driver_of1x_set_port_advertise_config(uint64_t dpid, unsigned int port_num, uint32_t advertise){

	switch_port_t* port = physical_switch_get_port_by_num(dpid,port_num);

	if(!port)	
		return HAL_FAILURE;
		
	port->advertised = (port_features_t)advertise;
	
	return HAL_SUCCESS;
}

/**
 * @name    hal_driver_of1x_set_pipeline_config
 * @brief   Instructs driver to process a PACKET_OUT event
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch 
 * @param flags		Capabilities bitmap (OF1X_CAP_FLOW_STATS, OF12_CAP_TABLE_STATS, ...)
 * @param miss_send_len	OF MISS_SEND_LEN
 */
hal_result_t hal_driver_of1x_set_pipeline_config(uint64_t dpid, unsigned int flags, uint16_t miss_send_len){
	
	of_switch_t* lsw;

	//Recover switch 
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);

	//Check switch and port
	if (!lsw ) {
		//TODO: log this... should never happen
		assert(0);
		return HAL_FAILURE;
	}	

	//Simply store the new config
	((of1x_switch_t*)lsw)->pipeline.capabilities = flags;
	((of1x_switch_t*)lsw)->pipeline.miss_send_len = miss_send_len;

	return HAL_SUCCESS;
}

/**
 * @name    hal_driver_of1x_set_table_config
 * @brief   Instructs driver to set table configuration(default action)
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch
 * @param table_id	Table ID or 0xFF for all 
 * @param miss_send_len Table miss config	
 */
hal_result_t hal_driver_of1x_set_table_config(uint64_t dpid, unsigned int table_id, of1x_flow_table_miss_config_t config){
	
	of1x_switch_t* lsw;
	unsigned int i;

	//Recover switch 
	lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);

	//Check switch and port
	if( !lsw || ( (table_id != OF1X_FLOW_TABLE_ALL) && (table_id >= lsw->pipeline.num_of_tables) ) ) {
		//TODO: log this... should never happen
		assert(0);
		return HAL_FAILURE;
	}	

	//Simply store the new config
	if( table_id == OF1X_FLOW_TABLE_ALL ){
		for( i=0; i < lsw->pipeline.num_of_tables; i++){
			lsw->pipeline.tables[i].default_action = config;
		}
	}else{
		lsw->pipeline.tables[table_id].default_action = config;
	}

	return HAL_SUCCESS;
}

/**
 * @name    hal_driver_of1x_process_packet_out
 * @brief   Instructs driver to process a PACKET_OUT event
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to process PACKET_OUT
 * @param buffer_id	Buffer ID
 * @param in_port 	Port IN
 * @param action_group 	Action group to apply
 * @param buffer		Pointer to the buffer
 * @param buffer_size	Buffer size
 */
hal_result_t hal_driver_of1x_process_packet_out(uint64_t dpid, uint32_t buffer_id, uint32_t in_port, of1x_action_group_t* action_group, uint8_t* buffer, uint32_t buffer_size)
{
	of_switch_t* lsw;
	datapacket_t* pkt;

	//Recover port	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);

	//Check switch and port
	if(!lsw || ((lsw->of_ver != OF_VERSION_10) && (lsw->of_ver != OF_VERSION_12) && (lsw->of_ver != OF_VERSION_13))) {
		//TODO: log this... should never happen
		assert(0);
		return HAL_FAILURE;
	}	
	
	//Avoid DoS. Check whether the action list contains an action ouput, otherwise drop, since the packet will never be freed
	if(!action_group_of1x_packet_in_contains_output(action_group)){

		if (OF1XP_NO_BUFFER != buffer_id) {
			pkt = ((datapacket_storage*)lsw->platform_state)->get_packet(buffer_id);
			if (NULL != pkt) {
				bufferpool::release_buffer(pkt);
			}
		}

		//FIXME: free action_group??
		return HAL_FAILURE; /*TODO add specific error */
	}
	
	//Recover pkt buffer if is stored. Otherwise pick a free buffer
	if( buffer_id && buffer_id != OF1XP_NO_BUFFER){
	
		//Retrieve the packet
		pkt = ((datapacket_storage*)lsw->platform_state)->get_packet(buffer_id);

		//Buffer has expired
		if(!pkt){
			return HAL_FAILURE; /* TODO: add specific error */
		}
	}else{
		//Retrieve a free buffer	
		pkt = bufferpool::get_free_buffer_nonblocking();

		if(!pkt){
			//No available buffers
			return HAL_FAILURE; /* TODO: add specific error */
		}	
	
		//Initialize the packet and copy
		struct rte_mbuf* mbuf = rte_pktmbuf_alloc(pool_direct);
		if(mbuf==NULL){
			ROFL_ERR("Error prependig packet to mbuf\n");
			return HAL_FAILURE;
		}
		rte_pktmbuf_append(mbuf, buffer_size);
		rte_memcpy(rte_pktmbuf_mtod(mbuf, uint8_t*), buffer, buffer_size);
		assert( rte_pktmbuf_pkt_len(mbuf) == buffer_size );
		
		init_datapacket_dpdk(((datapacket_dpdk_t*)pkt->platform_state), mbuf, lsw, in_port, 0, true, true);
		pkt->sw = lsw;
	}

	//Reclassify the packet
	datapacket_dpdk_t* pkt_dpdk = (datapacket_dpdk_t*)pkt->platform_state;
	classify_packet(pkt_dpdk->headers, get_buffer_dpdk(pkt_dpdk), get_buffer_length_dpdk(pkt_dpdk), in_port, 0);

	ROFL_DEBUG_VERBOSE("Getting packet out [%p]\n",pkt);	
	
	//Instruct pipeline to process actions. This may reinject the packet	
	of1x_process_packet_out_pipeline((of1x_switch_t*)lsw, pkt, action_group);
	
	return HAL_SUCCESS;
}

/**
 * @name    hal_driver_of1x_process_flow_mod
 * @brief   Instructs driver to process a FLOW_MOD event
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id to install the flowmod
 * @param flow_entry	Flow entry to be installed
 * @param buffer_id	Buffer ID
 * @param out_port 	Port to output
 * @param check_overlap	Check OVERLAP flag
 * @param check_counts	Check RESET_COUNTS flag
 */

hal_result_t hal_driver_of1x_process_flow_mod_add(uint64_t dpid, uint8_t table_id, of1x_flow_entry_t** flow_entry, uint32_t buffer_id, bool check_overlap, bool reset_counts){

	of1x_switch_t* lsw;
	rofl_of1x_fm_result_t result;

	//Recover port	
	lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);

	if(!lsw){
		assert(0);
		return HAL_FAILURE;
	}

	if(table_id >= lsw->pipeline.num_of_tables)
		return HAL_FAILURE;

	//TODO: enhance error codes. Contain invalid matches (pipeline enhancement) 
	if( (result = of1x_add_flow_entry_table(&lsw->pipeline, table_id, flow_entry, check_overlap, reset_counts)) != ROFL_OF1X_FM_SUCCESS){

		if(result == ROFL_OF1X_FM_OVERLAP)
			return HAL_FM_OVERLAP_FAILURE;
		
		return HAL_FAILURE;
	}

	if(buffer_id && buffer_id != OF1XP_NO_BUFFER){
	
		datapacket_t* pkt = ((datapacket_storage*)lsw->platform_state)->get_packet(buffer_id);
	
		if(!pkt){
			assert(0);
			return HAL_FAILURE; //TODO: return really failure?
		}

		of_process_packet_pipeline((of_switch_t*)lsw,pkt);
	}


#ifdef DEBUG
	of1x_dump_table(&lsw->pipeline.tables[table_id],false);
#endif
	
	return HAL_SUCCESS;
}

/**
 * @name    hal_driver_of1x_process_flow_mod_modify
 * @brief   Instructs driver to process a FLOW_MOD modify event
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id from which to modify the flowmod
 * @param flow_entry	Flow entry 
 * @param buffer_id	Buffer ID
 * @param strictness 	Strictness (STRICT NON-STRICT)
 * @param check_counts	Check RESET_COUNTS flag
 */
hal_result_t hal_driver_of1x_process_flow_mod_modify(uint64_t dpid, uint8_t table_id, of1x_flow_entry_t** flow_entry, uint32_t buffer_id, of1x_flow_removal_strictness_t strictness, bool reset_counts){

	of1x_switch_t* lsw;

	//Recover port	
	lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);

	if(!lsw){
		assert(0);
		return HAL_FAILURE;
	}

	if(table_id >= lsw->pipeline.num_of_tables)
		return HAL_FAILURE;

	if(of1x_modify_flow_entry_table(&lsw->pipeline, table_id, flow_entry, strictness, reset_counts) != ROFL_SUCCESS)
		return HAL_FAILURE;
	
	if(buffer_id && buffer_id != OF1XP_NO_BUFFER){
	
		datapacket_t* pkt = ((datapacket_storage*)lsw->platform_state)->get_packet(buffer_id);
	
		if(!pkt){
			assert(0);
			return HAL_FAILURE; //TODO: return really failure?
		}

		of_process_packet_pipeline((of_switch_t*)lsw,pkt);
	}


	return HAL_SUCCESS;
}


/**
 * @name    hal_driver_of1x_process_flow_mod_delete
 * @brief   Instructs driver to process a FLOW_MOD event
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id to install the flowmod
 * @param flow_entry	Flow entry to be installed
 * @param out_port 	Out port that entry must include
 * @param out_group 	Out group that entry must include	
 * @param strictness 	Strictness (STRICT NON-STRICT)
 */
hal_result_t hal_driver_of1x_process_flow_mod_delete(uint64_t dpid, uint8_t table_id, of1x_flow_entry_t* flow_entry, uint32_t out_port, uint32_t out_group, of1x_flow_removal_strictness_t strictness){

	of1x_switch_t* lsw;
	unsigned int i;

	//Recover port	
	lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);

	if(!lsw){
		assert(0);
		return HAL_FAILURE;
	}

	if(table_id >= lsw->pipeline.num_of_tables && table_id != OF1X_FLOW_TABLE_ALL)
		return HAL_FAILURE;


	if(table_id == OF1X_FLOW_TABLE_ALL){
		//Single table
		for(i = 0; i<lsw->pipeline.num_of_tables; i++){
			if(of1x_remove_flow_entry_table(&lsw->pipeline, i, flow_entry, strictness, out_port, out_group) != ROFL_SUCCESS)
			return HAL_FAILURE;
		}	
	}else{
		//Single table
		if(of1x_remove_flow_entry_table(&lsw->pipeline, table_id, flow_entry, strictness, out_port, out_group) != ROFL_SUCCESS)
			return HAL_FAILURE;
	}
	return HAL_SUCCESS;
} 

//
// Statistics
//

/**
 * @name    hal_driver_of1x_get_flow_stats
 * @brief   Recovers the flow stats given a set of matches 
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id to get the flows of 
 * @param cookie	Cookie to be applied 
 * @param cookie_mask	Mask for the cookie
 * @param out_port 	Out port that entry must include
 * @param out_group 	Out group that entry must include	
 * @param matches	Matches
 */
of1x_stats_flow_msg_t* hal_driver_of1x_get_flow_stats(uint64_t dpid, uint8_t table_id, uint32_t cookie, uint32_t cookie_mask, uint32_t out_port, uint32_t out_group, of1x_match_group_t* matches){
	
	of1x_switch_t* lsw;

	//Recover port	
	lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);

	if(!lsw){
		assert(0);
		return NULL;
	}
	
	if(table_id >= lsw->pipeline.num_of_tables && table_id != OF1X_FLOW_TABLE_ALL)
		return NULL; 

	return of1x_get_flow_stats(&lsw->pipeline, table_id, cookie, cookie_mask, out_port, out_group, matches);
}

 
/**
 * @name    hal_driver_of1x_get_flow_aggregate_stats
 * @brief   Recovers the aggregated flow stats given a set of matches 
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the FLOW_MOD
 * @param table_id 	Table id to get the flows of 
 * @param cookie	Cookie to be applied 
 * @param cookie_mask	Mask for the cookie
 * @param out_port 	Out port that entry must include
 * @param out_group 	Out group that entry must include	
 * @param matches	Matches
 */
of1x_stats_flow_aggregate_msg_t* hal_driver_of1x_get_flow_aggregate_stats(uint64_t dpid, uint8_t table_id, uint32_t cookie, uint32_t cookie_mask, uint32_t out_port, uint32_t out_group, of1x_match_group_t* matches){

	of1x_switch_t* lsw;

	//Recover port	
	lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);

	if(!lsw){
		assert(0);
		return NULL;
	}

	if(table_id >= lsw->pipeline.num_of_tables && table_id != OF1X_FLOW_TABLE_ALL)
		return NULL; 

	return of1x_get_flow_aggregate_stats(&lsw->pipeline, table_id, cookie, cookie_mask, out_port, out_group, matches);
} 

/**
 * @name    hal_driver_of1x_group_mod_add
 * @brief   Instructs driver to add a new GROUP
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the GROUP
 */
rofl_of1x_gm_result_t hal_driver_of1x_group_mod_add(uint64_t dpid, of1x_group_type_t type, uint32_t id, of1x_bucket_list_t** buckets){
	
	of1x_switch_t* lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(!lsw){
		assert(0);
		return ROFL_OF1X_GM_UNKGRP;
	}
	
	return of1x_group_add(lsw->pipeline.groups, type, id, buckets);
}

/**
 * @name    hal_driver_of1x_group_mod_modify
 * @brief   Instructs driver to modify the GROUP with identification ID
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the GROUP
 */
rofl_of1x_gm_result_t hal_driver_of1x_group_mod_modify(uint64_t dpid, of1x_group_type_t type, uint32_t id, of1x_bucket_list_t** buckets){
	
	of1x_switch_t* lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(!lsw){
		assert(0);
		return ROFL_OF1X_GM_UNKGRP;
	}
	
	return of1x_group_modify(lsw->pipeline.groups, type, id, buckets);
}

/**
 * @name    hal_driver_of1x_group_mod_del
 * @brief   Instructs driver to delete the GROUP with identification ID
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to install the GROUP
 */
rofl_of1x_gm_result_t hal_driver_of1x_group_mod_delete(uint64_t dpid, uint32_t id){
	
	of1x_switch_t* lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(!lsw){
		assert(0);
		return ROFL_OF1X_GM_UNKGRP;
	}
	
	return of1x_group_delete(&lsw->pipeline, lsw->pipeline.groups, id);
}

/**
 * @name    hal_driver_of1x_group_search
 * @brief   Instructs driver to search the GROUP with identification ID
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch to search the GROUP
 */
hal_result_t hal_driver_of1x_fetch_group_table(uint64_t dpid, of1x_group_table_t *group_table){
		
	of1x_switch_t* lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(!lsw){
		assert(0);
		return HAL_FAILURE;
	}
	
	if(of1x_fetch_group_table(&lsw->pipeline,group_table)!=ROFL_SUCCESS)
		return HAL_FAILURE;
	
	return HAL_SUCCESS;
}
/**
 * @name    hal_driver_of1x_get_group_stats
 * @brief   Instructs driver to fetch the GROUP statistics
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch where the GROUP is
 */
of1x_stats_group_msg_t * hal_driver_of1x_get_group_stats(uint64_t dpid, uint32_t id){

	of1x_switch_t* lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(!lsw){
		assert(0);
		return NULL;
	}
	
	return of1x_get_group_stats(&lsw->pipeline,id);
}

/**
 * @name    hal_driver_of1x_get_group_all_stats
 * @brief   Instructs driver to fetch the GROUP statistics from all the groups
 * @ingroup of1x_driver_async_event_processing
 *
 * @param dpid 		Datapath ID of the switch where the GROUPS are
 */
of1x_stats_group_msg_t * hal_driver_of1x_get_group_all_stats(uint64_t dpid, uint32_t id){
		
	of1x_switch_t* lsw = (of1x_switch_t*)physical_switch_get_logical_switch_by_dpid(dpid);

	if(!lsw){
		assert(0);
		return NULL;
	}

	return of1x_get_group_all_stats(&lsw->pipeline,id);
}
