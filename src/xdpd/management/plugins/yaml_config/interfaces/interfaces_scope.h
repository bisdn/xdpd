#ifndef XDPD_PLUGINS_YAML_CONFIG_INTERFACES_SCOPE_H
#define XDPD_PLUGINS_YAML_CONFIG_INTERFACES_SCOPE_H

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
namespace plugins {
namespace yaml_config {

class interfaces_scope:public scope {

public:
	interfaces_scope(scope* parent);

	//This is cached during dry-runs
	bool is_blacklisted(std::string& port_name){
		return blacklisted.find(port_name) != blacklisted.end();
	}
protected:
	virtual void post_validate(YAML::Node& node, bool dry_run);
	std::set<std::string> blacklisted;
};

class virtual_ifaces_scope:public scope {

public:
	virtual_ifaces_scope(scope* parent);

protected:

	virtual void post_validate(YAML::Node& setting, bool dry_run);
};

}// namespace yaml_config
}// namespace plugins
}// namespace xdpd

#endif /* XDPD_PLUGINS_YAML_CONFIG_INTERFACES_SCOPE_H */


