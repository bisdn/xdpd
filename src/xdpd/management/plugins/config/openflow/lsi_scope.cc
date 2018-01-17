#include "lsi_scope.h"
#include <tuple>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <algorithm>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>
#include "../../../switch_manager.h"
#include "../../../port_manager.h"
#include "../../../../openflow/pirl/pirl.h"

#include "../config.h"
#include "../interfaces/nf_scope.h"
#include "lsi_connections.h"

using namespace xdpd;
using namespace rofl;

//Constants
#define LSI_DPID "dpid"
#define LSI_FLAVOR "flavor"
#define LSI_VERSION "version"
#define LSI_DESCRIPTION "description"
#define LSI_RECONNECT_TIME "reconnect-time"
#define LSI_PIRL_ENABLED "pirl-enabled"
#define LSI_PIRL_RATE "pirl-rate"
#define LSI_NUM_OF_TABLES "num-of-tables"
#define LSI_TABLES_MATCHING_ALGORITHM "tables-matching-algorithm"
#define LSI_PORTS "ports" 

lsi_scope::lsi_scope(std::string name, scope* parent):scope(name, parent, false){

	register_parameter(LSI_DPID, true);
	register_parameter(LSI_FLAVOR);
	register_parameter(LSI_VERSION, true);
	register_parameter(LSI_DESCRIPTION);

	//Register connections subscope
	register_subscope(new lsi_connections_scope(this));	
	
	//Reconnect time
	register_parameter(LSI_RECONNECT_TIME);

	//PIRL
	register_parameter(LSI_PIRL_ENABLED);
	register_parameter(LSI_PIRL_RATE);
	
	//Number of tables and matching algorithms
	register_parameter(LSI_NUM_OF_TABLES);
	register_parameter(LSI_TABLES_MATCHING_ALGORITHM);

	//Port mappings
	register_parameter(LSI_PORTS, true);
}


void lsi_scope::parse_flavor(libconfig::Setting& setting, sw_flavor_t* flavor){

	//Parse version
	std::string s_flavor("generic");

	if(setting.exists(LSI_FLAVOR)){
		s_flavor = (const char*)setting[LSI_FLAVOR];
	}

	if(s_flavor == "generic"){
		*flavor = SW_FLAVOR_GENERIC;
	}else if(s_flavor == "ofdpa"){
		*flavor = SW_FLAVOR_OFDPA;
	}else{
		XDPD_ERR(CONF_PLUGIN_ID "%s: invalid switch flavor. Valid switch flavors include \"generic\" (default) and \"ofdpa\". Found: %s\n", setting.getPath().c_str(), s_flavor.c_str());
		throw eConfParseError();
	}
}


void lsi_scope::parse_version(libconfig::Setting& setting, of_version_t* version){

	//Parse version
	double of_ver = setting[LSI_VERSION];

	if(of_ver == 1.0){
		*version = OF_VERSION_10;
	}else if(of_ver == 1.2){
		*version = OF_VERSION_12;
	}else if(of_ver == 1.3){
		*version = OF_VERSION_13;
	}else{
		XDPD_ERR(CONF_PLUGIN_ID "%s: invalid OpenFlow version. Valid version numbers are 1.0, 1.2 and 1.3. Found: %f\n", setting.getPath().c_str(), of_ver);
		throw eConfParseError(); 	
	}
}


void lsi_scope::parse_reconnect_time(libconfig::Setting& setting, unsigned int* reconnect_time){

	if(setting.exists(LSI_RECONNECT_TIME))
		*reconnect_time = setting[LSI_RECONNECT_TIME];
	
	if(*reconnect_time <1 || *reconnect_time > 90){
		XDPD_ERR(CONF_PLUGIN_ID "%s: invalid reconnect-time of %u. Value must be > 1 and <= 90\n", setting.getPath().c_str(), *reconnect_time);
		throw eConfParseError(); 	

	}
}

void lsi_scope::parse_pirl(libconfig::Setting& setting, bool* pirl_enabled, int* pirl_rate){ 

	if(setting.exists(LSI_PIRL_ENABLED)){
		*pirl_enabled = setting[LSI_PIRL_ENABLED]; 
	}
	if(setting.exists(LSI_PIRL_RATE)){
		*pirl_rate = setting[LSI_PIRL_RATE];	
	}
	
	if(*pirl_rate < pirl::PIRL_MIN_RATE ){
		XDPD_ERR(CONF_PLUGIN_ID "%s: invalid pirl-rate of %u. Value must be >=  %d\n", setting.getPath().c_str(), *pirl_rate, pirl::PIRL_MIN_RATE);
		throw eConfParseError(); 	

	}
}
void lsi_scope::parse_ports(libconfig::Setting& setting, std::vector<std::tuple<std::string, uint32_t> >& ports, bool dry_run){

	//TODO: improve conf file to be able to control the OF port number when attaching

	std::set<std::string> platform_ports = port_manager::list_available_port_names();	

	if(!setting.exists(LSI_PORTS) || !setting[LSI_PORTS].isList()){
 		XDPD_ERR(CONF_PLUGIN_ID "%s: missing or unable to parse port attachment list.\n", setting.getPath().c_str());
		throw eConfParseError(); 	
	
	}

	for(int i=0; i<setting[LSI_PORTS].getLength(); ++i){
		std::string s_port = setting[LSI_PORTS][i];
		std::string port = setting[LSI_PORTS][i];
		size_t epos;
		uint32_t of_port_num=i+1;

		if (((epos = s_port.find(":")) != std::string::npos)){
			port = s_port.substr(0, epos);
			std::stringstream ss(s_port.substr(epos+1, s_port.length()));
			ss >> of_port_num;
		}

		if(port != ""){
			//Check if blacklisted to print a nice trace
			if(port_manager::is_blacklisted(port)){
				XDPD_ERR(CONF_PLUGIN_ID "%s: invalid port '%s'. Port is BLACKLISTED!\n", setting.getPath().c_str(), port.c_str());
				throw eConfParseError(); 	
			
			}
			//Check if exists
			if(port_manager::exists(port) == false && ((nf_scope*)get_scope_abs_path("config.interfaces.nf"))->is_nf_port(port)){
				XDPD_ERR(CONF_PLUGIN_ID "%s: invalid port '%s'. Port does not exist!\n", setting.getPath().c_str(), port.c_str());
				throw eConfParseError(); 	
			
			}
			if(find_if(ports.begin(), ports.end(), find_port_tuple_by_name(port))!=ports.end()){
				XDPD_ERR(CONF_PLUGIN_ID "%s: attempting to attach twice port '%s'!\n", setting.getPath().c_str(), port.c_str());
				throw eConfParseError();
			}
			if(find_if(ports.begin(), ports.end(), find_port_tuple_by_portno(of_port_num))!=ports.end()){
				XDPD_ERR(CONF_PLUGIN_ID "%s: attempting to use portno '%u' twice!\n", setting.getPath().c_str(), of_port_num);
				throw eConfParseError(); 	
			}				
		}
		
		//Then push it to the list of ports
		//Note empty ports are valid (empty, ignore slot)
		ports.push_back(std::make_tuple(port, of_port_num));
	
	}	

	if(ports.size() < 2 && dry_run){
 		XDPD_WARN(CONF_PLUGIN_ID "%s: WARNING the LSI has less than two ports attached.\n", setting.getPath().c_str());
	}
}

void lsi_scope::parse_matching_algorithms(libconfig::Setting& setting, of_version_t version, unsigned int num_of_tables, int* ma_list, bool dry_run){
	
	std::list<std::string> available_algorithms = switch_manager::list_matching_algorithms(version);
	std::list<std::string>::iterator it;
	int i;
	
	if(!setting.exists(LSI_TABLES_MATCHING_ALGORITHM))
		return;

	if(setting.exists(LSI_TABLES_MATCHING_ALGORITHM) && !setting[LSI_TABLES_MATCHING_ALGORITHM].isList()){
 		XDPD_ERR(CONF_PLUGIN_ID "%s: unable to parse matching algorithms.\n", setting.getPath().c_str());
		throw eConfParseError(); 	
	}

	if(setting[LSI_TABLES_MATCHING_ALGORITHM].getLength() > (int)num_of_tables){
		XDPD_ERR(CONF_PLUGIN_ID "%s: error while parsing matching algorithms. The amount of matching algorithms specified (%u) exceeds the number of tables(%u)\n", setting.getPath().c_str(), setting[LSI_TABLES_MATCHING_ALGORITHM].getLength(), num_of_tables);
		throw eConfParseError(); 	
	}

	for(i=0; i<setting[LSI_TABLES_MATCHING_ALGORITHM].getLength(); ++i){
	
		std::string algorithm = setting[LSI_TABLES_MATCHING_ALGORITHM][i];
	
		if(algorithm == ""){
			XDPD_ERR(CONF_PLUGIN_ID "%s: unable to parse matching algorithm for table number %u. Empty algorithm.\n", setting.getPath().c_str(), i+1);
			throw eConfParseError(); 	
		}
	
		//Check existance
		it = std::find(available_algorithms.begin(), available_algorithms.end(), algorithm);

		if(it == available_algorithms.end()){
			XDPD_ERR(CONF_PLUGIN_ID "%s: unknown matching algorithm '%s' (OFP version 0x%x) for table number %u. Available matching algorithms are: [", setting.getPath().c_str(), algorithm.c_str(),version, i+1);
			for (it = available_algorithms.begin(); it != available_algorithms.end(); ++it)
				XDPD_ERR(CONF_PLUGIN_ID "%s,", (*it).c_str());
			XDPD_ERR(CONF_PLUGIN_ID "]\n");
			throw eConfParseError(); 	
		}
				
		//Then push it to the list of ports
		ma_list[i] = std::distance(available_algorithms.begin(), it);
	}

	if(i != (int)num_of_tables && dry_run){
		XDPD_WARN(CONF_PLUGIN_ID "%s: a matching algorithm has NOT been specified for all tables. Default algorithm for tables from %u to %u included has been set to the default '%s'\n", setting.getPath().c_str(), i+1, num_of_tables, (*available_algorithms.begin()).c_str());
	}
}

/* Case insensitive */
void lsi_scope::post_validate(libconfig::Setting& setting, bool dry_run){

	uint64_t dpid;
	sw_flavor_t flavor = SW_FLAVOR_GENERIC;
	of_version_t version;
	std::vector<lsi_connection> connections;
 
	//Default values
	//bool passive = false;
	unsigned int num_of_tables = 1;
	caddress master_controller;
	caddress slave_controller;
	unsigned int reconnect_time = 5;
	//std::string bind_address_ip = "0.0.0.0";
	//caddress bind_address;
	std::vector<std::tuple<std::string, uint32_t> > ports;
	int ma_list[OF1X_MAX_FLOWTABLES] = { 0 };
	bool pirl_enabled = true;
	int pirl_rate=pirl::PIRL_DEFAULT_MAX_RATE;

	//Recover dpid and try to parse
	std::string dpid_s = setting[LSI_DPID];
	dpid = strtoull(dpid_s.c_str(),NULL,0);
	if(!dpid){
		XDPD_ERR(CONF_PLUGIN_ID "%s: Unable to convert parameter DPID to a proper uint64_t\n", setting.getPath().c_str());
		throw eConfParseError(); 	
	}

	//Parse flavor
	if (setting.exists(LSI_FLAVOR)) {
		parse_flavor(setting, &flavor);
	}

	//Parse version
	parse_version(setting, &version);

	//Parse reconnect 
	parse_reconnect_time(setting, &reconnect_time);


	//Num of tables
	if(setting.exists(LSI_NUM_OF_TABLES)){
		//Parse num_of_tables
		num_of_tables = setting[LSI_NUM_OF_TABLES];

		if(version == OF_VERSION_10 && num_of_tables > 1){
			XDPD_ERR(CONF_PLUGIN_ID "%s: number of tables %u > 1. An LSI running in OF 1.0 native mode, can only be instantiated with 1 table.\n", setting.getPath().c_str(), num_of_tables);
			throw eConfParseError(); 	
		}	
	}
	if(num_of_tables < 1 || num_of_tables > 255){
		XDPD_ERR(CONF_PLUGIN_ID "%s: invalid num of tables %u. An LSI shall have from 1 to 255 tables.\n", setting.getPath().c_str(), num_of_tables);
		throw eConfParseError(); 	
	
	}

	//Parse matching algorithms
	parse_matching_algorithms(setting, version, num_of_tables, ma_list, dry_run);


	//Parse pirl
	parse_pirl(setting, &pirl_enabled, &pirl_rate);

	//Parse ports	
	parse_ports(setting, ports, dry_run);

	//Execute
	if(!dry_run){
		std::vector<lsi_connection> conns = static_cast<lsi_connections_scope*>(get_subscope(lsi_connections_scope::SCOPE_NAME))->get_parsed_connections();
		openflow_switch* sw;

		//Create switch with the initial connection (connection 0)
		sw = switch_manager::create_switch(version, dpid, name, num_of_tables, ma_list, reconnect_time, conns[0].type, conns[0].params, flavor);


		if(!sw){
			XDPD_ERR(CONF_PLUGIN_ID "%s: Unable to create LSI %s; unknown error.\n", setting.getPath().c_str(), name.c_str());
			throw eConfParseError(); 	
		}	
	
		//Attach ports
		std::vector<std::tuple<std::string, uint32_t> >::iterator port_it;
		for(port_it = ports.begin(); port_it != ports.end(); ++port_it){
				
			std::string port_name = std::get<0>(*port_it);
			unsigned int i = std::get<1>(*port_it);

			//Ignore empty ports	
			if(port_name == "")
				continue;
		
			try{
				//Attach
				port_manager::attach_port_to_switch(dpid, port_name, &i);
				//Bring up
				port_manager::bring_up(port_name);
			}catch(...){	
				XDPD_ERR(CONF_PLUGIN_ID "%s: unable to attach port '%s'. Unknown error.\n", setting.getPath().c_str(), port_name.c_str());
				throw;
			}
		}

		//Configure PIRL
		if(pirl_enabled == false)
			switch_manager::reconfigure_pirl(dpid, pirl::PIRL_DISABLED);
		else
			switch_manager::reconfigure_pirl(dpid, pirl_rate);
			
		
		//Connect(1..N-1)
		for(std::vector<lsi_connection>::iterator it = (conns.begin()+1); it != conns.end(); ++it) {
			switch_manager::rpc_connect_to_ctl(dpid, it->type, it->params); 
		}	
	}
}
