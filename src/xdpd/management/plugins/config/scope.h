#ifndef CONFIG_SCOPE_PLUGIN_H
#define CONFIG_SCOPE_PLUGIN_H 

#include <assert.h>
#include <libconfig.h++> 
#include <map> 
#include <sstream> 
#include <vector> 

#include "xdpd/common/utils/c_logger.h"
#include "xdpd/common/exception.h"

/**
* @file scope_plugin.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief libconfig based scope 
* 
*/

namespace xdpd {

class eConfParamNotFound: public xdpd::exception {
public:
	eConfParamNotFound() : exception("eConfParamNotFound") {};
};
class eConfFileNotFound: public xdpd::exception {
public:
	eConfFileNotFound() : exception("eConfFileNotFound") {};
};
class eConfParseError: public xdpd::exception {
public:
	eConfParseError() : exception("eConfParseError") {};
};
class eConfDuplicatedScope: public xdpd::exception {
public:
	eConfDuplicatedScope() : exception("eConfDuplicatedScope") {};
};
class eConfDuplicatedPriority: public xdpd::exception {
public:
	eConfDuplicatedPriority() : exception("eConfDuplicatedPriority") {};
};
class eConfDuplicatedParameter: public xdpd::exception {
public:
	eConfDuplicatedParameter() : exception("eConfDuplicatedParameter") {};
};
class eConfMandatoryParameterNotPresent: public xdpd::exception {
public:
	eConfMandatoryParameterNotPresent() : exception("eConfMandatoryParameterNotPresent") {};
};
class eConfMandatoryScopeNotPresent: public xdpd::exception {
public:
	eConfMandatoryScopeNotPresent() : exception("eConfMandatoryScopeNotPresent") {};
};
class eConfUnknownElement: public xdpd::exception {
public:
	eConfUnknownElement() : exception("eConfUnknownElement") {};
};

class scope {
	
public:
	scope(std::string scope_name, scope* parent, bool mandatory=false);
	virtual ~scope();
	
	void execute(libconfig::Setting& setting, bool dry_run=false, bool priority_call=false);
	void execute(libconfig::Config& setting, bool dry_run=false);

	std::string name;
protected:

	//Scope types
	typedef enum { 
		//Normal scope, to be executed in the same order as added
		//and after priority scopes
		NORMAL_SCOPE,
		
		//Specially ordered scope, with priority over normal scopes 
		//(executed before). Do not taint subscopes
		PRIORITY_SCOPE,
		
		//Specially ordered scope, with priority over normal scopes
		//(executed before). Executes the inner subscopes with the
		//same priority.
		PRIORITY_TAINT_SCOPE,
	}__scope_type_t;

	//Type of scope
	__scope_type_t __type;

	//Processed flag
	bool __processed;
	
	//Is scope really mandatory
	bool mandatory;
	
	//Parent pointer
	scope* parent;
	
	//Root Config element
	libconfig::Config* __root_config;
	
	//Contents of the scope
	std::map<unsigned int, scope*> priority_sub_scopes;
	std::vector<scope*> sub_scopes;
	std::map<std::string, bool> parameters;

	//Register methods
	void register_subscope(const std::string& name, scope* sc);
	void register_priority_subscope(const std::string& name, scope* sc, unsigned int priority, bool taint_subscopes);
	void register_subscope(scope* sc){register_subscope(sc->name,sc);};
	void register_priority_subscope(scope* sc, unsigned int priority, bool taint_subscopes){register_priority_subscope(sc->name, sc, priority, taint_subscopes);};
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

	//Get libconfig setting
	 libconfig::Setting& __get_libconfig_setting(scope* sc){

		std::string name = sc->get_path();
		scope* root = __get_root();
		assert(root != NULL);
		
#ifdef DEBUG
		//Get the setting if exists
		if(root->__root_config->exists(name)){
			assert(root->__root_config->lookup(name).isGroup() == true);
		}
#endif
		return root->__root_config->lookup(name);
	}
	
	//Get scope object by its absolute path
	//x.y.z
	scope* get_scope_abs_path(const std::string& abs_path);

	//Get our path
	std::string get_path(){
		std::stringstream ss("");
		
		if(parent){
			std::string p = parent->get_path();
			if( p.empty() == false )
				ss<<p<<".";
			ss<<this->name;
		}
		
		return ss.str();
	}

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


