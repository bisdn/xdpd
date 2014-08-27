#include "scope.h"

using namespace xdpd;
using namespace rofl;

#ifndef CONF_PLUGIN_ID
	#define CONF_PLUGIN_ID ""
#endif

scope::scope(std::string scope_name, bool mandatory){
	this->name = scope_name;
	this->mandatory = mandatory;
}

scope::~scope(){

	//Destroy all subscopes
	std::vector<scope*>::iterator scope_iter;
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter)
		delete *scope_iter;
	sub_scopes.clear();
		
}
		
void scope::register_subscope(std::string _name, scope* sc){

	//Look for the key in subscopes 
	if(get_subscope(sc->name))
		throw eConfDuplicatedScope();

	sub_scopes.push_back(sc);
}

void scope::register_parameter(std::string _name, bool mandatory){

	if(parameters.find(_name) != parameters.end())
		throw eConfDuplicatedParameter();
	

	parameters[_name] = mandatory;
}


void scope::__pre_execute(libconfig::Setting& setting, bool dry_run){

	this->pre_execute(setting, dry_run);
	
	std::vector<scope*>::iterator scope_iter;
	scope* sc;

	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {
		sc = *scope_iter;
		if(setting.exists(sc->name))
			sc->__pre_execute(setting[sc->name], dry_run);
	}

}

void scope::execute(libconfig::Setting& setting, bool dry_run){

	//Call pre-hook
	pre_validate(setting, dry_run);

	//Detect invalid paramaters if they are fixed
	if(sub_scopes.size() != 0 || parameters.size() != 0){
		for(int i=0; i < setting.getLength(); ++i){

			std::string aux = setting[i].getName();

			//Look for the key in parameters
			if(parameters.find(aux) != parameters.end())
				continue;

			//Look for the key in subscopes 
			if(get_subscope(aux))
				continue;
	
			//Not found, so not recognised. Throw exception
			ROFL_ERR(CONF_PLUGIN_ID "%s: ERROR, unknow parameter or scope '%s'. Perhaps an old configuration file syntax?\n", setting.getPath().c_str(), aux.c_str());

			throw eConfUnknownElement();
	
		}
	}
	
	//Go through parameters and validate if mandatory
	std::map<std::string, bool>::iterator param_iter;
	for (param_iter = parameters.begin(); param_iter != parameters.end(); ++param_iter) {
		if(param_iter->second && !setting.exists(param_iter->first.c_str())){
			ROFL_ERR(CONF_PLUGIN_ID "%s: mandatory parameter '%s' not found\n", setting.getPath().c_str(), param_iter->first.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
	}
	
	//Go through sub scopes
	std::vector<scope*>::iterator scope_iter;
	scope* sc;

	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {

		//Make it easy
		sc = *scope_iter;

		if(	sc->mandatory && 
			(	! setting.exists(sc->name) || 
				! setting[sc->name].isGroup()
			)
		){
			ROFL_ERR(CONF_PLUGIN_ID "%s: mandatory subscope '%s' not found\n", setting.getPath().c_str(), sc->name.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
		
		if(setting.exists(sc->name))
			sc->execute(setting[sc->name], dry_run);
	}

	//Call post-hook
	post_validate(setting, dry_run);
}


void scope::__pre_execute(libconfig::Config& config, bool dry_run){

	
	std::vector<scope*>::iterator scope_iter;
	scope* sc;

	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {
		sc = *scope_iter;
		if(config.exists(sc->name))
			sc->__pre_execute(config.lookup(sc->name), dry_run);
	}
	
	this->pre_execute(config, dry_run);
}

void scope::execute(libconfig::Config& config, bool dry_run){

	//First execute all pre_execute hooks
	__pre_execute(config, dry_run);
	
	//Call pre-hook
	pre_validate(config, dry_run);

	//Go through parameters and validate if mandatory
	std::map<std::string, bool>::iterator param_iter;
	for (param_iter = parameters.begin(); param_iter != parameters.end(); ++param_iter) {
		if(param_iter->second && !config.exists(param_iter->first.c_str())){
			ROFL_ERR(CONF_PLUGIN_ID "%s: mandatory parameter '%s' not found\n", name.c_str(), param_iter->first.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
	}
	
	//Go through sub scopes
	std::vector<scope*>::iterator scope_iter;
	scope* sc;
	
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {

		//Make it easy
		sc = *scope_iter;

		if(	sc->mandatory && 
			(	! config.exists(sc->name) || 
				! config.lookup(sc->name).isGroup()
			)
		){
			ROFL_ERR(CONF_PLUGIN_ID "%s: mandatory subscope '%s' not found\n", name.c_str(), sc->name.c_str());
	
			throw eConfMandatoryParameterNotPresent();
		}
		if(config.exists(sc->name))
			sc->execute(config.lookup(sc->name), dry_run);
	}

	
	//Call post-hook
	post_validate(config, dry_run);
}

