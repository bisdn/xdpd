#ifndef XDPD_PLUGINS_YAML_CONFIG_OPENFLOW_SCOPE_H
#define XDPD_PLUGINS_YAML_CONFIG_OPENFLOW_SCOPE_H

#include "../scope.h"

/**
* @file openflow_scope.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Openflow libconfig file hierarchy
*
*/

namespace xdpd {
namespace plugins {
namespace yaml_config {

class openflow_scope:public scope {

public:
	openflow_scope(scope* parent);

protected:

};

class of_lsis_scope:public scope {

public:
	of_lsis_scope(scope* parent);

protected:

	virtual void pre_validate(YAML::Node& node, bool dry_run);
};

}// namespace yaml_config
}// namespace plugins
}// namespace xdpd

#endif /* XDPD_PLUGINS_YAML_CONFIG_OPENFLOW_SCOPE_H */


