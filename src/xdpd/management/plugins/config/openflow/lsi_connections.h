#ifndef CONFIG_OPENFLOW_CONN_PLUGIN_H
#define CONFIG_OPENFLOW_CONN_PLUGIN_H 

#include <vector>
#include <libconfig.h++>
#include <rofl/common/csocket.h>
#include <rofl/common/cparams.h>
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
	rofl::csocket::socket_type_t type;
	rofl::cparams params;	
};

class lsi_connections_scope:public scope {
	
public:
	lsi_connections_scope(std::string scope_name="controller-connections", bool mandatory=true);
	std::vector<lsi_connection> get_parsed_connections(void){ return parsed_connections;};
		
protected:
	std::vector<lsi_connection> parsed_connections;
	
	virtual void pre_validate(libconfig::Setting& setting, bool dry_run);
	void get_connection_values(libconfig::Setting& setting, std::string& hostname);
	void parse_connection_params(libconfig::Setting& setting, lsi_connection& con);
	void parse_ssl_connection_params(libconfig::Setting& setting, lsi_connection& con);
	lsi_connection parse_connection(libconfig::Setting& setting);
};

}// namespace xdpd 

#endif /* CONFIG_OPENFLOW_CONN_PLUGIN_H_ */


