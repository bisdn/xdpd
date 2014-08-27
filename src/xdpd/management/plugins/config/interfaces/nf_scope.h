#ifndef CONFIG_NF_PLUGIN_H
#define CONFIG_NF_PLUGIN_H 

#include <string>
#include <set>
#include "../scope.h"

/**
* @file nf_scope.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NF Interfaces libconfig file hierarchy 
* 
*/

namespace xdpd {

class nf_scope:public scope {
	
public:
	nf_scope(std::string scope_name="nf", bool mandatory=false);
	
protected:

	virtual void __pre_execute(libconfig::Setting& setting, bool dry_run);

	std::set<std::string> nf_interfaces;
};

}// namespace xdpd 

#endif /* CONFIG_NF_PLUGIN_H_ */


