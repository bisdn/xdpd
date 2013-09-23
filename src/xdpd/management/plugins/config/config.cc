#include "config.h"
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/utils/c_logger.h>

#include "root_scope.h"

using namespace xdpd;
using namespace rofl;
using namespace libconfig;


config::config(){
}

config::~config(){
	//Remove all objects
}

void config::parse_config(Config* cfg){

	std::string conf_file;

	if(!cunixenv::getInstance().is_arg_set("config-file")){
		ROFL_ERR("No configuration file specified either via -c or --config-file\n");	
		throw eConfParamNotFound();
	}

	try{
		conf_file = cunixenv::getInstance().get_arg("config-file").c_str();
		cfg->readFile(conf_file.c_str());
	}catch(const FileIOException &fioex){
		ROFL_ERR("Config file %s not found. Aborting...\n",conf_file.c_str());	
		throw eConfFileNotFound();
		throw fioex;
	}catch(const ParseException &pex){
		ROFL_ERR("Error while parsing file %s at line: %u \nAborting...\n",conf_file.c_str(),pex.getLine());
		throw eConfParseError();
	}
}

void config::init(int args, char** argv){
	Config* cfg = new Config;
	root_scope* root = new root_scope();

	//Dry run
	parse_config(cfg);
	root->execute(*cfg,true);
	delete cfg;
	delete root;

	//Execute
	cfg = new Config;
	root = new root_scope();

	parse_config(cfg);
	root->execute(*cfg,true);
	delete cfg;
	delete root;
}
