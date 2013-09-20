#include "plugin_manager.h"

using namespace xdpd;
using namespace std;

//Static initialization
std::vector<plugin*> plugin_manager::plugins;


rofl_result_t plugin_manager::init(int args, char** argv){

#if 0
	ROFL_INFO("Initializing Plugin Manager\n");

	for(std::vector<plugin&>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		ROFL_INFO("Initializing plugin [%s]...\n",plugin.get_name().c_str());
		it.init(args,argv);
	}
#endif	
	return ROFL_SUCCESS;
}
	
void plugin_manager::register_plugin(plugin* p){
	plugins.push_back(p);	
}
