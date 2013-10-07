#include "example.h"
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;

void example::init(int args, char** argv){
	//DO something
	ROFL_INFO("This is the init function of the example plugin. It won't do anything beyond printing this trace.\n\n");	
	ROFL_INFO("If you have compiled xdpd with this plugin only, then this xdpd execution is pretty useless, since no Logical Switch Instances will be created and there will be no way (RPC) to create them... But hey, what do you expect, it is just an example plugin!\n\n");	
	ROFL_INFO("You may now press Ctrl+C to finish xdpd execution.\n");
};

