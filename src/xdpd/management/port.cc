#include "port.h"
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;

//System port admin commands 
void port::up(){
	if(fwd_module_enable_port(name.c_str()) != AFA_SUCCESS)
		throw ePortUnknownError();	

	ROFL_INFO("[port: %s] brought administratively up\n", name.c_str());
}

void port::down(){

	if(fwd_module_disable_port(name.c_str()) != AFA_SUCCESS)
		throw ePortUnknownError();	
	
	ROFL_INFO("[port: %s] brought administratively down\n", name.c_str());
}


