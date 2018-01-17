#include "lsi_scope.h"
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <algorithm>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>
#include "../../../switch_manager.h"
#include "../../../port_manager.h"
#include "../../../../openflow/pirl/pirl.h"

#include "../yaml_config.h"
#include "../interfaces/nf_scope.h"
#include "lsi_connections.h"

using namespace xdpd::plugins::yaml_config;
using namespace rofl;

//Constants
#define LSI_DPID "dpid"
#define LSI_FLAVOR "flavor"
#define LSI_VERSION "version"
#define LSI_DESCRIPTION "description"
#define LSI_RECONNECT_TIME "reconnect-time"
#define LSI_PIRL_ENABLED "pirl-enabled"
#define LSI_PIRL_RATE "pirl-rate"
#define LSI_TABLES "tables"
#define LSI_TABLE_ID "id"
#define LSI_TABLE_MATCHING_ALGORITHM "matching-algorithm"
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
	register_parameter(LSI_TABLES);

	//Port mappings
	register_parameter(LSI_PORTS, true);
}


void lsi_scope::parse_flavor(YAML::Node& node, sw_flavor_t* flavor){

	//Parse version
	std::string s_flavor("generic");

	if(node[LSI_FLAVOR] && node[LSI_FLAVOR].IsScalar()){
		s_flavor = node[LSI_FLAVOR].as<std::string>();
	}

	if(s_flavor == "generic"){
		*flavor = SW_FLAVOR_GENERIC;
	}else if(s_flavor == "ofdpa"){
		*flavor = SW_FLAVOR_OFDPA;
	}else{
		XDPD_ERR(YAML_PLUGIN_ID "%s: invalid switch flavor. Valid switch flavors include \"generic\" (default) and \"ofdpa\". Found: %s\n", name.c_str(), s_flavor.c_str());
		throw eYamlConfParseError();
	}
}


void lsi_scope::parse_version(YAML::Node& node, of_version_t* version){

	//Parse version
	double of_ver = node[LSI_VERSION].as<double>();

	if(of_ver == 1.0){
		*version = OF_VERSION_10;
	}else if(of_ver == 1.2){
		*version = OF_VERSION_12;
	}else if(of_ver == 1.3){
		*version = OF_VERSION_13;
	}else{
		XDPD_ERR(YAML_PLUGIN_ID "%s: invalid OpenFlow version. Valid version numbers are 1.0, 1.2 and 1.3. Found: %f\n", name.c_str(), of_ver);
		throw eYamlConfParseError();
	}
}


void lsi_scope::parse_reconnect_time(YAML::Node& node, unsigned int* reconnect_time){

	if(node[LSI_RECONNECT_TIME])
		*reconnect_time = node[LSI_RECONNECT_TIME].as<unsigned int>();

	if(*reconnect_time <1 || *reconnect_time > 90){
		XDPD_ERR(YAML_PLUGIN_ID "%s: invalid reconnect-time of %u. Value must be > 1 and <= 90\n", name.c_str(), *reconnect_time);
		throw eYamlConfParseError();

	}
}

void lsi_scope::parse_pirl(YAML::Node& node, bool* pirl_enabled, int* pirl_rate){

	if(node[LSI_PIRL_ENABLED]){
		*pirl_enabled = node[LSI_PIRL_ENABLED].as<bool>();
	}
	if(node[LSI_PIRL_RATE]){
		*pirl_rate = node[LSI_PIRL_RATE].as<int>();
	}

	if(*pirl_rate < pirl::PIRL_MIN_RATE ){
		XDPD_ERR(YAML_PLUGIN_ID "%s: invalid pirl-rate of %u. Value must be >=  %d\n", name.c_str(), *pirl_rate, pirl::PIRL_MIN_RATE);
		throw eYamlConfParseError();

	}
}
void lsi_scope::parse_ports(YAML::Node& node, std::vector<std::tuple<std::string, uint32_t> >& ports, bool dry_run){

	//TODO: improve conf file to be able to control the OF port number when attaching

	std::set<std::string> platform_ports = port_manager::list_available_port_names();

	if(!node[LSI_PORTS] || !node[LSI_PORTS].IsMap()){
		XDPD_ERR(YAML_PLUGIN_ID "%s: missing or unable to parse port attachment list.\n", name.c_str());
		throw eYamlConfParseError();
	}

	uint32_t of_port_num = 0;
	for(auto it : node[LSI_PORTS]){
		std::string port = it.first.as<std::string>();
		YAML::Node port_node = it.second;
		of_port_num++;

		if(port != ""){
			//Get openflow port number
			if(port_node && port_node["portno"] && port_node["portno"].IsScalar()){
				of_port_num = port_node["portno"].as<uint32_t>();
			}
			//Check if blacklisted to print a nice trace
			if(port_manager::is_blacklisted(port)){
				XDPD_ERR(YAML_PLUGIN_ID "%s: invalid port '%s'. Port is BLACKLISTED!\n", name.c_str(), port.c_str());
				throw eYamlConfParseError();
			}
			//Check if exists
			if(port_manager::exists(port) == false && ((nf_scope*)get_scope_abs_path("config.interfaces.nf"))->is_nf_port(port)){
				XDPD_ERR(YAML_PLUGIN_ID "%s: invalid port '%s'. Port does not exist!\n", name.c_str(), port.c_str());
				throw eYamlConfParseError();
			}
			if(find_if(ports.begin(), ports.end(), find_port_tuple_by_name(port))!=ports.end()){
				XDPD_ERR(YAML_PLUGIN_ID "%s: attempting to attach twice port '%s'!\n", name.c_str(), port.c_str());
				throw eYamlConfParseError();
			}
			if(find_if(ports.begin(), ports.end(), find_port_tuple_by_portno(of_port_num))!=ports.end()){
				XDPD_ERR(YAML_PLUGIN_ID "%s: attempting to use portno '%u' twice!\n", name.c_str(), of_port_num);
				throw eYamlConfParseError();
			}

		}

		//Then push it to the list of ports
		//Note empty ports are valid (empty, ignore slot)
		ports.push_back(std::make_tuple(port, of_port_num));
	}

	if(ports.size() < 2 && dry_run){
		XDPD_WARN(YAML_PLUGIN_ID "%s: WARNING the LSI has less than two ports attached.\n", name.c_str());
	}
}

void lsi_scope::parse_tables(YAML::Node& node, of_version_t version, int* ma_list, bool dry_run){

	std::list<std::string> available_algorithms = switch_manager::list_matching_algorithms(version);
	std::list<std::string>::iterator it;
	int i = 0;

	tables.clear();

	if(!node[LSI_TABLES])
		return;

	if(node[LSI_TABLES] && !node[LSI_TABLES].IsMap()){
		XDPD_ERR(YAML_PLUGIN_ID "%s: unable to parse matching algorithms.\n", name.c_str());
		throw eYamlConfParseError();
	}

	for(auto it : node[LSI_TABLES]){
		std::string table_name = it.first.as<std::string>();
		YAML::Node table_node = it.second;
		std::string algorithm;

		if (table_node[LSI_TABLE_MATCHING_ALGORITHM]) {
			algorithm = table_node[LSI_TABLE_MATCHING_ALGORITHM].as<std::string>();
		}

		if(algorithm == ""){
			XDPD_ERR(YAML_PLUGIN_ID "%s: unable to parse matching algorithm for table number %u. Empty algorithm.\n", name.c_str(), i+1);
			throw eYamlConfParseError();
		}

		//Check existance
		auto jt = std::find(available_algorithms.begin(), available_algorithms.end(), algorithm);

		if(jt == available_algorithms.end()){
			XDPD_ERR(YAML_PLUGIN_ID "%s: unknown matching algorithm '%s' (OFP version 0x%x) for table number %u. Available matching algorithms are: [", name.c_str(), algorithm.c_str(),version, i+1);
			for (jt = available_algorithms.begin(); jt != available_algorithms.end(); ++jt)
				XDPD_ERR(YAML_PLUGIN_ID "%s,", (*jt).c_str());
			XDPD_ERR(YAML_PLUGIN_ID "]\n");
			throw eYamlConfParseError();
		}

		//Then push it to the list of ports
		ma_list[i++] = std::distance(available_algorithms.begin(), jt);

		tables[table_name] = algorithm;
	}

	if(i != (int)tables.size() && dry_run){
		XDPD_WARN(YAML_PLUGIN_ID "%s: a matching algorithm has NOT been specified for all tables. Default algorithm for tables from %u to %u included has been set to the default '%s'\n", name.c_str(), i+1, tables.size(), (*available_algorithms.begin()).c_str());
	}
}

/* Case insensitive */
void lsi_scope::post_validate(YAML::Node& node, bool dry_run){

	uint64_t dpid;
	sw_flavor_t flavor;
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
	std::string dpid_s = node[LSI_DPID].as<std::string>();
	dpid = strtoull(dpid_s.c_str(),NULL,0);
	if(!dpid){
		XDPD_ERR(YAML_PLUGIN_ID "%s: Unable to convert parameter DPID to a proper uint64_t\n", name.c_str());
		throw eYamlConfParseError();
	}

	//Parse flavor
	parse_flavor(node, &flavor);

	//Parse version
	parse_version(node, &version);

	//Parse reconnect
	parse_reconnect_time(node, &reconnect_time);

	//Parse pirl
	parse_pirl(node, &pirl_enabled, &pirl_rate);

	//Parse ports
	parse_ports(node, ports, dry_run);

	//Parse tables
	parse_tables(node, version, ma_list, dry_run);

	//Num of tables
	if(version == OF_VERSION_10 && tables.size() > 1){
		XDPD_ERR(YAML_PLUGIN_ID "%s: number of tables %u > 1. An LSI running in OF 1.0 native mode, can only be instantiated with 1 table.\n", name.c_str(), num_of_tables);
		throw eYamlConfParseError();
	}

	if(num_of_tables < 1 || num_of_tables > 255){
		XDPD_ERR(YAML_PLUGIN_ID "%s: invalid num of tables %u. An LSI shall have from 1 to 255 tables.\n", name.c_str(), num_of_tables);
		throw eYamlConfParseError();
	}

	//Execute
	if(!dry_run){
		std::vector<lsi_connection> conns = static_cast<lsi_connections_scope*>(get_subscope(lsi_connections_scope::SCOPE_NAME))->get_parsed_connections();
		openflow_switch* sw;

		//Create switch with the initial connection (connection 0)
		sw = switch_manager::create_switch(version, dpid, name, tables.size(), ma_list, reconnect_time, conns[0].type, conns[0].params, flavor);


		if(!sw){
			XDPD_ERR(YAML_PLUGIN_ID "%s: Unable to create LSI %s; unknown error.\n", name.c_str(), name.c_str());
			throw eYamlConfParseError();
		}

		//Attach ports
		std::vector<std::tuple<std::string, uint32_t> >::iterator port_it;
		for(port_it = ports.begin(); port_it != ports.end(); ++port_it){

			std::string portname = std::get<0>(*port_it);
			uint32_t of_port_num = std::get<1>(*port_it);

			//Ignore empty ports
			if(portname == "")
				continue;

			try{
				//Attach
				port_manager::attach_port_to_switch(dpid, portname, &of_port_num);
				//Bring up
				port_manager::bring_up(portname);
			}catch(...){
				XDPD_ERR(YAML_PLUGIN_ID "%s: unable to attach port '%s'. Unknown error.\n", name.c_str(), portname.c_str());
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
