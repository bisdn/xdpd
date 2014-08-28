#include "root_scope.h"
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/utils/c_logger.h>

//sub scopes
#include "openflow/openflow_scope.h" 
#include "interfaces/interfaces_scope.h" 
#include "system/system_scope.h" 

using namespace xdpd;
using namespace rofl;
using namespace libconfig; 

root_scope::root_scope():scope("root", NULL){
	//config subhierarchy
	register_subscope(new config_scope(this));
}

root_scope::~root_scope(){
	//Remove all objects
}

config_scope::config_scope(scope* parent):scope("config", parent, true){

	//Openflow subhierarchy
	register_subscope(new openflow_scope(this));
	
	//Interfaces subhierarchy
	register_priority_subscope(new interfaces_scope(this), 1, false);	
	
	//System subhierarchy
	register_subscope(new system_scope(this));	
}

config_scope::~config_scope(){
	//Remove all objects
}
