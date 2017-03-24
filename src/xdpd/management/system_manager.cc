#include "system_manager.h"

#include <rofl_common_conf.h>
#include <rofl_datapath_conf.h>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

#include <rofl_common.h>
#include <rofl/datapath/pipeline/util/logging.h>

#include "xdpd/common/cdaemon.h"
#include "switch_manager.h"
#include "port_manager.h"
#include "plugin_manager.h"
#include "plugins/pm_timestamp.h" //Regnerate on configure for version numbers

using namespace xdpd;

//getopt
extern int optind;

/*
* Static member initialization
*/
bool system_manager::inited = false;
hal_extension_ops_t system_manager::hal_extension_ops;
std::string system_manager::id("000000001");
driver_info_t system_manager::driver_info;
xdpd::cunixenv* system_manager::env_parser = NULL;
const std::string system_manager::XDPD_CLOG_FILE="./xdpd.log";
const std::string system_manager::XDPD_LOG_FILE="/var/log/xdpd.log";
const std::string system_manager::XDPD_PID_FILE="/var/run/xdpd.pid";
const unsigned int system_manager::XDPD_DEFAULT_DEBUG_LEVEL=3; //ERROR

const std::string system_manager::XDPD_TEST_RUN_OPT_FULL_NAME="test-config";
const std::string system_manager::XDPD_EXTRA_PARAMS_OPT_FULL_NAME="extra-params";
pthread_t system_manager::ciosrv_thread = 0;

void interrupt_handler(int dummy=0) {
	/* not used any more, TODO: remove handler */
}


std::string system_manager::__get_driver_extra_params(){

	std::string extra="";
	std::string extra_plugins;

	//First recover the CL option if exists
	if(is_option_set(XDPD_EXTRA_PARAMS_OPT_FULL_NAME))
		extra = get_option_value(XDPD_EXTRA_PARAMS_OPT_FULL_NAME);

	//Recover plugins extra
	extra_plugins = plugin_manager::__get_driver_extra_params();

	if(extra_plugins != ""){
		if(extra != ""){
			//Notify user
			XDPD_ERR("[xdpd][system_manager] Warning: Ignoring extra driver parameters provided by plugins (%s), since xDPd was launched with -e (%s)\n", extra_plugins.c_str(), extra.c_str());
		}else
			extra = extra_plugins;
	}

	return extra;
}

//Set the debug level
void system_manager::set_logging_debug_level(unsigned int level){

	enum xdpd_debug_levels c_level;
	
	if( inited && env_parser->is_arg_set("debug") ){
		XDPD_ERR("[xdpd][system_manager] Ignoring the attempt to set_logging_debug_level(); logging level set via command line has preference.\n");
		throw eSystemLogLevelSetviaCLI(); 
	}

	//Validate level and map to C
	switch(level){
		case logging::EMERG:
		case logging::ALERT:
		case logging::CRIT:
		case logging::ERROR: c_level = ERROR_LEVEL;
			break;
		case logging::WARN: c_level = WARN_LEVEL;
			break;
		case logging::NOTICE:
		case logging::INFO: c_level = INFO_LEVEL;
			break;
		case logging::DBG: c_level = DBG_LEVEL;
			break;
		case logging::DBG2: c_level = DBG_LEVEL;
			break;
		case logging::DBG3: c_level = DBG_LEVEL;
			break;
		case logging::TRACE: c_level = DBG_VERBOSE_LEVEL;
			break;
		default:
			throw eSystemLogInvalidLevel(); 
	}
	//Adjust C++ logging debug level	
	logging::set_debug_level(level);
	
	//Adjust C logging debug level
	rofl_pipeline_set_logging_level(/*cn,*/(rofl_pipeline_debug_levels)c_level);
}

void system_manager::init_command_line_options(){

	std::stringstream composed_usage("");

	//Daemonize
	env_parser->add_option(coption(true, NO_ARGUMENT,'D',"daemonize","Daemonize execution",""));
	
	//Log file
	env_parser->add_option(coption(true,REQUIRED_ARGUMENT,'l',"logfile","Log file used when daemonization", XDPD_CLOG_FILE));
	
	//Extra driver parameters
	composed_usage << "Quoted string of extra parameters that will be passed to the platform driver \n\n";
	composed_usage << "\t\t\t       ["<<get_driver_code_name()<<"] supported extra parameters: "<<std::endl<< get_driver_usage()<<std::endl<<"\t\t\t\t";
	env_parser->add_option(coption(true,REQUIRED_ARGUMENT, 'e', XDPD_EXTRA_PARAMS_OPT_FULL_NAME, composed_usage.str(), ""));

	//Test
	env_parser->add_option(coption(true, NO_ARGUMENT, 't', XDPD_TEST_RUN_OPT_FULL_NAME, "Test configuration only and exit", ""));

	//Version
	env_parser->add_option(coption(true, NO_ARGUMENT, 'v', "version", "Retrieve xDPd version and exit", std::string("")));


	//Add plugin options
	std::vector<coption> plugin_options = plugin_manager::__get_plugin_options();
	
	//Add them 
	for(std::vector<coption>::iterator it = plugin_options.begin(); it != plugin_options.end(); ++it){
		env_parser->add_option(*it);
	}
}

void system_manager::init(int argc, char** argv){

	unsigned int debug_level;

	//Prevent double calls to init()
	if(inited)
		XDPD_ERR("[xdpd][system_manager] ERROR: double call to system_amanager::init(). This can only be caused by a spurious call from a misbehaving plugin. Please notify this error. Continuing execution...\n");

	//Set ciosrv thread
	ciosrv_thread = pthread_self();

	//Set driver info cache
	hal_driver_get_info(&driver_info);

	//Initialize srand
	srand(time(NULL));

	/* Parse arguments. Add first additional arguments */
	env_parser = new cunixenv(argc, argv);

	//Initialize command line options 
	init_command_line_options();
	
	//Parse arguments
	env_parser->parse_args();

	//If -v is set, print version and return immediately. Note that this must be here after
	//get_info 
	if(env_parser->is_arg_set("version")) {
		XDPD_INFO(get_version().c_str());
		goto SYSTEM_MANAGER_CLEANUP;
	}

	//Help
	if(env_parser->is_arg_set("help")) {
		dump_help();
		goto SYSTEM_MANAGER_CLEANUP;
	}
	
	//Set debugging
	debug_level = XDPD_DEFAULT_DEBUG_LEVEL;
	if(env_parser->is_arg_set("debug"))	
		debug_level = atoi(env_parser->get_arg("debug").c_str());
	set_logging_debug_level(debug_level);

	//Daemonize
	if(env_parser->is_arg_set("daemonize")) {
		xdpd::cdaemon::daemonize(XDPD_PID_FILE, XDPD_LOG_FILE);
		XDPD_INFO("[xdpd][system_manager] daemonizing successful\n");
	}else{
		//If not daemonized and logfile is set: redirects the output to logfile. TODO: maybe logfile shouldn't depend on daemonize.
		if(env_parser->is_arg_set("logfile")){
			try{
				int fd = open(env_parser->get_arg("logfile").c_str(), O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
				if (fd >=0){
					std::cout << " All OUTPUT will be redirected to the logfile: " << env_parser->get_arg("logfile").c_str() <<  "    pid: " << getpid()<< std::endl;
				}

				//Change the logfile mode, readable for all
				if (chmod(env_parser->get_arg("logfile").c_str(), S_IRWXU|S_IRGRP|S_IROTH)){
					std::cout << " Couldn't change the logfile mode." << std::endl;
				}

				//Redirect
				if (fd < 0) {
					throw eSysCall("open()");
				}
				if (dup2(fd, STDIN_FILENO) < 0) {
					throw eSysCall("dup2(): STDIN");
				}
				if (dup2(fd, STDOUT_FILENO) < 0) {
					throw eSysCall("dup2(): STDOUT");
				}
				if (dup2(fd, STDERR_FILENO) < 0) {
					throw eSysCall("dup2(): STDERR");
				}
			} catch (eSysCall& e) {
					std::cerr << "Log file problem: "<< e << std::endl;
			}
		}
	}

	//Capture control+C
	signal(SIGINT, interrupt_handler);
		
	if(is_test_run()) {
		XDPD_INFO("[xdpd][system_manager] Launched with -t %s. Doing a test-run execution", XDPD_TEST_RUN_OPT_FULL_NAME.c_str());
	}

	//Driver initialization
	if(hal_driver_init(&hal_extension_ops, __get_driver_extra_params().c_str()) != HAL_SUCCESS){
		XDPD_ERR("[xdpd][system_manager] ERROR: initialization of platform driver failed! Aborting...\n");
		exit(EXIT_FAILURE);
	}

	//Mark as initied. MUST BE here, after the first setting of the logging level has been
	//done
	inited = true;
	
	//Load plugins
	optind=0;
	plugin_manager::init();

	//If test-config is not set, launch ciosrv loop, otherwise terminate execution
	if(!is_test_run()){
		//run main loop. Only will stop in Ctrl+C
		pselect(0, NULL, NULL, NULL, NULL, NULL);
	}

	//Printing nice trace
	XDPD_INFO("\n[xdpd][system_manager] Shutting down...\n");

	//Destroy all state
	switch_manager::destroy_all_switches();

	//Call driver to shutdown
	hal_driver_destroy();
	
	//Let plugin manager destroy all registered plugins
	//This must be after calling hal_driver_destroy()
	plugin_manager::destroy();
	
	//Logging	
	logging::close();

	//Print a nice trace
	XDPD_INFO("[xdpd][system_manager] xdpd is now ...down.\n");

SYSTEM_MANAGER_CLEANUP:

	//Release cunixenv
	delete env_parser;	
}




//Dumps help
void system_manager::dump_help(){
	std::string xdpd_name="xdpd";
	std::stringstream plugin_list("");	
	
	//Compose plugin list
	std::vector<plugin*> plugins = plugin_manager::get_plugins();

	//Destroy all plugins
	for(std::vector<plugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
		plugin_list<<(*it)->get_name();
		if(it+1 != plugins.end())
			plugin_list<<", ";
	}

	//Usage first
	XDPD_INFO("\n%s\n", env_parser->get_usage((char*)xdpd_name.c_str()).c_str());
	//Other information
	XDPD_INFO("Compiled with plugins: %s\n", plugin_list.str().c_str());
	XDPD_INFO("Compiled with hardware support for: %s\n", get_driver_code_name().c_str());
	if(get_driver_description()!="")
		XDPD_INFO("Hardware driver description: %s\n\n", get_driver_description().c_str());
	else
		XDPD_INFO("\n");

	if(get_driver_extra_params()!="")
		XDPD_INFO("%s\n", get_driver_extra_params().c_str());


}


//Generates the version string
std::string system_manager::get_version(){

	std::stringstream ss("");

	//xDPd CMM information
	ss << std::endl << "The eXtensible OpenFlow Datapath daemon (xDPd)" << std::endl;	
	ss << "Version: "<< XDPD_VERSION << std::endl;

#ifdef XDPD_BUILD
	ss << "Build: " << XDPD_BUILD << std::endl;
	ss << "Compiled in branch: " << XDPD_BRANCH << std::endl;
	ss << "Detailed build information:" << XDPD_DESCRIBE << std::endl;
#endif

	//xDPd driver information
	ss << "\n-- Hardware support --" << std::endl;
	ss << "Driver code name: "<< driver_info.code_name << std::endl;
	ss << "Driver version: "<< driver_info.version << std::endl;
	ss << "Driver description: "<< driver_info.description << std::endl;

	//Libraries info
	ss << std::endl << "-- Libraries --" << std::endl;
	ss << "[ROFL-common]" << std::endl;
	ss << "  Version: " << ROFL_COMMON_VERSION << std::endl;
	ss << "  Build: " << ROFL_COMMON_BUILD << std::endl;
	ss << "  Compiled in branch: " << ROFL_COMMON_BRANCH << std::endl;
	ss << "  Detailed build information:" << ROFL_COMMON_DESCRIBE << std::endl << std::endl;
	ss << "[ROFL-datapath]" << std::endl;
	ss << "  Version: " << ROFL_DATAPATH_VERSION << std::endl;
	ss << "  Build: " << ROFL_DATAPATH_BUILD << std::endl;
	ss << "  Compiled in branch: " << ROFL_DATAPATH_BRANCH << std::endl;
	ss << "  Detailed build information:" << ROFL_DATAPATH_DESCRIBE << std::endl << std::endl;

	return ss.str();
}

//
// Options
//

bool system_manager::is_option_set(const std::string& option_name){
	return env_parser->is_arg_set(option_name);	
}

std::string system_manager::get_option_value(const std::string& option_name){
	return env_parser->get_arg(option_name);
}

