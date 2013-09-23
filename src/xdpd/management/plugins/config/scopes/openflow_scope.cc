#include "openflow_scope.h"
#include "lsi_scope.h"

using namespace xdpd;

openflow_scope::openflow_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	//Register parameters
	//None for the moment

	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	register_subscope("logical_switches", new of_lsis_scope());	

}


of_lsis_scope::of_lsis_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	
}


void of_lsis_scope::pre_execute(libconfig::Setting& setting, bool dry_run){
	
	//Detect existing subscopes (logical switches) and register
 	for(int i = 0; i<setting.getLength(); ++i){
		ROFL_DEBUG("Found logical switch: %s\n", setting[i].getName());
		register_subscope(std::string(setting[i].getName()), new lsi_scope(setting[i].getName()));
	}
		
}

void of_lsis_scope::post_execute(libconfig::Setting& setting, bool dry_run){
	//Destroy all discovered subscopes. Since in each iteration they will (avoid duplications)
	std::map<std::string, scope*>::iterator scope_iter;
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter)
		delete scope_iter->second;
	
	sub_scopes.clear();
		
}
