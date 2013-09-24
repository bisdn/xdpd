#include "root_scope.h"
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/utils/c_logger.h>

//sub scopes
#include "openflow/openflow_scope.h" 
#include "interfaces/interfaces_scope.h" 

using namespace xdpd;
using namespace rofl;
using namespace libconfig; 

root_scope::root_scope():scope("root"){
	//config subhierarchy
	register_subscope(new config_scope());
}

root_scope::~root_scope(){
	//Remove all objects
}

config_scope::config_scope():scope("config", true){

	//Openflow subhierarchy
	register_subscope(new openflow_scope());
	
	//Interfaces subhierarchy
	register_subscope(new interfaces_scope());	
}

config_scope::~config_scope(){
	//Remove all objects
}
