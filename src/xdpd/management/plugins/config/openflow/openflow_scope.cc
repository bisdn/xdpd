#include "openflow_scope.h"
#include "lsi_scope.h"

using namespace xdpd;

openflow_scope::openflow_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	//Register parameters
	//None for the moment

	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	register_subscope(new of_lsis_scope());	

}


of_lsis_scope::of_lsis_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	
}


void of_lsis_scope::pre_validate(libconfig::Setting& setting, bool dry_run){

	if(setting.getLength() == 0){
		ROFL_ERR("%s: No logical switches found!\n", setting.getPath().c_str());
		throw eConfParseError(); 	
		
	}
	
	//Detect existing subscopes (logical switches) and register
 	for(int i = 0; i<setting.getLength(); ++i){
		ROFL_DEBUG_VERBOSE("Found logical switch: %s\n", setting[i].getName());
		register_subscope(std::string(setting[i].getName()), new lsi_scope(setting[i].getName()));
	}
		
}
