#include "example.h"
#include <rofl/common/utils/c_logger.h>
#include "../../plugin_manager.h"

using namespace xdpd;

#define PLUGIN_NAME "example_plugin" 

void example::init(){
	//DO something
	ROFL_INFO("\n\n[xdpd]["PLUGIN_NAME"] **************************\n");	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] This is the init function of the example plugin. This plugin won't do anything beyond printing this trace and other port notification traces.\n");
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] **************************\n\n");	

	if(plugin_manager::get_plugins().size() == 1){

		ROFL_INFO("[xdpd]["PLUGIN_NAME"] You have compiled xdpd with this plugin only. This xdpd execution is pretty useless, since no Logical Switch Instances will be created and there will be no way (RPC) to create them... But hey, what do you expect, it is just an example plugin!\n\n");	
		ROFL_INFO("[xdpd]["PLUGIN_NAME"]You may now press Ctrl+C to finish xdpd execution.\n");
	}
};

//Events; print nice traces
void example::notify_port_added(const switch_port_snapshot_t* port_snapshot){
		if(port_snapshot->is_attached_to_sw)
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] The port %s has been ADDED to the system and attached to the switch with DPID: 0x%"PRIx64":%u. Administrative status: %s, link detected: %s\n", port_snapshot->name, port_snapshot->attached_sw_dpid, port_snapshot->of_port_num, (port_snapshot->up)? "UP":"DOWN", ((port_snapshot->state & PORT_STATE_LINK_DOWN) > 0)? "NO":"YES");
		else
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] The port %s has been ADDED to the system. Administrative status: %s, link detected: %s\n", port_snapshot->name, (port_snapshot->up)? "UP":"DOWN", ((port_snapshot->state & PORT_STATE_LINK_DOWN) > 0)? "NO":"YES");
}
	
void example::notify_port_attached(const switch_port_snapshot_t* port_snapshot){
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] The port %s has been ATTACHED to the switch with DPID: 0x%"PRIx64":%u.\n", port_snapshot->name, port_snapshot->attached_sw_dpid, port_snapshot->of_port_num);
}	

void example::notify_port_status_changed(const switch_port_snapshot_t* port_snapshot){
		if(port_snapshot->is_attached_to_sw)
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] The port %s attached to the switch with DPID: 0x%"PRIx64":%u has CHANGED ITS STATUS. Administrative status: %s, link detected: %s\n", port_snapshot->name, port_snapshot->attached_sw_dpid, port_snapshot->of_port_num, (port_snapshot->up)? "UP":"DOWN", ((port_snapshot->state & PORT_STATE_LINK_DOWN) > 0)? "NO":"YES");
		else
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] The port %s has CHANGED ITS STATUS. Administrative status: %s, link detected: %s\n", port_snapshot->name, (port_snapshot->up)? "UP":"DOWN", ((port_snapshot->state & PORT_STATE_LINK_DOWN) > 0)? "NO":"YES");
}
		
void example::notify_port_detached(const switch_port_snapshot_t* port_snapshot){
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] The port %s has been DETACHED from the switch with DPID: 0x%"PRIx64":%u.\n", port_snapshot->name, port_snapshot->attached_sw_dpid, port_snapshot->of_port_num);
}	

void example::notify_port_deleted(const switch_port_snapshot_t* port_snapshot){
		if(port_snapshot->is_attached_to_sw)
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] The port %s has been REMOVED from the system and detached from the switch with DPID: 0x%"PRIx64":%u.\n", port_snapshot->name, port_snapshot->attached_sw_dpid, port_snapshot->of_port_num);
		else
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] The port %s has been REMOVED from the system.\n", port_snapshot->name);
}
	
void example::notify_monitoring_state_changed(const monitoring_snapshot_state_t* monitoring_snapshot){
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Got an event of MONITORING STATE CHANGE\n");
}



