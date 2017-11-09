#include "nf_scope.h"
#include "../yaml_config.h"
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

using namespace xdpd::plugins::yaml_config;

nf_scope::nf_scope(scope* parent):scope("nf", parent, false){}

void nf_scope::post_validate(YAML::Node& node, bool dry_run){

	if (node.IsSequence()) {
		for (auto nfnode : node) {

			if (not nfnode[NF_NAME]) {
				XDPD_ERR(YAML_PLUGIN_ID "%s: ERROR key '%s' not found\n", name.c_str(), NF_NAME);
				throw eYamlConfParseError();
			}
			if (not nfnode[TYPE]) {
				XDPD_ERR(YAML_PLUGIN_ID "%s: ERROR key '%s' not found\n", name.c_str(), TYPE);
				throw eYamlConfParseError();
			}

			std::string nf_name("");
			std::string nf_port_name(nfnode[NF_NAME].as<std::string>());
			std::string nf_port_type(nfnode[TYPE].as<std::string>());
			port_type_t type=PORT_TYPE_NF_EXTERNAL;

			XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Found nf interface: %s\n", get_path().c_str(), nf_port_name.c_str());
			//Check for port existance
			if(port_manager::exists(nf_port_name) == true){
				XDPD_ERR(YAML_PLUGIN_ID "%s: ERROR the port name '%s' already exists!\n", NF_NAME, nf_port_name.c_str());
				throw eYamlConfParseError();
			}

			//Check for NF port type
			try{
				if( nf_port_type.compare(std::string(PT_NATIVE)) == 0 ){
					type = PORT_TYPE_NF_NATIVE;
				}else if( nf_port_type.compare(std::string(PT_SHMEM)) == 0 ){
					type = PORT_TYPE_NF_SHMEM;
				}else if( nf_port_type.compare(std::string(PT_EXTERNAL)) == 0 ){
					type = PORT_TYPE_NF_EXTERNAL;
				}else{
					throw eYamlConfParseError();
				}
			}catch(...){
				XDPD_ERR(YAML_PLUGIN_ID "%s: ERROR the port type for port name '%s' is invalid!\n", TYPE, nf_port_name.c_str());
			}

			if(!dry_run){
				//Call the addition
				try{
					nf_port_manager::create_nf_port(nf_name, nf_port_name, type);
				}catch(...){
					XDPD_ERR(YAML_PLUGIN_ID "%s: unable to create port '%s'. Unknown error.\n", NF_NAME, nf_port_name.c_str());
					throw;
				}
			}
		}
	}
}

