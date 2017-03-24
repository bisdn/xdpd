#include "interfaces_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>
#include "../../../switch_manager.h"
#include "../../../port_manager.h"
#include "../../../../openflow/openflow_switch.h"

#include "nf_scope.h"

#include "../config.h"

using namespace xdpd;
using namespace rofl;

//Constants
#define BLACKLIST "blacklist"
#define NF "nf"
#define VIF_VIF "vif"
#define VIF_LINK "link"
#define VIF_LSI "lsi"
#define VIF_DESCRIPTION "description"


interfaces_scope::interfaces_scope(scope* parent):scope("interfaces", parent, false){
	
	//Register parameters
	register_parameter(BLACKLIST);

	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	register_priority_subscope(new nf_scope(this), 2, false);	
	register_subscope(new virtual_ifaces_scope(this));	
	

}

void interfaces_scope::post_validate(libconfig::Setting& setting, bool dry_run){
	
	//Update cache
	blacklisted.clear();

	//If the configuration has a blacklist port	
	if(setting.exists(BLACKLIST)){	
		for(int i=0; i<setting[BLACKLIST].getLength(); ++i){
			std::string port = setting[BLACKLIST][i];

			//Check for port existance		
			if(port_manager::exists(port) == false){
				XDPD_ERR(CONF_PLUGIN_ID "%s: attempting to blacklist an invalid port '%s'. Port does not exist!\n", setting.getPath().c_str(), port.c_str());
				throw eConfParseError(); 	
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


void virtual_ifaces_scope::post_validate(libconfig::Setting& setting, bool dry_run){

	std::map<std::string, std::string> partial_links; //linkname, lsiname
	std::map<std::string, bool> provisioned_links; //linkname, true 
	std::string lsi_name;

	if(setting.getLength()%2 != 0){
		XDPD_ERR(CONF_PLUGIN_ID "%s: malformed virtual interfaces section. There must be defined in pairs\n", setting.getPath().c_str());
		throw eConfParseError(); 	
		
	}
	
	//Detect existing subscopes (logical switches) and register
 	for(int i = 0; i<setting.getLength(); ++i){
		if( !setting[i].exists(VIF_LINK) || !setting[i].exists(VIF_LSI) ){
			XDPD_ERR(CONF_PLUGIN_ID "%s: missing '%s' and/or '%s' mandatory parameters in vif '%s' configuration section.\n", setting.getPath().c_str(), VIF_LINK, VIF_LSI, setting[i].getName());
			throw eConfParseError(); 	
	
		}

		std::string key = setting[i][VIF_LINK];
		std::string value = setting[i][VIF_LSI];

		if((partial_links.find(key) == partial_links.end())){
			//Incomplete vlink -> store
			partial_links[key] = value;
		}else{
			//Pop from
			lsi_name = partial_links[key];
			partial_links.erase(key);			

			//Check link name is not repeated
			if(provisioned_links.find(key) != provisioned_links.end()){
				XDPD_ERR(CONF_PLUGIN_ID "%s: duplicated vlink '%s' name!\n", setting.getPath().c_str(), key.c_str());
				throw eConfParseError(); 
			
			}	
	
			//Check self connection
 			if(lsi_name == value){
				XDPD_ERR(CONF_PLUGIN_ID "%s: unable to create vlink. Switch '%s' cannot be connected to itself!\n", setting.getPath().c_str(), value.c_str());
				throw eConfParseError(); 
			}	

			//Complete vlink
			if(!dry_run){
				std::string port_name1, port_name2;
				uint64_t dpid1, dpid2;	

				//Recover switches by name
				try{
					dpid1 = switch_manager::get_switch_dpid(lsi_name);		
				}catch(eOfSmDoesNotExist& e){
					XDPD_ERR(CONF_PLUGIN_ID "%s: unable to create vlink. Switch '%s' does not exist.\n", setting.getPath().c_str(), lsi_name.c_str());
					throw eConfParseError(); 	
					
				}
		
				//Recover switches by name
				try{
					dpid2 = switch_manager::get_switch_dpid(value);
				}catch(eOfSmDoesNotExist& e){
					XDPD_ERR(CONF_PLUGIN_ID "%s: unable to create vlink. Switch '%s' does not exist.\n", setting.getPath().c_str(), value.c_str());
					throw e;
				}		

				unsigned int port_num1 = 0;
				unsigned int port_num2 = 0;
	
				//Call port manager API
				port_manager::connect_switches(dpid1, &port_num1, port_name1, dpid2, &port_num2, port_name2);
			}
			
			//Add to the list of provisioned
			provisioned_links[key] = true;
		}
	}
	
	//Check size of partial_links. Must be zero
	if(partial_links.size() > 0){
		XDPD_ERR(CONF_PLUGIN_ID "%s: error, some virtual interfaces could not be connected. This is likely due to one or more unpaired '%s' values.\n", setting.getPath().c_str(), VIF_LINK);
		throw eConfParseError(); 	
	}
}
