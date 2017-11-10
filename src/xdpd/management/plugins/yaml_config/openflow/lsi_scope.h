#ifndef XDPD_PLUGINS_YAML_CONFIG_LSI_SCOPE_H
#define XDPD_PLUGINS_YAML_CONFIG_LSI_SCOPE_H

#include <map>
#include <rofl/common/caddress.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>

#include "xdpd/common/csocket.h"
#include "xdpd/common/cparams.h"

#include "../scope.h"

/**
* @file lsi_scope.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Openflow libconfig file hierarchy
*
*/

namespace xdpd {
namespace plugins {
namespace yaml_config {

class lsi_scope:public scope {

public:
	lsi_scope(std::string scope_name, scope* parent);

protected:
	virtual void post_validate(YAML::Node& setting, bool dry_run);

	//Parsing routines
	void parse_version(YAML::Node& setting, of_version_t* version);
	void parse_reconnect_time(YAML::Node& setting, unsigned int* reconnect_time);
	void parse_pirl(YAML::Node& setting, bool* pirl_enabled, int* pirl_rate);
	void parse_active_connections(YAML::Node& setting, std::string& master_controller, int& master_controller_port, std::string& slave_controller, int& slave_controller_port);
	void parse_tables(YAML::Node& setting, of_version_t version, int* ma_list, bool dry_run);
	void parse_ports(YAML::Node& setting, std::vector<std::string>& ports, bool dry_run);
	enum xdpd::csocket::socket_type_t parse_socket(YAML::Node& setting, xdpd::cparams& socket_params);
private:
	void parse_ip(rofl::caddress& addr, std::string& ip, unsigned int port);

	std::map<std::string, std::string> tables;
};

}// namespace yaml_config
}// namespace plugins
}// namespace xdpd

#endif /* XDPD_PLUGINS_YAML_CONFIG_LSI_SCOPE_H */


