#include "lsi_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>
#include "../../../switch_manager.h"
#include "../../../port_manager.h"

using namespace xdpd;
using namespace rofl;

//Constants
#define LSI_DPID "dpid"
#define LSI_VERSION "version"
#define LSI_DESCRIPTION "description"
#define LSI_MODE "mode"
#define LSI_ACTIVE_MODE "active"
#define LSI_PASSIVE_MODE "passive"
#define LSI_MASTER_CONTROLLER_IP "master-controller-ip"
#define LSI_MASTER_CONTROLLER_PORT "master-controller-port"
#define LSI_SLAVE_CONTROLLER_IP "slave-controller-ip"
#define LSI_SLAVE_CONTROLLER_PORT "slave-controller-port"
#define LSI_RECONNECT_TIME "reconnect-time"
#define LSI_BIND_ADDRESS_IP "bind-address-ip"
#define LSI_BIND_ADDRESS_PORT "bind-address-port"
#define LSI_NUM_OF_TABLES "num-of-tables"
#define LSI_TABLES_MATCHING_ALGORITHM "tables-matching-algorithm"
#define LSI_PORTS "ports" 

lsi_scope::lsi_scope(std::string name, bool mandatory):scope(name, mandatory){

	register_parameter(LSI_DPID, true);
	register_parameter(LSI_VERSION, true);
	register_parameter(LSI_DESCRIPTION);
	register_parameter(LSI_MODE);

	//Connection params:active
	register_parameter(LSI_MASTER_CONTROLLER_IP); 
	register_parameter(LSI_MASTER_CONTROLLER_PORT); 
	register_parameter(LSI_SLAVE_CONTROLLER_IP); 
	register_parameter(LSI_SLAVE_CONTROLLER_PORT); 
	register_parameter(LSI_RECONNECT_TIME);

	//passive
	register_parameter(LSI_BIND_ADDRESS_IP);
	register_parameter(LSI_BIND_ADDRESS_PORT);

	//Number of tables and matching algorithms
	register_parameter(LSI_NUM_OF_TABLES);
	register_parameter(LSI_TABLES_MATCHING_ALGORITHM);

	//Port mappings
	register_parameter(LSI_PORTS, true);
}

void lsi_scope::parse_ip(caddress& addr, std::string& ip, unsigned int port){

	//Generate caddress master
	try{
		//IPv4
		addr = caddress(AF_INET, ip.c_str(), port); 
	}catch(...){
		//IPv6
		addr = caddress(AF_INET6, ip.c_str(), port); 
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
		//*version = OF_VERSION_13;
		ROFL_ERR("%s: version 1.3 version not yet supported\n", setting.getPath().c_str());
		throw eConfParseError(); 

	}else{
		ROFL_ERR("%s: invalid OpenFlow version. Valid version numbers are 1.0, 1.2 and 1.3. Found: %f\n", setting.getPath().c_str(), of_ver);
		throw eConfParseError(); 	
	}
}

void lsi_scope::parse_passive_connection(libconfig::Setting& setting, caddress& bind_address){
	
	std::string bind_address_ip = "0.0.0.0";
	unsigned int bind_address_port = 6634;

	if(setting.exists(LSI_BIND_ADDRESS_IP)){
		std::string _bind_address = setting[LSI_BIND_ADDRESS_IP];
		bind_address_ip = _bind_address;
	}

	if(setting.exists(LSI_BIND_ADDRESS_PORT)){
		bind_address_port = setting[LSI_BIND_ADDRESS_PORT];
		
		if(bind_address_port < 1 || bind_address_port > 65535){
			ROFL_ERR("%s: invalid bind address TCP port number %u. Must be [1-65535]\n", setting.getPath().c_str(), bind_address_port);
			throw eConfParseError(); 	
				
		}
	}
	try{
		parse_ip(bind_address, bind_address_ip, bind_address_port);
	}catch(...){
		ROFL_ERR("%s: unable to parse passive bind IP. It must be either an IPv4 or an IPv6 IP\n", setting.getPath().c_str());
		throw eConfParseError(); 	
	
	}
}

void lsi_scope::parse_active_connections(libconfig::Setting& setting, caddress& master_controller, caddress& slave_controller, unsigned int* reconnect_time){

	std::string master_controller_ip = "127.0.0.1";
	unsigned int master_controller_port = 6633;
	std::string slave_controller_ip = "";
	unsigned int slave_controller_port = 6634;
	
	//Parse master controller IP if it exists
	if(setting.exists(LSI_MASTER_CONTROLLER_IP)){
		std::string _master_controller = setting[LSI_MASTER_CONTROLLER_IP];
		master_controller_ip = _master_controller; 
	}

	//Parse master controller port if it exists 
	if(setting.exists(LSI_MASTER_CONTROLLER_PORT)){
		master_controller_port = setting[LSI_MASTER_CONTROLLER_PORT];
		
		if(master_controller_port < 1 || master_controller_port > 65535){
			ROFL_ERR("%s: invalid Master controller TCP port number %u. Must be [1-65535]\n", setting.getPath().c_str(), master_controller_port);
			throw eConfParseError(); 	
				
		}
	}

	
	//Parse r controller port if it exists 
	if(setting.exists(LSI_SLAVE_CONTROLLER_IP)){
		std::string _slave_controller = setting[LSI_SLAVE_CONTROLLER_IP];
		slave_controller_ip = _slave_controller; 
		
		if(setting.exists(LSI_SLAVE_CONTROLLER_PORT)){
			slave_controller_port = setting[LSI_SLAVE_CONTROLLER_PORT];
			
			if(slave_controller_port < 1 || slave_controller_port > 65535){
				ROFL_ERR("%s: invalid Slave controller TCP port number %u. Must be [1-65535]\n", setting.getPath().c_str(), master_controller_port);
				throw eConfParseError(); 	
					
			}
		}
	}


	//Parse master ip
	try{
		parse_ip(master_controller, master_controller_ip, master_controller_port);
	}catch(...){
		ROFL_ERR("%s: unable to parse Master controller IP. It must be either an IPv4 or an IPv6 IP\n", setting.getPath().c_str());
		throw eConfParseError(); 	
	
	}
	
	//Parse slave ip
	if(slave_controller_ip != ""){
		try{
			parse_ip(slave_controller, slave_controller_ip, slave_controller_port);
		}catch(...){
			ROFL_ERR("%s: unable to parse Master controller IP. It must be either an IPv4 or an IPv6 IP\n", setting.getPath().c_str());
			throw eConfParseError(); 	
		
		}
	}

	if(setting.exists(LSI_RECONNECT_TIME))
		*reconnect_time = setting[LSI_RECONNECT_TIME];
	
	if(*reconnect_time <1 || *reconnect_time > 90){
		ROFL_ERR("%s: invalid reconnect-time of %u. Value must be > 1 and <= 90\n", setting.getPath().c_str(), *reconnect_time);
		throw eConfParseError(); 	

	}
}

void lsi_scope::parse_ports(libconfig::Setting& setting, std::vector<std::string>& ports, bool dry_run){

	//TODO: improve conf file to be able to control the OF port number when attaching

	std::list<std::string> platform_ports = port_manager::list_available_port_names();	

	if(!setting.exists(LSI_PORTS) || !setting[LSI_PORTS].isList()){
 		ROFL_ERR("%s: missing or unable to parse port attachment list.\n", setting.getPath().c_str());
		throw eConfParseError(); 	
	
	}

	for(int i=0; i<setting[LSI_PORTS].getLength(); ++i){
		std::string port = setting[LSI_PORTS][i];
		if(port != ""){
			//Check if exists
			if((std::find(platform_ports.begin(), platform_ports.end(), port) == platform_ports.end())){
				ROFL_ERR("%s: invalid port '%s'. Port does not exist!\n", setting.getPath().c_str(), port.c_str());
				throw eConfParseError(); 	
			
			}
			if((std::find(ports.begin(), ports.end(), port) != ports.end())){
				ROFL_ERR("%s: attempting to attach twice port '%s'!\n", setting.getPath().c_str(), port.c_str());
				throw eConfParseError(); 	
			
			}				
			//Then push it to the list of ports
			ports.push_back(port);	
		}
	}	

	if(ports.size() < 2 && dry_run){
 		ROFL_WARN("%s: WARNING the LSI has less than two ports attached.\n", setting.getPath().c_str());
	}
}

void lsi_scope::parse_matching_algorithms(libconfig::Setting& setting, of_version_t version, unsigned int num_of_tables, int* ma_list, bool dry_run){
	
	std::list<std::string> available_algorithms = switch_manager::list_matching_algorithms(version);
	std::list<std::string>::iterator it;
	int i;
	
	if(!setting.exists(LSI_TABLES_MATCHING_ALGORITHM))
		return;

	if(setting.exists(LSI_TABLES_MATCHING_ALGORITHM) && !setting[LSI_TABLES_MATCHING_ALGORITHM].isList()){
 		ROFL_ERR("%s: unable to parse matching algorithms.\n", setting.getPath().c_str());
		throw eConfParseError(); 	
	}

	if(setting[LSI_TABLES_MATCHING_ALGORITHM].getLength() > (int)num_of_tables){
		ROFL_ERR("%s: error while parsing matching algorithms. The amount of matching algorithms specified (%u) exceeds the number of tables(%u)\n", setting.getPath().c_str(), setting[LSI_TABLES_MATCHING_ALGORITHM].getLength(), num_of_tables);
		throw eConfParseError(); 	
	}

	for(i=0; i<setting[LSI_TABLES_MATCHING_ALGORITHM].getLength(); ++i){
	
		std::string algorithm = setting[LSI_TABLES_MATCHING_ALGORITHM][i];
	
		if(algorithm == ""){
			ROFL_ERR("%s: unable to parse matching algorithm for table number %u. Empty algorithm.\n", setting.getPath().c_str(), i+1);
			throw eConfParseError(); 	
		}
	
		//Check existance
		it = std::find(available_algorithms.begin(), available_algorithms.end(), algorithm);

		if(it == available_algorithms.end()){
			ROFL_ERR("%s: unknown matching algorithm '%s' (OFP version 0x%x) for table number %u. Available matching algorithms are: [", setting.getPath().c_str(), algorithm.c_str(),version, i+1);
			for (it = available_algorithms.begin(); it != available_algorithms.end(); ++it)
				ROFL_ERR("%s,", (*it).c_str());
			ROFL_ERR("]\n");
			throw eConfParseError(); 	
		}
				
		//Then push it to the list of ports
		ma_list[i] = std::distance(available_algorithms.begin(), it);
	}

	if(i != (int)num_of_tables && dry_run){
		ROFL_WARN("%s: a matching algorithm has NOT been specified for all tables. Default algorithm for tables from %u to %u included has been set to the default '%s'\n", setting.getPath().c_str(), i+1, num_of_tables, (*available_algorithms.begin()).c_str());
	}
}

/* Case insensitive */
void lsi_scope::post_validate(libconfig::Setting& setting, bool dry_run){

	uint64_t dpid;
	of_version_t version;

	//Default values
	bool passive = false;
	unsigned int num_of_tables = 1;
	caddress master_controller;
	caddress slave_controller;
	unsigned int reconnect_time = 5;
	std::string bind_address_ip = "0.0.0.0";
	caddress bind_address;
	std::vector<std::string> ports;
	int ma_list[OF1X_MAX_FLOWTABLES] = { 0 };

	//Recover dpid and try to parse
	std::string dpid_s = setting[LSI_DPID];
	dpid = strtoull(dpid_s.c_str(),NULL,0);
	if(!dpid){
		ROFL_ERR("%s: Unable to convert parameter DPID to a proper uint64_t\n", setting.getPath().c_str());
		throw eConfParseError(); 	
	}

	//Parse version
	parse_version(setting, &version);

	//Parse mode
	if((setting.exists(LSI_MODE))){
		std::string mode= setting[LSI_MODE];
		if( mode == LSI_PASSIVE_MODE){
			passive = true;
		}else if(mode != LSI_ACTIVE_MODE){	
			if(dry_run)
				ROFL_WARN("%s: Unable to parse mode.. assuming ACTIVE\n", setting.getPath().c_str()); 
		}
	}	

	//Parse connection details
	if(passive){
		parse_passive_connection(setting, bind_address);	
	}else{
		parse_active_connections(setting, master_controller, slave_controller, &reconnect_time);	
	}

	if(setting.exists(LSI_NUM_OF_TABLES)){
		//Parse num_of_tables
		num_of_tables = setting[LSI_NUM_OF_TABLES];

		if(version == OF_VERSION_10 && num_of_tables > 1){
			ROFL_ERR("%s: number of tables %u > 1. An LSI running in OF 1.0 native mode, can only be instantiated with 1 table.\n", setting.getPath().c_str(), num_of_tables);
			throw eConfParseError(); 	
		}	
	}

	if(num_of_tables < 1 || num_of_tables > 255){
		ROFL_ERR("%s: invalid num of tables %u. An LSI shall have from 1 to 255 tables.\n", setting.getPath().c_str(), num_of_tables);
		throw eConfParseError(); 	
	
	}

	//Parse matching algorithms
	parse_matching_algorithms(setting, version, num_of_tables, ma_list, dry_run);

	//Parse ports	
	parse_ports(setting, ports, dry_run);

	//Execute
	if(!dry_run){
		openflow_switch* sw;
		unsigned int port_num;

		//Create switch
		sw = switch_manager::create_switch(version, dpid, name, num_of_tables, ma_list, reconnect_time, master_controller, bind_address); //FIXME:: add slave and reconnect

		if(!sw){
			ROFL_ERR("%s: Unable to create LSI %s; unknown error.\n", setting.getPath().c_str(), name.c_str());
			throw eConfParseError(); 	
		}	
	
		//Attach ports
		std::vector<std::string>::iterator port;
		for(port = ports.begin(); port != ports.end(); ++port){
			try{
				//Attach
				port_manager::attach_port_to_switch(dpid, *port, &port_num);
				//Bring up
				port_manager::enable_port(*port);
			}catch(...){	
				ROFL_ERR("%s: unable to attach port '%s'. Unknown error.\n", setting.getPath().c_str(), (*port).c_str());
				throw;
			}
		}
		
	}
}
