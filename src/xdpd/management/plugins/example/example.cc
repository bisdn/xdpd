#include "example.h"
#include <sstream>
#include <rofl/common/utils/c_logger.h>
#include "../../plugin_manager.h"
#include "../../switch_manager.h"

using namespace xdpd;

#define PLUGIN_NAME "example_plugin" 

//Static initializations
bool example::dump = false;

void sigusr1_callback(int signum){
	example::dump = true;	
}

void example::handle_timeout(int opaque, void *data){

	unsigned int j,k;
	
	if(example::dump){
		std::stringstream ss;

		//print some nice headers
		ss << "\n\n[xdpd]["PLUGIN_NAME"] **************************\n";	
		ss << "[xdpd]["PLUGIN_NAME"] Received SIGUSR1; dumping state...\n";


		//Get the list of lsis
		std::list<std::string> list = switch_manager::list_sw_names();
		std::list<flow_entry_snapshot> flows;
		openflow_switch_snapshot sw;
		std::list<openflow_group_mod_snapshot> group_mods;

		for(std::list<std::string>::iterator it = list.begin(); it != list.end(); ++it){

			//Get snapshot
			switch_manager::get_switch_info(switch_manager::get_switch_dpid(*it), sw);		
	
			//Output this switch (headeR)
			ss << sw;			

			//For each table in the switch
			std::list<openflow_switch_table_snapshot>::iterator table_it = sw.tables.begin();

			for(j=0;j<sw.num_of_tables; ++j, ++table_it){
				
				//Get the flows
				switch_manager::get_switch_table_flows(sw.dpid, j, flows);

				ss << *table_it; 

				//Dump
				std::list<flow_entry_snapshot>::iterator f_it = flows.begin();
				for(k=0; f_it != flows.end(); ++f_it, ++k)
					ss <<"\t\t["<<k<<"]"<< *f_it;
			}
			
			//Dump Group table info
			ss << sw.group_table;

			//Get group table entries
			switch_manager::get_switch_group_mods(switch_manager::get_switch_dpid(*it), group_mods);
			
			//Dump
			std::list<openflow_group_mod_snapshot>::iterator g_it = group_mods.begin();
			for(j=0; g_it != group_mods.end(); ++g_it, ++j)
				ss << "\t\t[group-mod:" <<j<< " {" << *g_it;
			
		}
		
		//Ending trace
		ss << "[xdpd]["PLUGIN_NAME"] **************************\n\n";	

		//Perform atomic output
		std::cout<<ss.str();		

		//Mark as 	
		example::dump = false;
	}

	//Reprogram timer
	register_timer(1,1);
}

void example::init(){

	//DO something
	ROFL_INFO("\n\n[xdpd]["PLUGIN_NAME"] **************************\n");	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] This is the init function of the example plugin. This plugin won't do anything beyond printing this trace and other port notification traces.\n");
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] **************************\n\n");	

	if(plugin_manager::get_plugins().size() == 1){

		ROFL_INFO("[xdpd]["PLUGIN_NAME"] You have compiled xdpd with this plugin only. This xdpd execution is pretty useless, since no Logical Switch Instances will be created and there will be no way (RPC) to create them... But hey, what do you expect, it is just an example plugin!\n\n");	
		ROFL_INFO("[xdpd]["PLUGIN_NAME"]You may now press Ctrl+C to finish xdpd execution.\n");
	}

	//Register timer
	register_timer(1,1);

	//Register signal handler
	signal(SIGUSR1, sigusr1_callback);
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



