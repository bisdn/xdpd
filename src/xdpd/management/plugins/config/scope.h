#ifndef CONFIG_SCOPE_PLUGIN_H
#define CONFIG_SCOPE_PLUGIN_H 

#include <libconfig.h++> 
#include <map> 
#include <vector> 
#include <rofl/common/cerror.h>
#include <rofl/common/utils/c_logger.h>

/**
* @file scope_plugin.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief libconfig based scope 
* 
*/

namespace xdpd {

class eConfParamNotFound: public rofl::cerror {};
class eConfFileNotFound: public rofl::cerror {};
class eConfParseError: public rofl::cerror {};

class eConfDuplicatedScope: public rofl::cerror {};
class eConfDuplicatedParameter: public rofl::cerror {};
class eConfMandatoryParameterNotPresent: public rofl::cerror {};
class eConfMandatoryScopeNotPresent: public rofl::cerror {};

class scope {
	
public:
	scope(std::string scope_name, bool mandatory=false);
	virtual ~scope();
	
	void execute(libconfig::Setting& setting, bool dry_run=false);
	void execute(libconfig::Config& setting, bool dry_run=false);

	std::string name;
protected:
	bool mandatory;
	std::vector<scope*> sub_scopes;
	std::map<std::string, bool> parameters;

	//Register methods
	void register_subscope(std::string name, scope* sc);
	void register_subscope(scope* sc){register_subscope(sc->name,sc);};
	void register_parameter(std::string name, bool mandatory=false);
	
	//Allow actions before and after parameter and scope validation	
	virtual void pre_validate(libconfig::Config& config, bool dry_run){};
	virtual void post_validate(libconfig::Config& config, bool dry_run){};
	
	virtual void pre_validate(libconfig::Setting& setting, bool dry_run){};
	virtual void post_validate(libconfig::Setting& setting, bool dry_run){};

};


}// namespace xdpd 

#endif /* CONFIG_SCOPE_PLUGIN_H_ */


