#include "root_scope.h"
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/utils/c_logger.h>

//sub scopes
#include "openflow/openflow_scope.h" 

using namespace xdpd;
using namespace rofl;
using namespace libconfig; 


root_scope::root_scope():scope("Root"){
	
	//Interfaces subhierarchy
	
	//Openflow subhierarchy
	register_subscope("openflow", new openflow_scope());	
}

root_scope::~root_scope(){
	//Remove all objects
}
