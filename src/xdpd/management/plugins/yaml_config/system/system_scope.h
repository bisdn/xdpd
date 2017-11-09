#ifndef XDPD_PLUGINS_YAML_CONFIG_SYSTEM_SCOPE_H
#define XDPD_PLUGINS_YAML_CONFIG_SYSTEM_SCOPE_H

#include "../scope.h"

/**
* @file system_scope.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief System libconfig file hierarchy 
* 
*/

namespace xdpd {
namespace plugins {
namespace yaml_config {

class system_scope:public scope {
	
public:
	system_scope(scope* parent);
		
	virtual void post_validate(YAML::Node& setting, bool dry_run);
	
	static std::string get_driver_extra_params(YAML::Node& cfg);
};

}// namespace yaml_config
}// namespace plugins
}// namespace xdpd 

#endif /* XDPD_PLUGINS_YAML_CONFIG_SYSTEM_SCOPE_H */


