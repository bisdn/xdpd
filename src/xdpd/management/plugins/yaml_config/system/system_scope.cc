#include "system_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include "xdpd/common/logging.h"
#include "../../../system_manager.h"
#include "../yaml_config.h"

using namespace xdpd::plugins::yaml_config;

#define ID "id" 
#define LOGGING_LEVEL "logging-level"
#define DRIVER_EXTRA_PARAMS "driver-extra-params"
#define DRIVER_EXTRA_PARAMS_FULL "config.system.driver-extra-params"


system_scope::system_scope(scope* parent):scope("system", parent, false){
	
	//Register parameters
	register_parameter(ID);
	register_parameter(LOGGING_LEVEL);
	register_parameter(DRIVER_EXTRA_PARAMS);

	//Register subscopes
	//None for the moment
}

/* Case insensitive */
void system_scope::post_validate(YAML::Node& node, bool dry_run){

	int logging_level=-1;

	//Parse mode
	if(node[LOGGING_LEVEL]){
		std::string log_level = node[LOGGING_LEVEL].as<std::string>();

		//Validate
		if(log_level == "EMERG")
			logging_level = logging::EMERG;
		else if(log_level == "ALERT")
			logging_level = logging::ALERT;
		else if(log_level == "CRIT")
			logging_level = logging::CRIT;
		else if(log_level == "ERROR")
			logging_level = logging::ERROR;
		else if(log_level == "WARN")
			logging_level = logging::WARN;
		else if(log_level == "NOTICE")
			logging_level = logging::NOTICE;
		else if(log_level == "INFO")
			logging_level = logging::INFO;
		else if(log_level == "DEBUG")
			logging_level = logging::DBG; 
		else if(log_level == "TRACE")
			logging_level = logging::TRACE;
		else {
 			XDPD_WARN(YAML_PLUGIN_ID "%s: Invalid logging level '%s'.\n", LOGGING_LEVEL, log_level.c_str());
		}
	}	
	//Execute
	if(!dry_run && logging_level!=-1){
		
		//Set id
		if(node[ID]){
			std::string id = node[ID].as<std::string>();
			system_manager::set_id(id);
		}
		
		//Set logging
		try{
			system_manager::set_logging_debug_level(logging_level);
		}catch(eSystemLogLevelSetviaCLI& e){
			//Ignore. Trace is already printed by system_manager
		}
		
	}
}

//By passes all validations since it is pre-plugin init bootstrap
//and xDPd core services CANNOT be used
std::string system_scope::get_driver_extra_params(YAML::Node& node){

	std::string extra;

	if (node["system"]) {
		YAML::Node system = node["system"];
		if (system["driver-extra-params"]){
			return system["driver-extra-params"].as<std::string>();
		}
	}

	return extra;
}
