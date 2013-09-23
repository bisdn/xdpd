#ifndef CONFIG_PLUGIN_H
#define CONFIG_PLUGIN_H 

#include <iostream>
#include "../../plugin_manager.h"
#include <libconfig.h++> 

/**
* @file config_plugin.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief libconfig based configuration plugin
* 
*/

namespace xdpd {

class config:public plugin {
	
public:
	virtual void init(int args, char** argv);
	
	virtual std::string get_name(void){
		return std::string("config");
	};
};

}// namespace xdpd 

#endif /* CONFIG_PLUGIN_H_ */


