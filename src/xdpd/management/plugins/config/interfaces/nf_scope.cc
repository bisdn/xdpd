#include "nf_scope.h"
#include "../config.h"
#include "../../../switch_manager.h"
#include "../../../port_manager.h"
#include "../../../nf_port_manager.h"
#include "../../../../openflow/openflow_switch.h"

#define TYPE "type"
#define NF_NAME "nf-name"

//Port types
#define PT_NATIVE "NATIVE"
#define PT_SHMEM "SHMEM"
#define PT_EXTERNAL "EXTERNAL"

using namespace xdpd;

nf_scope::nf_scope(scope* parent):scope("nf", parent, false){}

void nf_scope::post_validate(libconfig::Setting& setting, bool dry_run){

	
	for(int i=0; i<setting.getLength(); ++i){

		std::string nf_name("");
		std::string nf_port_name(setting[i].getName());
		std::string nf_port_type("");
		port_type_t type=PORT_TYPE_NF_EXTERNAL;		

		ROFL_DEBUG_VERBOSE(CONF_PLUGIN_ID "[%s] Found nf interface: %s\n", get_path().c_str(), nf_port_name.c_str());
		//Check for port existance		
		if(port_manager::exists(nf_port_name) == true){
			ROFL_ERR(CONF_PLUGIN_ID "%s: ERROR the port name '%s' already exists!\n", setting.getPath().c_str(), nf_port_name.c_str());
			throw eConfParseError(); 	
		}

		if( setting[i].exists(TYPE) == false){
			ROFL_ERR(CONF_PLUGIN_ID "%s: ERROR the port type for port name '%s' is missing or has an invalid type!\n", setting.getPath().c_str(), nf_port_name.c_str());
		}
		
		//Check for NF port type
		try{
			std::string _tmp = setting[i][TYPE];
			nf_port_type = _tmp;

			if( nf_port_type.compare(std::string(PT_NATIVE)) == 0 ){
				type = PORT_TYPE_NF_NATIVE;
			}else if( nf_port_type.compare(std::string(PT_SHMEM)) == 0 ){
				type = PORT_TYPE_NF_SHMEM;
			}else if( nf_port_type.compare(std::string(PT_EXTERNAL)) == 0 ){
				type = PORT_TYPE_NF_EXTERNAL;
			}else{
				throw eConfParseError(); 	
			}
		}catch(...){
			ROFL_ERR(CONF_PLUGIN_ID "%s: ERROR the port type for port name '%s' is invalid!\n", setting.getPath().c_str(), nf_port_name.c_str());
		}	

		//Check for nf-name 
		if(type == PORT_TYPE_NF_SHMEM){
			
			try{
				//Recover nf-name
				std::string _tmp = setting[i][NF_NAME];
				nf_name = _tmp;
			}catch(...){
				ROFL_ERR(CONF_PLUGIN_ID "%s: ERROR mandatory '%s' is missing or is invalid for port name '%s' of type '%s'!\n", setting.getPath().c_str(), NF_NAME, nf_port_name.c_str(), PT_SHMEM);
				throw eConfParseError(); 	
			}
		}
	
		if(!dry_run){
			//Call the addition
			try{
				nf_port_manager::create_nf_port(nf_name, nf_port_name, type);
			}catch(...){
				ROFL_ERR(CONF_PLUGIN_ID "%s: unable to create port '%s'. Unknown error.\n", setting.getPath().c_str(), nf_port_name.c_str());
				throw;

			}	
		}
	}

	
}

