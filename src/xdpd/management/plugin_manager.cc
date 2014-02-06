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
		ROFL_INFO("[plugin_manager] Loading plugin [%s]...\n", (*it)->get_name().c_str());
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
