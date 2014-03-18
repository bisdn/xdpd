#include "config.h"
#include <rofl/common/utils/c_logger.h>

#include "../../system_manager.h"
#include "root_scope.h"

using namespace xdpd;
using namespace rofl;
using namespace libconfig;

//Constant for the option
const std::string config::CONFIG_FILE_OPT_FULL_NAME="config-file";

config::config(){
}

config::~config(){
	//Remove all objects
}

void config::get_config_file_contents(Config* cfg){

	std::string conf_file;

	if(!system_manager::is_option_set(CONFIG_FILE_OPT_FULL_NAME)){
		ROFL_ERR(CONF_PLUGIN_ID "No configuration file specified either via -c or --config-file\n");	
		throw eConfParamNotFound();
	}

	try{
		conf_file = system_manager::get_option_value(CONFIG_FILE_OPT_FULL_NAME).c_str();
		cfg->readFile(conf_file.c_str());
	}catch(const FileIOException &fioex){
		ROFL_ERR(CONF_PLUGIN_ID "Config file %s not found. Aborting...\n",conf_file.c_str());	
		throw eConfFileNotFound();
		throw fioex;
	}catch(ParseException &pex){
		ROFL_ERR(CONF_PLUGIN_ID "Error while parsing file %s at line: %u \nAborting...\n",conf_file.c_str(),pex.getLine());
		throw eConfParseError();
	}
}

void config::init(){
	Config* cfg = new Config;
	root_scope* root = new root_scope();

	//Dry run
	get_config_file_contents(cfg);
	root->execute(*cfg,true);

	delete cfg;
	delete root;

	//Unless test-config is set, execute the config
	if(!system_manager::is_test_run()) {
		//Execute
		cfg = new Config;
		root = new root_scope();

		get_config_file_contents(cfg);
		root->execute(*cfg);
		delete cfg;
		delete root;
	}
}
