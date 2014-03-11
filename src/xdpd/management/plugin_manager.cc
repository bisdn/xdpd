#include "plugin_manager.h"
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;
using namespace std;

//Static initialization
std::vector<plugin*> plugin_manager::plugins;

//Getopt 
extern int optind;

rofl_result_t plugin_manager::init(int argc, char** argv){

	ROFL_DEBUG("[plugin_manager] Initializing Plugin Manager\n");

	//Call register
	plugin_manager::pre_init();

	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		ROFL_INFO("[plugin_manager] Loading plugin (%s)...\n", (*it)->get_name().c_str());
		(*it)->init(argc,argv);
		optind=0; //Reset getopt
	}

	ROFL_INFO("[plugin_manager] All plugins loaded.\n");
	
	return ROFL_SUCCESS;
}

plugin* plugin_manager::get_plugin_by_name(std::string name){
	
	//Find plugin by name
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		if((*it)->get_name() == name)
			return (*it);
	}
	
	return NULL;
}

rofl_result_t plugin_manager::destroy(){

	//Destroy all plugins
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		delete *it;
	}
	plugins.clear();
	
	return ROFL_SUCCESS;
}
	
void plugin_manager::register_plugin(plugin* p){
	plugins.push_back(p);	
}

//
// Events sub-API
//


/*
* Notify plugins of a new port on the system
* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_port_added(const switch_port_snapshot_t* port_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		(*it)->notify_port_added(port_snapshot); 
	}
}	
	
/*
* Notify plugins of an attachment of a port to a LSI
* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_port_attached(const switch_port_snapshot_t* port_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		(*it)->notify_port_attached(port_snapshot); 
	}
}

/*
* Notify plugins of a change in the state of a system port 
* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_port_status_changed(const switch_port_snapshot_t* port_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		(*it)->notify_port_status_changed(port_snapshot); 
	}
}	

/*
* Notify plugins of a detachment of a port from an LSI
* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_port_detached(const switch_port_snapshot_t* port_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		(*it)->notify_port_detached(port_snapshot); 
	}

}

/*
* Notify plugins of the deletion of a port of the system
* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_port_deleted(const switch_port_snapshot_t* port_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		(*it)->notify_port_deleted(port_snapshot); 
	}

}	

/**
* Notify plugins of a changed in the monitoring state of the platform
* @warning: monitoring_snapshot MUST NOT be written or destroyed, use monitoring_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_monitoring_state_changed(const monitoring_snapshot_state_t* monitoring_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		(*it)->notify_monitoring_state_changed(monitoring_snapshot); 
	}

}


