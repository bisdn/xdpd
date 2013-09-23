#include "config.h"
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/utils/c_logger.h>

//sub scopes
#include "scopes/openflow_scope.h" 

using namespace xdpd;
using namespace rofl;
using namespace libconfig;


config::config():scope("Root"){
	
	//Interfaces subhierarchy
	
	//Openflow subhierarchy
	register_subscope("openflow", new openflow_scope());	
}

config::~config(){
	//Remove all objects
}

void config::init(int args, char** argv){
	Config cfg;
	std::string conf_file;

	if(!cunixenv::getInstance().is_arg_set("config-file")){
		ROFL_ERR("No configuration file specified either via -c or --config-file\n");	
		throw eConfParamNotFound();
	}

	try{
		conf_file= cunixenv::getInstance().get_arg("config-file").c_str();
		cfg.readFile(conf_file.c_str());
	}catch(const FileIOException &fioex){
		ROFL_ERR("Config file %s not found. Aborting...\n",conf_file.c_str());	
		throw eConfFileNotFound();
		throw fioex;
	}catch(const ParseException &pex){
		ROFL_ERR("Error while parsing file %s at line: %u \nAborting...\n",conf_file.c_str(),pex.getLine());
		throw eConfParseError();
	}
	
	//Dry run
	execute(cfg,true);

	//Execute
	execute(cfg);
}
