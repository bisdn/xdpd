#ifndef CONFIG_INTERFACES_PLUGIN_H
#define CONFIG_INTERFACES_PLUGIN_H 

#include "../scope.h"

/**
* @file openflow_scope.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Openflow libconfig file hierarchy 
* 
*/

namespace xdpd {

class interfaces_scope:public scope {
	
public:
	interfaces_scope(std::string scope_name="interfaces", bool mandatory=false);
		
protected:
	
};

class virtual_ifaces_scope:public scope {
	
public:
	virtual_ifaces_scope(std::string scope_name="virtual", bool mandatory=false);
		
protected:
	
	virtual void post_validate(libconfig::Setting& setting, bool dry_run);
};

}// namespace xdpd 

#endif /* CONFIG_INTERFACES_PLUGIN_H_ */


