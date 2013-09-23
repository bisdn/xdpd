#include "scope.h"

using namespace xdpd;
using namespace rofl;

scope::scope(std::string scope_name, bool mandatory){
	this->name = scope_name;
	this->mandatory = mandatory;
}

scope::~scope(){

	//Destroy all subscopes
	std::map<std::string, scope*>::iterator scope_iter;
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter)
		delete scope_iter->second;
	sub_scopes.clear();
		
}
		
void scope::register_subscope(std::string name, scope* sc){

	if(sub_scopes.find(name) != sub_scopes.end())
		throw eConfDuplicatedScope();
	
	sub_scopes[name] = sc;
}

void scope::register_parameter(std::string name, bool mandatory){

	if(parameters.find(name) != parameters.end())
		throw eConfDuplicatedParameter();
	

	parameters[name] = mandatory;
}


void scope::execute(libconfig::Setting& setting, bool dry_run){
	
	//Call pre-hook
	pre_execute(setting, dry_run);

	//Go through parameters and validate if mandatory
	std::map<std::string, bool>::iterator param_iter;
	for (param_iter = parameters.begin(); param_iter != parameters.end(); ++param_iter) {
		if(param_iter->second && !setting.exists(param_iter->first.c_str())){
			ROFL_ERR("Mandatory parameter %s under scope %s not found\n", param_iter->first.c_str(), name.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
	}
	
	//Go through sub scopes
	std::map<std::string, scope*>::iterator scope_iter;
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {
		if(	scope_iter->second->mandatory && 
			(	! setting.exists(scope_iter->first.c_str()) || 
				! setting[scope_iter->first].isGroup()
			)
		){
			ROFL_ERR("Mandatory subscope %s under scope %s not found\n", scope_iter->first.c_str(), name.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
		scope_iter->second->execute(setting[scope_iter->first], dry_run);
	}

	//Call post-hook
	post_execute(setting, dry_run);
}

void scope::execute(libconfig::Config& config, bool dry_run){

	//Call pre-hook
	pre_execute(config, dry_run);
	
	//Go through parameters and validate if mandatory
	std::map<std::string, bool>::iterator param_iter;
	for (param_iter = parameters.begin(); param_iter != parameters.end(); ++param_iter) {
		if(param_iter->second && !config.exists(param_iter->first.c_str())){
			ROFL_ERR("Mandatory parameter %s under scope %s not found\n", param_iter->first.c_str(), name.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
	}
	
	//Go through sub scopes
	std::map<std::string, scope*>::iterator scope_iter;
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {
		if(	scope_iter->second->mandatory && 
			(	! config.exists(scope_iter->first.c_str()) || 
				! config.lookup(scope_iter->first.c_str()).isGroup()
			)
		){
			ROFL_ERR("Mandatory subscope %s under scope %s not found\n", scope_iter->first.c_str(), name.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
		scope_iter->second->execute(config.lookup(scope_iter->first), dry_run);
	}

	
	//Call post-hook
	post_execute(config, dry_run);
}

