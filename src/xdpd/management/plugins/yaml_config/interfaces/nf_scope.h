#ifndef XDPD_PLUGINS_YAML_CONFIG_NF_SCOPE_H
#define XDPD_PLUGINS_YAML_CONFIG_NF_SCOPE_H

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
namespace plugins {
namespace yaml_config {

class nf_scope:public scope {

public:
	nf_scope(scope* parent);

	bool is_nf_port(std::string& port_name){
		return nf_interfaces.find(port_name) != nf_interfaces.end();
	}
protected:

	virtual void post_validate(YAML::Node& node, bool dry_run);

	std::set<std::string> nf_interfaces;
};

}// namespace yaml_config
}// namespace plugins
}// namespace xdpd

#endif /* XDPD_PLUGINS_YAML_CONFIG_NF_SCOPE_H */


