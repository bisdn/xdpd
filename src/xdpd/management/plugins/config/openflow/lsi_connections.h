#ifndef CONFIG_OPENFLOW_CONN_PLUGIN_H
#define CONFIG_OPENFLOW_CONN_PLUGIN_H 

#include <vector>
#include <libconfig.h++>
#include "xdpd/common/csocket.h"
#include "xdpd/common/cparams.h"
#include "../config.h"

/**
* @file lsi_connection_utils.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Parse LSI connection utils 
* 
*/

namespace xdpd {

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
	
	virtual void pre_validate(libconfig::Setting& setting, bool dry_run);
	void get_connection_values(libconfig::Setting& setting, std::string& hostname);
	void parse_connection_params(libconfig::Setting& setting, lsi_connection& con);
	void parse_ssl_connection_params(libconfig::Setting& setting, lsi_connection& con, bool dry_run);
	lsi_connection parse_connection(libconfig::Setting& setting, bool dry_run);
};

}// namespace xdpd 

#endif /* CONFIG_OPENFLOW_CONN_PLUGIN_H_ */


