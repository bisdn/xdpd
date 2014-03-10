#include "example.h"
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;

void example::init(int args, char** argv){
	//DO something
	ROFL_INFO("[plugin_example]This is the init function of the example plugin. It won't do anything beyond printing this trace.\n\n");	
	ROFL_INFO("[plugin_example]If you have compiled xdpd with this plugin only, then this xdpd execution is pretty useless, since no Logical Switch Instances will be created and there will be no way (RPC) to create them... But hey, what do you expect, it is just an example plugin!\n\n");	
	ROFL_INFO("[plugin_example]You may now press Ctrl+C to finish xdpd execution.\n");
};

//Events; print nice traces
void example::notify_port_add(const switch_port_snapshot_t* port_snapshot){
	ROFL_INFO("[plugin_example] Got a port add event for port %s, admin status: %s, link %s\n", port_snapshot->name, (port_snapshot->up)?"UP":"DOWN", ((port_snapshot->state & PORT_STATE_LINK_DOWN) > 0)? "NOT DETECTED":"DETECTED");
}
		
void example::notify_port_status_changed(const switch_port_snapshot_t* port_snapshot){
	ROFL_INFO("[plugin_example] Got a port status changed event for port %s, admin status: %s, link %s\n", port_snapshot->name, (port_snapshot->up)?"UP":"DOWN", ((port_snapshot->state & PORT_STATE_LINK_DOWN) > 0)? "NOT DETECTED":"DETECTED");
}
	
void example::notify_port_delete(const switch_port_snapshot_t* port_snapshot){
	ROFL_INFO("[plugin_example] Got a port delete event for port %s, admin status: %s, link %s\n", port_snapshot->name, (port_snapshot->up)?"UP":"DOWN", ((port_snapshot->state & PORT_STATE_LINK_DOWN) > 0)? "NOT DETECTED":"DETECTED");
}
	
void example::notify_monitoring_state_changed(const monitoring_snapshot_state_t* monitoring_snapshot){

}



