#include "scope.h"

using namespace xdpd::plugins::yaml_config;

#include "yaml_config.h"

scope::scope(std::string scope_name, scope* parent, bool mandatory)
{
	//Assign basic
	this->name = scope_name;
	this->parent = parent;
	this->mandatory = mandatory;
	this->__processed = false;
	this->__type = NORMAL_SCOPE;
}

scope::~scope(){

	//Destroy all subscopes
	std::vector<scope*>::iterator scope_iter;
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter)
		delete *scope_iter;
	sub_scopes.clear();
}


scope* scope::get_scope_abs_path(const std::string& abs_path){

	//First recover the root scope
	scope* root = __get_root();
	scope* curr = NULL;

	assert(root != NULL);
	assert(root->parent == NULL);

	//Check if there is only one level
	std::stringstream ss(abs_path);
	std::string item;
	char delim = '.';

	//Split the absolute path
	while (std::getline(ss, item, delim)) {
		if(!curr){
			curr = root->get_subscope(item);
		}else{
			curr = curr->get_subscope(item);
		}
		if(!curr)
			return NULL;
	}

	return curr;
}

void scope::register_priority_subscope(const std::string& _name, scope* sc, unsigned int priority, bool taint_subscopes){
	//Register in the normal subscopes list
	register_subscope(_name, sc);

	//Add to the priority queue
	scope* root = __get_root();
	assert(root != NULL);
	assert(root->parent == NULL);

	//Set new type
	if(taint_subscopes)
		sc->__type = PRIORITY_TAINT_SCOPE;
	else
		sc->__type = PRIORITY_SCOPE;
	//Check
	if( root->priority_sub_scopes.find(priority) == root->priority_sub_scopes.end() ){
		//Add
		root->priority_sub_scopes[priority] = sc;		
	}else{
		//An element is register with the same priori
		XDPD_ERR(YAML_PLUGIN_ID "%s: ERROR; failed to add scope '%s' with priority %d. Scope '%s' is already registered with this priority\n", sc->get_path().c_str(), priority, root->priority_sub_scopes[priority]->get_path().c_str());
		assert(0);
		throw eYamlConfDuplicatedPriority();
	}
}	

void scope::register_subscope(const std::string& _name, scope* sc){

	//Look for the key in subscopes 
	if(get_subscope(sc->name))
		throw eYamlConfDuplicatedScope();

	sub_scopes.push_back(sc);
}

void scope::register_parameter(std::string _name, bool mandatory){

	if(parameters.find(_name) != parameters.end())
		throw eYamlConfDuplicatedParameter();
	

	parameters[_name] = mandatory;
}

void scope::execute(YAML::Node node, bool dry_run, bool priority_call){

	if(this->__processed && this->__type == PRIORITY_TAINT_SCOPE){
		//current and inner scopes have been already processed
		return;
	}

	if(this->__processed == false){
		//Call pre-hook
		XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Calling 'pre_validate()'\n", get_path().c_str());
		pre_validate(node, dry_run);

		//Detect invalid paramaters if they are fixed
		if(sub_scopes.size() != 0 || parameters.size() != 0){
			if (node.IsMap()) {
				for (auto it : node) {
					std::string subscope_name = it.first.as<std::string>();

					//Look for the key in parameters
					if(parameters.find(subscope_name) != parameters.end())
						continue;

					//Look for the key in subscopes
					if(get_subscope(subscope_name))
						continue;

					//Not found, so not recognised. Throw exception
					XDPD_ERR(YAML_PLUGIN_ID "%s: ERROR, unknow parameter or scope '%s'. Perhaps an old configuration file syntax?\n", name.c_str(), subscope_name.c_str());

					throw eYamlConfUnknownElement();
				}
			}
		}
		
		//Go through parameters and validate if mandatory
		std::map<std::string, bool>::iterator param_iter;
		for (param_iter = parameters.begin(); param_iter != parameters.end(); ++param_iter) {
			if(param_iter->second && !node[param_iter->first.c_str()]){
				XDPD_ERR(YAML_PLUGIN_ID "%s: mandatory parameter '%s' not found\n", name.c_str(), param_iter->first.c_str());
				throw eYamlConfMandatoryParameterNotPresent();
			}
		}
	}

	if( (priority_call && this->__type == PRIORITY_TAINT_SCOPE) || (!priority_call && this->__type != PRIORITY_TAINT_SCOPE)  ){
		//Go through sub scopes
		std::vector<scope*>::iterator scope_iter;
		scope* sc;

		for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {

			//Make it easy
			sc = *scope_iter;

			if(	sc->mandatory && 
				(	! node[sc->name] ||
					! (node[sc->name].IsMap() ||
					   node[sc->name].IsSequence())
				)
			){
				XDPD_ERR(YAML_PLUGIN_ID "%s: mandatory subscope '%s' not found\n", name.c_str(), sc->name.c_str());
				throw eYamlConfMandatoryParameterNotPresent();
			}
			
			if(node[sc->name]){
				XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Calling 'execute()'\n", sc->get_path().c_str());
				sc->execute(node[sc->name], dry_run);
			}
		}
	}
		
	if(this->__processed == false){
		//Call post-hook
		XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Calling 'post_validate()'\n", get_path().c_str());
		post_validate(node, dry_run);
		
		this->__processed = true;
	}
}

void scope::parse(YAML::Node node, bool dry_run){

	assert(parent == NULL);

	if (not node[name]) {
		XDPD_ERR(YAML_PLUGIN_ID "%s: unable to find YAML sub-tree for scope %s\n", name.c_str());
		return;
	}
	subnode = node[name];

	//Execute priority scopes in order
	std::map<unsigned int, scope*>::iterator p_iter;
	for (p_iter = priority_sub_scopes.begin(); p_iter  != priority_sub_scopes.end(); ++p_iter) {
		XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Processing priority {%s} scope 'execute()' with p=%u\n", p_iter->second->get_path().c_str(), (p_iter->second->__type == PRIORITY_TAINT_SCOPE )? "TAINT": "DO NOT TAINT", p_iter->first);
		try{
			if (subnode[p_iter->second->name]) {
				p_iter->second->execute(subnode[p_iter->second->name], dry_run, true);
			}
		}catch(YAML::ParserException& e){
			XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Priority scope with p(%u) NOT present.\n", p_iter->second->get_path().c_str(), p_iter->first);
		}
	}

	//Call pre-hook
	XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Calling 'pre_validate()'\n", get_path().c_str());
	pre_validate(subnode, dry_run);

	//Go through parameters and validate if mandatory
	std::map<std::string, bool>::iterator param_iter;
	for (param_iter = parameters.begin(); param_iter != parameters.end(); ++param_iter) {
		if(param_iter->second && !subnode[param_iter->first.c_str()]){
			XDPD_ERR(YAML_PLUGIN_ID "%s: mandatory parameter '%s' not found\n", name.c_str(), param_iter->first.c_str());
			throw eYamlConfMandatoryParameterNotPresent();
		}
	}
	
	//Go through sub scopes
	std::vector<scope*>::iterator scope_iter;
	scope* sc;
	
	for (scope_iter = sub_scopes.begin(); scope_iter != sub_scopes.end(); ++scope_iter) {

		//Make it easy
		sc = *scope_iter;

		if(	sc->mandatory && 
			(	! subnode[sc->name] ||
				! (subnode[sc->name].IsMap() ||
				   subnode[sc->name].IsSequence())
			)
		){
			XDPD_ERR(YAML_PLUGIN_ID "%s: mandatory subscope '%s' not found\n", name.c_str(), sc->name.c_str());
	
			throw eYamlConfMandatoryParameterNotPresent();
		}
		if(subnode[sc->name]){
			XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Calling 'pre_execute()'\n", sc->get_path().c_str());
			sc->execute(subnode[sc->name], dry_run);
		}
	}

	
	//Call post-hook
	XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "[%s] Calling 'post_validate()'\n", get_path().c_str());
	post_validate(subnode, dry_run);
}

