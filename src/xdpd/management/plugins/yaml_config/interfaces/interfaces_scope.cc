#include "interfaces_scope.h"
#include <list>
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>
#include "../../../switch_manager.h"
#include "../../../port_manager.h"
#include "../../../../openflow/openflow_switch.h"

#include "nf_scope.h"

#include "../yaml_config.h"

using namespace xdpd::plugins::yaml_config;
using namespace rofl;

//Constants
#define BLACKLIST "blacklist"
#define NF "nf"
#define VIRTUAL "virtual"

#define VLINK_LEFT "left"
#define VLINK_RIGHT "right"
#define VLINK_PORTNAME "portname"
#define VLINK_LSI "lsi"
#define VLINK_DESCRIPTION "description"


interfaces_scope::interfaces_scope(scope* parent):scope("interfaces", parent, false){

	//Register parameters
	register_parameter(BLACKLIST);

	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	register_priority_subscope(new nf_scope(this), 2, false);
	register_subscope(new virtual_ifaces_scope(this));
}

void interfaces_scope::post_validate(YAML::Node& node, bool dry_run){

	//Update cache
	blacklisted.clear();

	//If the configuration has a blacklist port
	if(node[BLACKLIST] && node[BLACKLIST].IsSequence()){
		YAML::Node blacklist = node[BLACKLIST];
		for (auto iface : blacklist) {

			std::string port(iface.as<std::string>());

			//Check for port existance
			if(port_manager::exists(port) == false){
				XDPD_ERR(YAML_PLUGIN_ID "%s: attempting to blacklist an invalid port '%s'. Port does not exist!\n", BLACKLIST, port.c_str());
				throw eYamlConfParseError();
			}

			//Blacklist
			blacklisted.insert(port);

			//If it is not dry run, then really blacklist it
			if(!dry_run){
				port_manager::blacklist(port);
			}
		}
	}
}

virtual_ifaces_scope::virtual_ifaces_scope(scope* parent):scope("virtual", parent, false){
	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
}


void virtual_ifaces_scope::post_validate(YAML::Node& node, bool dry_run){

	//If the configuration has virtual links
	if(node[VIRTUAL] && node[VIRTUAL].IsMap()){
		for (auto vlink : node[VIRTUAL]) {
			std::string vlink_name = vlink.first.as<std::string>();

			// port_name1
			std::string port_name1;
			if ((vlink.second[VLINK_LEFT]) && (vlink.second[VLINK_LEFT][VLINK_PORTNAME])) {
				port_name1 = vlink.second[VLINK_LEFT][VLINK_PORTNAME].as<std::string>();
			}

			// lsi_name1
			std::string lsi_name1;
			if ((vlink.second[VLINK_LEFT]) && (vlink.second[VLINK_LEFT][VLINK_LSI])) {
				lsi_name1 = vlink.second[VLINK_LEFT][VLINK_LSI].as<std::string>();
			}

			// port_name2
			std::string port_name2;
			if ((vlink.second[VLINK_RIGHT]) && (vlink.second[VLINK_RIGHT][VLINK_PORTNAME])) {
				port_name2 = vlink.second[VLINK_RIGHT][VLINK_PORTNAME].as<std::string>();
			}

			// lsi_name2
			std::string lsi_name2;
			if ((vlink.second[VLINK_RIGHT]) && (vlink.second[VLINK_RIGHT][VLINK_LSI])) {
				lsi_name2 = vlink.second[VLINK_RIGHT][VLINK_LSI].as<std::string>();
			}

			if (port_name1.empty()) {
				XDPD_ERR(YAML_PLUGIN_ID "%s: no left port for virtual link defined\n", vlink_name.c_str());
				throw eYamlConfParseError();
			}
			if(port_manager::exists(port_name1) == true){
				XDPD_ERR(YAML_PLUGIN_ID "%s: port %s already exists\n", vlink_name.c_str(), port_name1.c_str());
				throw eYamlConfParseError();
			}
			if (lsi_name1.empty()) {
				XDPD_ERR(YAML_PLUGIN_ID "%s: no left lsi for virtual link defined\n", vlink_name.c_str());
				throw eYamlConfParseError();
			}
			if (port_name2.empty()) {
				XDPD_ERR(YAML_PLUGIN_ID "%s: no right port for virtual link defined\n", vlink_name.c_str());
				throw eYamlConfParseError();
			}
			if(port_manager::exists(port_name2) == true){
				XDPD_ERR(YAML_PLUGIN_ID "%s: port %s already exists\n", vlink_name.c_str(), port_name2.c_str());
				throw eYamlConfParseError();
			}
			if (lsi_name2.empty()) {
				XDPD_ERR(YAML_PLUGIN_ID "%s: no right lsi for virtual link defined\n", vlink_name.c_str());
				throw eYamlConfParseError();
			}


			//Complete vlink
			if(!dry_run){
				uint64_t dpid1, dpid2;

				//Recover switches by name
				try{
					dpid1 = switch_manager::get_switch_dpid(lsi_name1);
				}catch(eOfSmDoesNotExist& e){
					XDPD_ERR(YAML_PLUGIN_ID "%s: unable to create virtual link %s. LSI '%s' does not exist.\n",
							VIRTUAL, vlink_name.c_str(), lsi_name1.c_str());
					throw eYamlConfParseError();
				}

				//Recover switches by name
				try{
					dpid2 = switch_manager::get_switch_dpid(lsi_name2);
				}catch(eOfSmDoesNotExist& e){
					XDPD_ERR(YAML_PLUGIN_ID "%s: unable to create virtual link %s. LSI '%s' does not exist.\n",
							VIRTUAL, vlink_name.c_str(), lsi_name2.c_str());
					throw eYamlConfParseError();
				}

				unsigned int port_num1 = 0;
				unsigned int port_num2 = 0;

				//Call port manager API
				port_manager::connect_switches(dpid1, &port_num1, port_name1, dpid2, &port_num2, port_name2);
			}
		}
	}
}
