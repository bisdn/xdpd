#ifndef CONFIG_SCOPE_PLUGIN_H
#define CONFIG_SCOPE_PLUGIN_H 

#include <assert.h>
#include <libconfig.h++> 
#include <map> 
#include <sstream> 
#include <vector> 
#include <rofl/common/croflexception.h>
#include <rofl/common/utils/c_logger.h>

/**
* @file scope_plugin.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief libconfig based scope 
* 
*/

namespace xdpd {

class eConfParamNotFound: public rofl::RoflException {};
class eConfFileNotFound: public rofl::RoflException {};
class eConfParseError: public rofl::RoflException {};

class eConfDuplicatedScope: public rofl::RoflException {};
class eConfDuplicatedParameter: public rofl::RoflException {};
class eConfMandatoryParameterNotPresent: public rofl::RoflException {};
class eConfMandatoryScopeNotPresent: public rofl::RoflException {};
class eConfUnknownElement: public rofl::RoflException {};

class scope {
	
public:
	scope(std::string scope_name, scope* parent, bool mandatory=false);
	virtual ~scope();
	
	void execute(libconfig::Setting& setting, bool dry_run=false);
	void execute(libconfig::Config& setting, bool dry_run=false);

	std::string name;
protected:

	//Scope types
	typedef enum { 
		//Normal scope, to be executed in the same order
		//as added
		SCOPE_NORMAL,
		
		//Specially ordered scope, to be executed before
		//any normal scope. Does not propagate to subscopes
		SCOPE_ORDERED,
		
		//Specially ordered scope, to be executed before
		//any normal scope. It does propagate to NORMAL subscopes
		SCOPE_ORDERED_TRANS_SUB,				
	}scope_types_t;

	//Type of scope
	scope_types_t type;

	//Processed flag
	bool processed;
	
	//Is scope really mandatory
	bool mandatory;
	
	//Parent pointer
	scope* parent;
	
	//Contents of the scope
	std::vector<scope*> sub_scopes;
	std::map<std::string, bool> parameters;

	//Register methods
	void register_subscope(std::string name, scope* sc);
	void register_subscope(scope* sc){register_subscope(sc->name,sc);};
	void register_parameter(std::string name, bool mandatory=false);

	//Geters
	scope* get_subscope(const std::string& name){
		std::vector<scope*>::iterator scope_iter;
	
		for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {
			if((*scope_iter)->name == name)
				return *scope_iter;
		}
		return NULL;
	}

	//Get the very initial root scope(internal)
	scope* __get_root(){
		if(parent)
			return parent->__get_root();
		return this;
	}
	
	//Get scope object by its absolute path
	//x.y.z
	scope* get_scope_abs_path(const std::string& abs_path);

	//Get our path
	std::string get_path(){
		std::stringstream ss("");
		
		if(parent)
			ss<<parent->get_path()<<".";
	
		ss << this->name;
		
		return ss.str();
	}

	//Pre-execute hooks	
	virtual void __pre_execute(libconfig::Config& config, bool dry_run);
	virtual void __pre_execute(libconfig::Setting& setting, bool dry_run);
	

	//Pre-execute hooks	
	virtual void pre_execute(libconfig::Config& config, bool dry_run){};
	virtual void pre_execute(libconfig::Setting& setting, bool dry_run){};
	
	//Allow actions before and after parameter and scope validation	
	virtual void pre_validate(libconfig::Config& config, bool dry_run){};
	virtual void post_validate(libconfig::Config& config, bool dry_run){};
	
	virtual void pre_validate(libconfig::Setting& setting, bool dry_run){};
	virtual void post_validate(libconfig::Setting& setting, bool dry_run){};

};


}// namespace xdpd 

#endif /* CONFIG_SCOPE_PLUGIN_H_ */


