#include "system_manager.h"

#include <sstream>
#include <rofl/common/ciosrv.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/platform/unix/cdaemon.h>
#include "switch_manager.h"
#include "port_manager.h"
#include "plugin_manager.h"


using namespace xdpd;
using namespace rofl;

//getopt
extern int optind;

/*
* Static member initialization
*/
bool system_manager::inited = false;
fwd_module_info_t system_manager::info;
rofl::cunixenv* system_manager::env_parser = NULL;
const std::string system_manager::XDPD_CLOG_FILE="./xdpd.log";
const std::string system_manager::XDPD_LOG_FILE="/var/log/xdpd.log";
const std::string system_manager::XDPD_PID_FILE="/var/run/xdpd.pid";
const unsigned int system_manager::XDPD_DEFAULT_DEBUG_LEVEL=5; //NOTICE


//Handler to stop ciosrv
void interrupt_handler(int dummy=0) {
	//Only stop ciosrv 
	ciosrv::stop();
}

//Prints version and build numbers and exits
std::string system_manager::get_version(){

	std::stringstream ss("");
	
	//Print version and exit
	ss << "The eXtensible OpenFlow Datapath daemon (xDPd)" << std::endl;	
	ss << "Version: "<< XDPD_VERSION << std::endl;

#ifdef XDPD_BUILD
	ss << "Build: " << XDPD_BUILD << std::endl;
	ss << "Compiled in branch: " << XDPD_BRANCH << std::endl;
	ss << "\t" << XDPD_DESCRIBE << std::endl;
#endif	

	//TODO: add forwarding module

	ss << "\n-- Libraries --" << std::endl;
	ss << "[ROFL]" << std::endl;
	ss << "ROFL version: " << ROFL_VERSION << std::endl;
	ss << "ROFL build: " << ROFL_BUILD_NUM << std::endl;
	ss << "ROFL compiled in branch: " << ROFL_BUILD_BRANCH << std::endl;
	ss << "\t" << ROFL_BUILD_DESCRIBE << std::endl << std::endl;
	
	return ss.str();
}


void system_manager::init(int argc, char** argv){

	bool dry_run;
	unsigned int debug_level;

	//Prevent double calls to init()
	if(inited)
		ROFL_ERR("[xdpd][system] ERROR: double call to system_amanager::init(). This can only be caused by a spurious call from a misbehaving plugin. Please notify this error. Continuing execution...\n");

	//Mark as initied
	inited = true;
	
	/* Parse arguments. Add first additional arguments */
	env_parser = new cunixenv(argc, argv);
	
	/* update defaults */
	env_parser->update_default_option("logfile", XDPD_CLOG_FILE);
	env_parser->add_option(coption(true, NO_ARGUMENT, 'v', "version", "Retrieve xDPd version and exit", std::string("")));

	//Request extra arguments to the plugin manager
	//XXX 

	//Parse arguments
	env_parser->parse_args();

	//If -v is set, print version and return immediately
	if (env_parser->is_arg_set("version")) {
		ROFL_INFO(get_version().c_str());	
		return;
	}
	
	//Set debugging
	debug_level = XDPD_DEFAULT_DEBUG_LEVEL;
	if(env_parser->is_arg_set("debug"))
		debug_level = atoi(env_parser->get_arg("debug").c_str());
	logging::set_debug_level(debug_level);
	#if DEBUG && VERBOSE_DEBUG
		//Set verbose debug if necessary
		rofl_set_logging_level(/*cn,*/ DBG_VERBOSE_LEVEL);
	#endif

	//Daemonize
	if (env_parser->is_arg_set("daemonize")) {
		rofl::cdaemon::daemonize(XDPD_PID_FILE, XDPD_LOG_FILE);
		rofl::logging::notice << "[xdpd][system] daemonizing successful" << std::endl;
	}

	//Set dry-run flag
	dry_run = env_parser->is_arg_set(std::string("test-config"));

	//Capture control+C
	signal(SIGINT, interrupt_handler);
		
	if(dry_run)
		rofl::logging::notice << "[xdpd][system] Launched with -t (--test-config). Doing a dry-run execution" << std::endl;

	//Forwarding module initialization
	if(fwd_module_init(NULL/*XXX*/) != AFA_SUCCESS){
		ROFL_ERR("[xdpd][system] ERROR: initialization of platform driver failed! Aborting...\n");	
		exit(EXIT_FAILURE);
	}

	//Set forwarding module info cache
	fwd_module_get_info(&info);

	//Load plugins
	optind=0;
	plugin_manager::init(argc, argv);

	//If test-config is not set, launch ciosrv loop, otherwise terminate execution
	if(!dry_run){
		//ciosrv run. Only will stop in Ctrl+C
		ciosrv::run();
	}

	//Printing nice trace
	ROFL_INFO("\n[xdpd][system] Shutting down...\n");	

	//Destroy all state
	switch_manager::destroy_all_switches();

	//Call fwd_module to shutdown
	fwd_module_destroy();
	
	//Let plugin manager destroy all registered plugins
	//This must be after calling fwd_module_destroy()
	plugin_manager::destroy();
	
	//Logging	
	logging::close();

	//Release ciosrv loop resources
	rofl::cioloop::shutdown();

	//Release cunixenv
	delete env_parser;	

	ROFL_INFO("\n[xdpd][system] Shutted down.\n");	
}

std::string system_manager::get_option_value(std::string& option){

	//XXX
	return std::string();
};


