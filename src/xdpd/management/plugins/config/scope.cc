#include "scope.h"

using namespace xdpd;
using namespace rofl;

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

	std::vector<scope*>::iterator scope_iter;
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {
		if(*scope_iter == sc)
			throw eConfDuplicatedScope();
			
	}
	
	sub_scopes.push_back(sc);
}

void scope::register_parameter(std::string _name, bool mandatory){

	if(parameters.find(_name) != parameters.end())
		throw eConfDuplicatedParameter();
	

	parameters[_name] = mandatory;
}


void scope::execute(libconfig::Setting& setting, bool dry_run){
	
	//Call pre-hook
	pre_validate(setting, dry_run);

	//Go through parameters and validate if mandatory
	std::map<std::string, bool>::iterator param_iter;
	for (param_iter = parameters.begin(); param_iter != parameters.end(); ++param_iter) {
		if(param_iter->second && !setting.exists(param_iter->first.c_str())){
			ROFL_ERR("%s: mandatory parameter '%s' not found\n", setting.getPath().c_str(), param_iter->first.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
	}
	
	//Go through sub scopes
	std::vector<scope*>::iterator scope_iter;
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {
		if(	(*scope_iter)->mandatory && 
			(	! setting.exists((*scope_iter)->name) || 
				! setting[(*scope_iter)->name].isGroup()
			)
		){
			ROFL_ERR("%s: mandatory subscope '%s' not found\n", setting.getPath().c_str(), (*scope_iter)->name.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
		
		if(setting.exists((*scope_iter)->name))
			(*scope_iter)->execute(setting[(*scope_iter)->name], dry_run);
	}

	//Call post-hook
	post_validate(setting, dry_run);
}

void scope::execute(libconfig::Config& config, bool dry_run){

	//Call pre-hook
	pre_validate(config, dry_run);
	
	//Go through parameters and validate if mandatory
	std::map<std::string, bool>::iterator param_iter;
	for (param_iter = parameters.begin(); param_iter != parameters.end(); ++param_iter) {
		if(param_iter->second && !config.exists(param_iter->first.c_str())){
			ROFL_ERR("%s: mandatory parameter '%s' not found\n", name.c_str(), param_iter->first.c_str());
			throw eConfMandatoryParameterNotPresent();
		}
	}
	
	//Go through sub scopes
	std::vector<scope*>::iterator scope_iter;
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {
		if(	(*scope_iter)->mandatory && 
			(	! config.exists((*scope_iter)->name) || 
				! config.lookup((*scope_iter)->name).isGroup()
			)
		){
			ROFL_ERR("%s: mandatory subscope '%s' not found\n", name.c_str(), (*scope_iter)->name.c_str());
	
			throw eConfMandatoryParameterNotPresent();
		}
		if(config.exists((*scope_iter)->name))
			(*scope_iter)->execute(config.lookup((*scope_iter)->name), dry_run);
	}

	
	//Call post-hook
	post_validate(config, dry_run);
}

