#ifndef CONFIG_OF_LSI_PLUGIN_H
#define CONFIG_OF_LSI_PLUGIN_H 

#include <tuple>
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

class lsi_scope:public scope {
	
public:
	lsi_scope(std::string scope_name, scope* parent);
		
protected:
	virtual void post_validate(libconfig::Setting& setting, bool dry_run);

	//Parsing routines
	void parse_flavor(libconfig::Setting& setting, sw_flavor_t* flavor);
	void parse_version(libconfig::Setting& setting, of_version_t* version);
	void parse_reconnect_time(libconfig::Setting& setting, unsigned int* reconnect_time);
	void parse_pirl(libconfig::Setting& setting, bool* pirl_enabled, int* pirl_rate); 
	void parse_active_connections(libconfig::Setting& setting, std::string& master_controller, int& master_controller_port, std::string& slave_controller, int& slave_controller_port);
	void parse_matching_algorithms(libconfig::Setting& setting, of_version_t version, unsigned int num_of_tables, int* ma_list, bool dry_run);
	void parse_ports(libconfig::Setting& setting, std::vector<std::tuple<std::string, uint32_t> >& ports, bool dry_run);
	enum xdpd::csocket::socket_type_t parse_socket(libconfig::Setting& setting, xdpd::cparams& socket_params);
private:
	void parse_ip(rofl::caddress& addr, std::string& ip, unsigned int port);
	class find_port_tuple_by_name {
		std::string portname;
	public:
		find_port_tuple_by_name(const std::string& portname) :
			portname(portname)
		{};
		bool operator()(const std::tuple<std::string, uint32_t>& t) const{
			return (std::get<0>(t) == portname);
		};
	};
	class find_port_tuple_by_portno {
		uint32_t portno;
	public:
		find_port_tuple_by_portno(uint32_t portno) :
			portno(portno)
		{};
		bool operator()(const std::tuple<std::string, uint32_t>& t) const{
			return (std::get<1>(t) == portno);
		};
	};
};

}// namespace xdpd 

#endif /* CONFIG_OF_LSI_PLUGIN_H_ */


