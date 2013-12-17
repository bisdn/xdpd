#ifndef CONFIG_OF_LSI_PLUGIN_H
#define CONFIG_OF_LSI_PLUGIN_H 

#include <rofl/common/caddress.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include "../scope.h"

/**
* @file lsi_scope.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Openflow libconfig file hierarchy 
* 
*/

namespace xdpd {

class lsi_scope:public scope {
	
public:
	lsi_scope(std::string scope_name, bool mandatory=false);
		
protected:
	virtual void post_validate(libconfig::Setting& setting, bool dry_run);

	//Parsing routines
	void parse_version(libconfig::Setting& setting, of_version_t* version);
	void parse_active_connections(libconfig::Setting& setting, rofl::caddress& master_controller, rofl::caddress& slave_controller, unsigned int* reconnect_time);
	void parse_passive_connection(libconfig::Setting& setting, rofl::caddress& bind_address);
	void parse_matching_algorithms(libconfig::Setting& setting, of_version_t version, unsigned int num_of_tables, int* ma_list, bool dry_run);
	void parse_ports(libconfig::Setting& setting, std::vector<std::string>& ports, bool dry_run);
private:
	void parse_ip(rofl::caddress& addr, std::string& ip, unsigned int port);
};

}// namespace xdpd 

#endif /* CONFIG_OF_LSI_PLUGIN_H_ */


