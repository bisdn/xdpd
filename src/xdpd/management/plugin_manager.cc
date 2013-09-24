#include "plugin_manager.h"
#include <rofl/common/utils/c_logger.h>

/* Plugin header inclusion. THEY MUST BE CONDITIONALLY INCLUDED ALWAYS*/
#ifdef WITH_MGMT_CONFIG
	#include "plugins/config/config.h"
#endif
#ifdef WITH_MGMT_CLI
	#include "plugins/cli/xdpd_cli.h"
#endif

#ifdef WITH_MGMT_QMF
	#include "plugins/cli/qmfagent.h"
#endif
//Add more here [+]...


/*
*
*/

using namespace xdpd;
using namespace std;

//Static initialization
std::vector<plugin*> plugin_manager::plugins;


//Register compiled plugins. ALWAYS conditionally!
void plugin_manager::pre_init(){

#ifdef WITH_MGMT_CONFIG
	//Register CONFIG 
	register_plugin(new config());	
#endif
	
#ifdef WITH_MGMT_CLI
	//Register CLI
	register_plugin(new xdpd_cli());	
#endif
	
#ifdef WITH_MGMT_QMF
	//Register QMF
	register_plugin(new qmf());	
#endif

//Add more here [+]...
	
}

rofl_result_t plugin_manager::init(int argc, char** argv){

	ROFL_DEBUG("Initializing Plugin Manager\n");

	//Call register
	plugin_manager::pre_init();

	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		ROFL_INFO("Initializing plugin [%s]...\n", (*it)->get_name().c_str());
		(*it)->init(argc,argv);
	}

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
