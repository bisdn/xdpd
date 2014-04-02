#include "plugin_manager.h"
#include <assert.h>
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;
using namespace std;

//Static initialization
std::vector<plugin*> plugin_manager::plugins;

//Getopt 
extern int optind;

rofl_result_t plugin_manager::init(){

	ROFL_DEBUG("[xdpd][plugin_manager] Initializing Plugin Manager\n");

	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		ROFL_INFO("[xdpd][plugin_manager] Loading plugin (%s)...\n", (*it)->get_name().c_str());
		(*it)->init();
		optind=0; //Reset getopt
	}

	ROFL_INFO("[xdpd][plugin_manager] All plugins loaded.\n");
	
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
		try{
			(*it)->notify_port_added(port_snapshot);
 		}catch(...){
			ROFL_ERR("[xdpd][plugin_manager] ERROR: uncaught exception throw by plugin [%s] thrown during callback of %s. This is a bug in the plugin code, please contact the mantainer of the plugin...\n", (*it)->get_name().c_str(), __func__);
			assert(0);
			//Continue with the rest of the plugins
		}
	}
}	
	
/*
* Notify plugins of an attachment of a port to a LSI
* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_port_attached(const switch_port_snapshot_t* port_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		try{
			(*it)->notify_port_attached(port_snapshot);
 		}catch(...){
			ROFL_ERR("[xdpd][plugin_manager] ERROR: uncaught exception throw by plugin [%s] thrown during callback of %s. This is a bug in the plugin code, please contact the mantainer of the plugin...\n", (*it)->get_name().c_str(), __func__);
			assert(0);
			//Continue with the rest of the plugins
		}
	}
}

/*
* Notify plugins of a change in the state of a system port 
* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_port_status_changed(const switch_port_snapshot_t* port_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		try{
			(*it)->notify_port_status_changed(port_snapshot); 
		}catch(...){
			ROFL_ERR("[xdpd][plugin_manager] ERROR: uncaught exception throw by plugin [%s] thrown during callback of %s. This is a bug in the plugin code, please contact the mantainer of the plugin...\n", (*it)->get_name().c_str(), __func__);
			assert(0);
			//Continue with the rest of the plugins
		}
	}
}	

/*
* Notify plugins of a detachment of a port from an LSI
* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_port_detached(const switch_port_snapshot_t* port_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		try{
			(*it)->notify_port_detached(port_snapshot); 
		}catch(...){
			ROFL_ERR("[xdpd][plugin_manager] ERROR: uncaught exception throw by plugin [%s] thrown during callback of %s. This is a bug in the plugin code, please contact the mantainer of the plugin...\n", (*it)->get_name().c_str(), __func__);
			assert(0);
			//Continue with the rest of the plugins
		}
	}

}

/*
* Notify plugins of the deletion of a port of the system
* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_port_deleted(const switch_port_snapshot_t* port_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		try{
			(*it)->notify_port_deleted(port_snapshot); 
		}catch(...){
			ROFL_ERR("[xdpd][plugin_manager] ERROR: uncaught exception throw by plugin [%s] thrown during callback of %s. This is a bug in the plugin code, please contact the mantainer of the plugin...\n", (*it)->get_name().c_str(), __func__);
			assert(0);
			//Continue with the rest of the plugins
		}
	}

}	

/**
* Notify plugins of a changed in the monitoring state of the platform
* @warning: monitoring_snapshot MUST NOT be written or destroyed, use monitoring_clone_snapshot() in case of need.
*/
void plugin_manager::__notify_monitoring_state_changed(const monitoring_snapshot_state_t* monitoring_snapshot){

	//Distribute event
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		try{
			(*it)->notify_monitoring_state_changed(monitoring_snapshot); 
		}catch(...){
			ROFL_ERR("[xdpd][plugin_manager] ERROR: uncaught exception throw by plugin [%s] thrown during callback of %s. This is a bug in the plugin code, please contact the mantainer of the plugin...\n", (*it)->get_name().c_str(), __func__);
			assert(0);
			//Continue with the rest of the plugins
		}
	}
}

/**
* Get plugin specific command line options. This shall only be called by system_manager
*/	
std::vector<rofl::coption> plugin_manager::__get_plugin_options(void){
	
	std::vector<rofl::coption> vec;
	std::vector<rofl::coption> plugin_vec;

	//Call register
	plugin_manager::pre_init();

	//Go through all plugins
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {

		//Retrieve the options specific to the plugin
		plugin_vec = (*it)->get_options();
	
		for(std::vector<rofl::coption>::iterator it2 = plugin_vec.begin(); it2 != plugin_vec.end(); ++it2)
			vec.push_back(*it2);
	}
	
	return vec;
}

//Get driver extra parameters
std::string plugin_manager::__get_driver_extra_params(){
	
	std::string extra = "";
	std::string tmp; 

	//Go through all plugins (backwards), highest priority first
	for(std::vector<plugin*>::reverse_iterator rit = plugins.rbegin(); rit!= plugins.rend(); ++rit){

		tmp = (*rit)->get_driver_extra_params();

		//Retrieve extra if any
		if( tmp != "") 
			extra = tmp;
	}
	
	return extra;
}
