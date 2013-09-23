#ifndef CONFIG_PLUGIN_H
#define CONFIG_PLUGIN_H 

#include <iostream>
#include <libconfig.h++> 
#include <rofl/common/cerror.h>
#include "../../plugin_manager.h"
#include "scope.h"

/**
* @file config_plugin.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief libconfig based configuration plugin
* 
*/

namespace xdpd {

class eConfParamNotFound: public rofl::cerror {};
class eConfFileNotFound: public rofl::cerror {};
class eConfParseError: public rofl::cerror {};

class config : public plugin, public scope {
	
public:
	config();
	virtual ~config();
		
	
	virtual void init(int args, char** argv);
	
	virtual std::string get_name(void){
		return std::string("config");
	};
};

}// namespace xdpd 

#endif /* CONFIG_PLUGIN_H_ */


