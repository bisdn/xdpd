#ifndef XDPD_PLUGINS_YAML_CONFIG_LSI_CONNECTIONS_H
#define XDPD_PLUGINS_YAML_CONFIG_LSI_CONNECTIONS_H

#include <vector>
#include <yaml-cpp/yaml.h>
#include "xdpd/common/csocket.h"
#include "xdpd/common/cparams.h"
#include "../yaml_config.h"

/**
* @file lsi_connection_utils.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Parse LSI connection utils
*
*/

namespace xdpd {
namespace plugins {
namespace yaml_config {

class lsi_connection{

public:
	xdpd::csocket::socket_type_t type;
	xdpd::cparams params;
};

class lsi_connection_scope:public scope {

public:
	lsi_connection_scope(std::string scope_name, scope* parent);
};

class lsi_connections_scope:public scope {

public:
	lsi_connections_scope(scope* parent);

	std::vector<lsi_connection> get_parsed_connections(void){ return parsed_connections;};

	static const std::string SCOPE_NAME;
protected:
	std::vector<lsi_connection> parsed_connections;

	virtual void pre_validate(YAML::Node& setting, bool dry_run);
	void get_connection_values(YAML::Node& setting, std::string& hostname);
	void parse_connection_params(YAML::Node& setting, lsi_connection& con);
	void parse_ssl_connection_params(YAML::Node& setting, lsi_connection& con, bool dry_run);
	lsi_connection parse_connection(YAML::Node& setting, bool dry_run);
};

}// namespace yaml_config
}// namespace plugins
}// namespace xdpd

#endif /* XDPD_PLUGINS_YAML_CONFIG_LSI_CONNECTIONS_H */


