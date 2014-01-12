#include <iostream>

#include "rest.h"
#include <rofl/common/utils/c_logger.h>

#include "../../plugin_manager.h"

#include <boost/asio.hpp>

using namespace xdpd;

void rest::init(int args, char** argv){
	//DO something
	ROFL_INFO("This is the init function of the rest plugin. It won't do anything beyond printing this trace.\n\n");	
	ROFL_INFO("If you have compiled xdpd with this plugin only, then this xdpd execution is pretty useless, since no Logical Switch Instances will be created and there will be no way (RPC) to create them... But hey, what do you expect, it is just an rest plugin!\n\n");	
	ROFL_INFO("You may now press Ctrl+C to finish xdpd execution.\n");

  std::vector<plugin*> plugin_list = plugin_manager::get_plugins();

  for (std::vector<plugin*>::iterator i = plugin_list.begin(); i != plugin_list.end(); ++i)
    {
    ROFL_INFO("Plugin: %s\n", (*i)->get_name().c_str());
    }

  boost::asio::io_service iosvc;
};

