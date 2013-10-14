#include <assert.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_async_events_hooks.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_table.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>
#include <rofl/common/utils/c_logger.h>

/*
* Hooks for configuration of the switch
*/
rofl_result_t platform_post_init_of1x_switch(of1x_switch_t* sw){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);

	return ROFL_SUCCESS;
}

rofl_result_t platform_pre_destroy_of1x_switch(of1x_switch_t* sw){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
	return ROFL_SUCCESS;
}


//Async notifications

/*
* Packet in
*/

void platform_of1x_packet_in(const of1x_switch_t* sw, uint8_t table_id, datapacket_t* pkt, of_packet_in_reason_t reason)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

//Flow removed
void platform_of1x_notify_flow_removed(const of1x_switch_t* sw, 	
						of1x_flow_remove_reason_t reason, 
						of1x_flow_entry_t* removed_flow_entry){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);

}


void plaftorm_of1x_add_entry_hook(of1x_flow_entry_t* new_entry){

}

void platform_of1x_modify_entry_hook(of1x_flow_entry_t* old_entry, of1x_flow_entry_t* mod, int reset_count){

}

void platform_of1x_remove_entry_hook(of1x_flow_entry_t* entry){

}

void platform_of1x_update_stats_hook(of1x_flow_entry_t* entry) {

}
