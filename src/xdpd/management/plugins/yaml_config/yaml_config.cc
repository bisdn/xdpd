#include "yaml_config.h"

#include "xdpd/common/utils/c_logger.h"

#include "../../system_manager.h"
#include "root_scope.h"

using namespace xdpd::plugins::yaml_config;

//Constant for the option
const std::string yaml_config::YAML_FILE_OPT_FULL_NAME="yaml-conf";

yaml_config::yaml_config(){
}

yaml_config::~yaml_config(){
	//Remove all objects
}

YAML::Node& yaml_config::reload_yaml_file(){

	std::string conf_file;

	if(!system_manager::is_option_set(YAML_FILE_OPT_FULL_NAME)){
		XDPD_INFO(YAML_PLUGIN_ID "No configuration file specified neither via -y nor --yaml-conf\n");
		throw eYamlConfParamNotFound();
	}

	try{
		conf_file = system_manager::get_option_value(YAML_FILE_OPT_FULL_NAME).c_str();
		cfg = YAML::LoadFile(conf_file.c_str());
		return cfg;

	}catch(YAML::BadFile& e){
		XDPD_ERR(YAML_PLUGIN_ID "Config file %s not found. Aborting...\n",conf_file.c_str());
		throw eYamlConfFileNotFound();
	}
}

void yaml_config::init(){

	try {
		root_scope* root = new root_scope();

		//Dry run
		XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "Starting dry-run\n");
		reload_yaml_file();
		root->parse(cfg,true);
		delete root;

		//Unless test-config is set, execute the config
		if(!system_manager::is_test_run()) {
			//Execute
			root = new root_scope();
			XDPD_DEBUG_VERBOSE(YAML_PLUGIN_ID "Starting real execution\n");
			root->parse(cfg);
			delete root;
		}
	} catch (eYamlConfParamNotFound& e) {
		// let other plugins continue to work
	}
}
