#ifndef CONFIG_OPENFLOW_PLUGIN_H
#define CONFIG_OPENFLOW_PLUGIN_H 

#include "../scope.h"

/**
* @file openflow_scope.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Openflow libconfig file hierarchy 
* 
*/

namespace xdpd {

class openflow_scope:public scope {
	
public:
	openflow_scope(scope* parent);
		
protected:
	
};

class of_lsis_scope:public scope {
	
public:
	of_lsis_scope(scope* parent);
		
protected:
	
	virtual void pre_validate(libconfig::Setting& setting, bool dry_run);
};

}// namespace xdpd 

#endif /* CONFIG_OPENFLOW_PLUGIN_H_ */


