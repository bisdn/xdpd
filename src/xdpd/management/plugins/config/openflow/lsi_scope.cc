#include "lsi_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>

using namespace xdpd;

//Constants
#define LSI_DPID "dpid"
#define LSI_VERSION "version"
#define LSI_DESCRIPTION "description"
#define LSI_MODE "mode"
#define LSI_ACTIVE_MODE "active"
#define LSI_PASSIVE_MODE "passive"
#define LSI_MASTER_CONTROLLER "master-controller"
#define LSI_SLAVE_CONTROLLER "slave-controller"
#define LSI_RECONNECT_TIME "reconnect-time"
#define LSI_BIND_ADDRESS "bind-address"
#define LSI_NUM_OF_TABLES "num-of-tables"
#define LSI_TABLES_MATCHING_ALGORITHM "tables-matching-algorithm"
#define LSI_PORTS "ports" 

lsi_scope::lsi_scope(std::string name, bool mandatory):scope(name, mandatory){

	register_parameter(LSI_DPID, true);
	register_parameter(LSI_VERSION, true);
	register_parameter(LSI_DESCRIPTION);
	register_parameter(LSI_MODE);

	//Connection params:active
	register_parameter(LSI_MASTER_CONTROLLER); 
	register_parameter(LSI_SLAVE_CONTROLLER); 
	register_parameter(LSI_RECONNECT_TIME);

	//passive
	register_parameter(LSI_BIND_ADDRESS);

	//Number of tables and matching algorithms
	register_parameter(LSI_NUM_OF_TABLES);
	register_parameter(LSI_TABLES_MATCHING_ALGORITHM);

	//Port mappings
	register_parameter(LSI_PORTS, true);
}

/* Case insensitive */
void lsi_scope::post_validate(libconfig::Setting& setting, bool dry_run){

	uint64_t dpid;
	of_version_t version;
	double of_ver;

	//Default values
	bool passive = false;
	unsigned int num_of_tables = 1;
	std::string master_controller = "127.0.0.1::6633";
	std::string slave_controller = "";
	unsigned int reconnect_time = 5;
	
	std::string bind_address = "0.0.0.0:6634";

	std::vector<std::string> ports;

	//Recover dpid and try to parse
	dpid = strtoull(setting[LSI_DPID].c_str(),NULL,0);
	if(!dpid){
		ROFL_ERR("Unable to convert parameter DPID to a proper uint64_t\n");
		throw eConfParseError(); 	
	}

	//Parse version
	of_ver = setting[LSI_VERSION];
	if(of_ver == 1.0){
		version = OF_VERSION_10;
	}else if(of_ver == 1.2){
		version = OF_VERSION_12;
	}/*else if(of_ver == 1.3){
		version = OF_VERSION_13;
	}*/else{
		ROFL_ERR("Unable to understand OpenFlow version. Valid version numbers are 1.0, 1.2 and 1.3. Found: %f\n", of_ver);
		throw eConfParseError(); 	
	}

	//Parse mode
	if((setting.exists(LSI_MODE))){
		std::string mode= setting[LSI_MODE];
		if( mode == LSI_PASSIVE_MODE){
			passive = true;
		}else if(mode != LSI_ACTIVE_MODE){	
			ROFL_WARN("Unable to parse mode.. assuming ACTIVE\n"); 
		}
	}	

	if(passive){
		//PASSIVE mode: parse listen-port
		if(setting.exists(LSI_BIND_ADDRESS)){
			std::string _bind_address = setting[LSI_BIND_ADDRESS];
			bind_address = _bind_address;
		}
	}else{
		//ACTIVE mode; parse master and slave controller, as well as reconnect time
		if(setting.exists(LSI_MASTER_CONTROLLER)){
			std::string _master_controller = setting[LSI_MASTER_CONTROLLER];
			std::string master_controller = _master_controller; 
		}
		if(setting.exists(LSI_SLAVE_CONTROLLER)){
			std::string _slave_controller = setting[LSI_SLAVE_CONTROLLER];
			std::string slave_controller = _slave_controller; 
		}
		//TODO: validate addresses

		if(setting.exists(LSI_RECONNECT_TIME))
			reconnect_time = setting[LSI_RECONNECT_TIME];
		
		if(reconnect_time <1 || reconnect_time > 90){
			ROFL_ERR("Invalid reconnect-time of %u. Value must be > 1 and <= 90\n", reconnect_time);
			throw eConfParseError(); 	
	
		}
			
	}

	if(setting.exists(LSI_NUM_OF_TABLES)){
		//Parse num_of_tables
		num_of_tables = setting[LSI_NUM_OF_TABLES];

		if(version == OF_VERSION_10 && num_of_tables > 1){
			ROFL_ERR("Num of tables %u > 1. A LSI running in OF 1.0 native mode, can only be instantiated with 1 table.\n", num_of_tables);
			throw eConfParseError(); 	
		}	
	}

	if(num_of_tables < 1 || num_of_tables > 255){
		ROFL_ERR("Invalid num of tables %u. An LSI shall have from 1 to 255 tables.\n", num_of_tables);
		throw eConfParseError(); 	
	
	}

	//Parse matching algorithms
	

	//Parse ports	
	if(!setting.exists(LSI_PORTS) || !setting[LSI_PORTS].isList()){
 		ROFL_ERR("Missing or unable to parse port attachment list.\n");
		throw eConfParseError(); 	
	
	}



	for(int i=0; i<setting[LSI_PORTS].getLength(); ++i){
		std::string port = setting[LSI_PORTS][i];
		if(port != "")
			ports.push_back(port);	
	}	

	if(ports.size() < 2){
 		ROFL_ERR("An LSI must have at least two ports attached.\n");
		throw eConfParseError(); 	
	
	}

	//Execute
	if(!dry_run){
		//TODO
		
		//Create switch
	
		//Attach ports
	}
}
