#ifndef CONFIG_INTERFACES_PLUGIN_H
#define CONFIG_INTERFACES_PLUGIN_H 

#include <string>
#include <set>
#include "../scope.h"

/**
* @file interfaces_scope.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Interfaces libconfig file hierarchy 
* 
*/

namespace xdpd {

class interfaces_scope:public scope {
	
public:
	interfaces_scope(scope* parent);
	
	//This is cached during dry-runs
	bool is_blacklisted(std::string& port_name){
		return blacklisted.find(port_name) != blacklisted.end();
	}
protected:
	virtual void __pre_execute(libconfig::Setting& setting, bool dry_run);
	std::set<std::string> blacklisted;
};

class virtual_ifaces_scope:public scope {
	
public:
	virtual_ifaces_scope(scope* parent);
		
protected:
	
	virtual void post_validate(libconfig::Setting& setting, bool dry_run);
};

}// namespace xdpd 

#endif /* CONFIG_INTERFACES_PLUGIN_H_ */


