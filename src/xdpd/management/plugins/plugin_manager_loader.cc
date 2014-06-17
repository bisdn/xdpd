#include "../plugin_manager.h"
#include <rofl/common/utils/c_logger.h>
#include "pm_timestamp.h" //Regenerated every time configure script is launched

//Convenience
using namespace xdpd;

/*
* This file contains the initialization code for the plugins
* It must for every plugin:
*   1) Include the appropriate header CONDITIONALLY
*   2) Call register_plugin(new plugin_class_name()) CONDITIONALLY
*
* Note also that orders in the pre_init function matters, as the order
* of registration will be the exact same order of initialization
*/

//
//Plugin header inclusion. They must be conditionally included, ALWAYS!
//
#ifdef WITH_MGMT_CONFIG
	#include "config/config.h"
#endif
#ifdef WITH_MGMT_QMF
	#include "qmf/qmfagent.h"
#endif
#ifdef WITH_MGMT_XMP
	#include "xmp/xmp.h"
#endif
#ifdef WITH_MGMT_EXAMPLE
	#include "example/example.h"
#endif
#ifdef WITH_MGMT_REST
	#include "rest/rest.h"
#endif
//Add more here [+]...

//
//Register compiled plugins. ALWAYS conditionally!
//
void plugin_manager::pre_init(){

	//Config should always be the first one
	#ifdef WITH_MGMT_CONFIG
		//Register CONFIG 
		register_plugin(new config());	
	#endif
		
	#ifdef WITH_MGMT_QMF
		//Register QMF
		register_plugin(new qmfagent());
	#endif

	#ifdef WITH_MGMT_XMP
		//Register XMP
		register_plugin(new xdpd::mgmt::protocol::xmp());
	#endif

	#ifdef WITH_MGMT_REST
		//Register example plugin
		register_plugin(new rest());	
	#endif

	//Generally the example should be the last one...
	#ifdef WITH_MGMT_EXAMPLE
		//Register example plugin
		register_plugin(new example());	
	#endif
		
	//Add more here [+]...
	
}
