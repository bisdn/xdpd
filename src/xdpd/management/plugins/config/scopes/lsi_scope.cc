#include "lsi_scope.h"
#include <stdlib.h>
#include <inttypes.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>

using namespace xdpd;

lsi_scope::lsi_scope(std::string name, bool mandatory):scope(name, mandatory){

	register_parameter("dpid", true);
	register_parameter("version", true);
	register_parameter("description");
	register_parameter("mode");

	//Connection params
	register_parameter("master-controller-ip"); 
	register_parameter("slave-controller-ip"); 

	register_parameter("connect-port");
	register_parameter("listen-port");
	register_parameter("reconnect-time");

	//Number of tables and matching algorithms
	register_parameter("num-of-tables", true);
	register_parameter("tables-matching-algorithms");

	//Port mappings
	register_parameter("ports");
}

void lsi_scope::post_validate(libconfig::Setting& setting, bool dry_run){

	uint64_t dpid;
	of_version_t version;
	float of_ver;
	bool passive = false;

	//Recover dpid and try to parse
	dpid = strtoull(setting["dpid"].c_str(),NULL,0);
	if(!dpid){
		ROFL_ERR("Unable to convert parameter DPID to a proper uint64_t\n");
		throw eConfParseError(); 	
	}

	//Parse version
	of_ver = setting["version"]; 
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
	if(setting.exists("mode")){
		if(setting["mode"] != "passe"){
			passive = true;
		}else if(setting["mode"] != "active"){	
			ROFL_WARN("Unable to parse mode.. assuming ACTIVE\n"); 
		}
	}	

	if(passive){
		//PASSIVE mode: parse listen-port
	}else{
		//ACTIVE mode; parse master and slave controller, as well as reconnect time
	}

	//Parse num_of_tables
	unsigned int num_of_tables = setting["num_of_tables"];
	
	(void)num_of_tables;	
	(void)version;	

}
