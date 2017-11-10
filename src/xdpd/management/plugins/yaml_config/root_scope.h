#ifndef XDPD_PLUGINS_YAML_CONFIG_ROOT_SCOPE_H
#define XDPD_PLUGINS_YAML_CONFIG_ROOT_SCOPE_H

#include <iostream>
#include <yaml-cpp/yaml.h>

#include "xdpd/common/exception.h"

#include "scope.h"

/**
* @file root_scope.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Root node of the configuration
*
*/

namespace xdpd {
namespace plugins {
namespace yaml_config {

class root_scope : public scope {

public:
	root_scope();
	virtual ~root_scope();

private:

};

class config_scope : public scope {

public:
	config_scope(scope* parent);
	virtual ~config_scope();

private:

};

}// namespace yaml_config
}// namespace plugins
}// namespace xdpd

#endif /* XDPD_PLUGINS_YAML_CONFIG_ROOT_SCOPE_H */


