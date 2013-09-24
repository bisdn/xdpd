#include "interfaces_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_pipeline.h>
#include "../../../switch_manager.h"
#include "../../../port_manager.h"
#include "../../../../openflow/openflow_switch.h"


using namespace xdpd;
using namespace rofl;

#define VIF_VIF "vif"
#define VIF_LINK "link"
#define VIF_LSI "lsi"
#define VIF_DESCRIPTION "description"


interfaces_scope::interfaces_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	//Register parameters
	//None for the moment

	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	register_subscope(new virtual_ifaces_scope());	

}


virtual_ifaces_scope::virtual_ifaces_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	
}


void virtual_ifaces_scope::post_validate(libconfig::Setting& setting, bool dry_run){

	std::map<std::string, std::string> partial_links; //linkname, lsiname
	std::string lsi_name;

	if(setting.getLength()%2 != 0){
		ROFL_ERR("%s: malformed virtual interfaces section. There must be defined in pairs\n", name.c_str());
		throw eConfParseError(); 	
		
	}
	
	//Detect existing subscopes (logical switches) and register
 	for(int i = 0; i<setting.getLength(); ++i){
		if( !setting[i].exists(VIF_LINK) || !setting[i].exists(VIF_LSI) ){
			ROFL_ERR("%s: missing '%s' and/or '%s' mandatory parameters in vif '%s' configuration section.\n", name.c_str(), VIF_LINK, VIF_LSI, setting[i].getName());
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
 
			//Complete vlink
			if(!dry_run){
				std::string port_name1, port_name2;
				openflow_switch *sw1, *sw2;			

				//Recover switches by name
				sw1 = switch_manager::find_by_name(lsi_name);
				if(!sw1){
					ROFL_ERR("%s: unable to create vlink. Switch '%s' does not exist.\n", name.c_str(), lsi_name.c_str());
					throw eConfParseError(); 	
					
				}
		
				//Recover switches by name
				sw2 = switch_manager::find_by_name(value);
				if(!sw2){
					ROFL_ERR("%s: unable to create vlink. Switch '%s' does not exist.\n", name.c_str(), value.c_str());
					throw eConfParseError(); 	
					
				}		
				if(sw2 == sw1){
					ROFL_ERR("%s: unable to create vlink. Switch '%s' cannot be connected to itself (loop)!\n", name.c_str(), value.c_str());
					throw eConfParseError(); 
					
				}	
				//Call port manager API
				port_manager::connect_switches(sw1->dpid, port_name1, sw2->dpid, port_name2);
			}
		}
	}
	
	//Check size of partial_links. Must be zero
	if(partial_links.size() > 0){
		ROFL_ERR("%s: error, some virtual interfaces could not be connected. This is likely due to one or more unpaired '%s' values.\n", name.c_str(), VIF_LINK);
		throw eConfParseError(); 	
	}
}
