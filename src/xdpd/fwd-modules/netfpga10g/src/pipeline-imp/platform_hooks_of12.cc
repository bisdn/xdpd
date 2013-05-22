#include <assert.h>
#include <rofl/datapath/pipeline/openflow/openflow12/of12_async_events_hooks.h>
#include <rofl/datapath/pipeline/openflow/openflow12/of12_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_flow_table.h>
#include <rofl/datapath/afa/openflow/openflow12/of12_cmm.h>
#include <rofl/common/utils/c_logger.h>

#include "../netfpga/netfpga.h" 
#include "../netfpga/stats.h" 

#define FWD_MOD_NAME "netfpga10g"
/*
* Hooks for configuration of the switch
*/
rofl_result_t platform_post_init_of12_switch(of12_switch_t* sw){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);

	return ROFL_SUCCESS;
}

rofl_result_t platform_pre_destroy_of12_switch(of12_switch_t* sw){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
	return ROFL_SUCCESS;
}


//Async notifications

/*
* Packet in
*/

void platform_of12_packet_in(const of12_switch_t* sw, uint8_t table_id, datapacket_t* pkt, of_packet_in_reason_t reason)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

//Flow removed
void platform_of12_notify_flow_removed(const of12_switch_t* sw, 	
						of12_flow_remove_reason_t reason, 
						of12_flow_entry_t* removed_flow_entry){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);

}


void plaftorm_of12_add_entry_hook(of12_flow_entry_t* new_entry){

	//Lock netfpga
	netfpga_lock();
	
	//Add entry
	netfpga_add_flow_entry(new_entry);
	
	//Release lock
	netfpga_unlock();

}

void platform_of12_modify_entry_hook(of12_flow_entry_t* old_entry, of12_flow_entry_t* mod, int reset_count){

	//TODO: Not implemented

	#if 0
	//Lock netfpga
	netfpga_lock();

	//Remove old one
	netfpga_delete_flow_entry(entry);

	//Do stuff...
	
	//Add entry
	netfpga_add_flow_entry(mod);
	
	//Release lock
	netfpga_unlock();
	
	#endif
	
}

void platform_of12_remove_entry_hook(of12_flow_entry_t* entry){

	//Lock netfpga
	netfpga_lock();
	
	//Add entry
	netfpga_delete_flow_entry(entry);
	
	//Release lock
	netfpga_unlock();
}

void platform_of12_update_stats_hook(of12_flow_entry_t* entry){

	//Lock netfpga
	netfpga_lock();
	
	//Add entry
	netfpga_update_entry_stats(entry);
	
	//Release lock
	netfpga_unlock();

}
