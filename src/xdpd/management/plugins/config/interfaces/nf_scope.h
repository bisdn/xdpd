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
	nf_scope(scope* parent);

	bool is_nf_port(std::string& port_name){
		return nf_interfaces.find(port_name) != nf_interfaces.end();
	}	
protected:

	virtual void post_validate(libconfig::Setting& setting, bool dry_run);

	std::set<std::string> nf_interfaces;
};

}// namespace xdpd 

#endif /* CONFIG_NF_PLUGIN_H_ */


