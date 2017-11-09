#ifndef XDPD_PLUGINS_YAML_CONFIG_SCOPE_H
#define XDPD_PLUGINS_YAML_CONFIG_SCOPE_H

#include <assert.h>
#include <yaml-cpp/yaml.h>
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
namespace plugins {
namespace yaml_config {

class eYamlConfParamNotFound: public xdpd::exception {
public:
	eYamlConfParamNotFound() : exception("eYamlConfParamNotFound") {};
};
class eYamlConfFileNotFound: public xdpd::exception {
public:
	eYamlConfFileNotFound() : exception("eYamlConfFileNotFound") {};
};
class eYamlConfParseError: public xdpd::exception {
public:
	eYamlConfParseError() : exception("eYamlConfParseError") {};
};
class eYamlConfDuplicatedScope: public xdpd::exception {
public:
	eYamlConfDuplicatedScope() : exception("eYamlConfDuplicatedScope") {};
};
class eYamlConfDuplicatedPriority: public xdpd::exception {
public:
	eYamlConfDuplicatedPriority() : exception("eYamlConfDuplicatedPriority") {};
};
class eYamlConfDuplicatedParameter: public xdpd::exception {
public:
	eYamlConfDuplicatedParameter() : exception("eYamlConfDuplicatedParameter") {};
};
class eYamlConfMandatoryParameterNotPresent: public xdpd::exception {
public:
	eYamlConfMandatoryParameterNotPresent() : exception("eYamlConfMandatoryParameterNotPresent") {};
};
class eYamlConfMandatoryScopeNotPresent: public xdpd::exception {
public:
	eYamlConfMandatoryScopeNotPresent() : exception("eYamlConfMandatoryScopeNotPresent") {};
};
class eYamlConfUnknownElement: public xdpd::exception {
public:
	eYamlConfUnknownElement() : exception("eYamlConfUnknownElement") {};
};

class scope {
	
public:
	scope(std::string scope_name, scope* parent, bool mandatory=false);
	virtual ~scope();
	
	void parse(YAML::Node node, bool dry_run=false);
	void execute(YAML::Node node, bool dry_run=false, bool priority_call=false);

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
	YAML::Node subnode;
	
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

	// Get YAML::Node for a specific path
	YAML::Node& get_node() {
		return subnode;
	};

	//Pre-execute hooks	
	virtual void pre_execute(YAML::Node& node, bool dry_run){};
	
	//Allow actions before and after parameter and scope validation	
	virtual void pre_validate(YAML::Node& node, bool dry_run){};
	virtual void post_validate(YAML::Node& node, bool dry_run){};

};

}// namespace yaml_config
}// namespace plugins
}// namespace xdpd 

#endif /* XDPD_PLUGINS_YAML_CONFIG_SCOPE_H */


