#ifndef CONFIG_SYSTEM_PLUGIN_H
#define CONFIG_SYSTEM_PLUGIN_H 

#include "../scope.h"

/**
* @file system_scope.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief System libconfig file hierarchy 
* 
*/

namespace xdpd {

class system_scope:public scope {
	
public:
	system_scope(std::string scope_name="system", bool mandatory=false);
		
	virtual void post_validate(libconfig::Setting& setting, bool dry_run);
};

}// namespace xdpd 

#endif /* CONFIG_SYSTEM_PLUGIN_H_ */


