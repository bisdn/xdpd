#ifndef CONFIG_OF_LSI_PLUGIN_H
#define CONFIG_OF_LSI_PLUGIN_H 

#include "../scope.h"

/**
* @file lsi_scope.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Openflow libconfig file hierarchy 
* 
*/

namespace xdpd {

class lsi_scope:public scope {
	
public:
	lsi_scope(std::string scope_name, bool mandatory=false);
		
protected:

};

}// namespace xdpd 

#endif /* CONFIG_OF_LSI_PLUGIN_H_ */


