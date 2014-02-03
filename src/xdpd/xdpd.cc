/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <rofl/platform/unix/cunixenv.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/platform/unix/cdaemon.h>
#include "management/switch_manager.h"
#include "management/port_manager.h"
#include "management/plugin_manager.h"

using namespace rofl;
using namespace xdpd;

extern int optind;

//TODO: Redirect C loggers to the output log
#define XDPD_CLOG_FILE "./xdpd.log"
#define XDPD_LOG_FILE "/var/log/xdpd.log"
#define XDPD_PID_FILE "/var/run/xdpd.pid"

//Handler to stop ciosrv
void interrupt_handler(int dummy=0) {
	//Only stop ciosrv 
	ciosrv::stop();
}

//Prints version and build numbers and exits
void dump_version(){
	//Print version and exit
	ROFL_INFO("The eXtensible OpenFlow Datapath daemon (xDPd)\n");	
	ROFL_INFO("Version: %s\n",XDPD_VERSION);

#ifdef XDPD_BUILD
	ROFL_INFO("Build: %s\n",XDPD_BUILD);
	ROFL_INFO("Compiled in branch: %s\n",XDPD_BRANCH);
	ROFL_INFO("%s\n",XDPD_DESCRIBE);
#endif	
	ROFL_INFO("\n[Libraries: ROFL]\n");
	ROFL_INFO("ROFL version: %s\n",ROFL_VERSION);
	ROFL_INFO("ROFL build: %s\n",ROFL_BUILD_NUM);
	ROFL_INFO("ROFL compiled in branch: %s\n",ROFL_BUILD_BRANCH);
	ROFL_INFO("%s\n",ROFL_BUILD_DESCRIBE);
	
	exit(EXIT_SUCCESS);

}

/*
 * xDPd Main routine
 */
int main(int argc, char** argv){


	//Check for root privileges 
	if(geteuid() != 0){
		ROFL_ERR("ERROR: Root permissions are required to run %s\n",argv[0]);	
		exit(EXIT_FAILURE);	
	}

#if DEBUG && VERBOSE_DEBUG
	//Set verbose debug if necessary
	rofl_set_logging_level(/*cn,*/ DBG_VERBOSE_LEVEL);
#endif
	
	/* Add additional arguments */
	char s_dbg[32];
	memset(s_dbg, 0, sizeof(s_dbg));
	snprintf(s_dbg, sizeof(s_dbg)-1, "%d", (int)csyslog::DBG);

	{ //Make valgrind happy
		cunixenv env_parser(argc, argv);
		
		/* update defaults */
		env_parser.update_default_option("logfile", XDPD_CLOG_FILE);
		env_parser.add_option(coption(true, NO_ARGUMENT, 'v', "version", "Retrieve xDPd version and exit", std::string("")));

		//Parse
		env_parser.parse_args();

		if (env_parser.is_arg_set("version")) {
			dump_version();
		}
		
		if (not env_parser.is_arg_set("daemonize")) {
			// only do this in non
			std::string ident(XDPD_CLOG_FILE);

			csyslog::initlog(csyslog::LOGTYPE_FILE,
					static_cast<csyslog::DebugLevel>(atoi(env_parser.get_arg("debug").c_str())), // todo needs checking
					env_parser.get_arg("logfile"),
					ident.c_str());

			logging::set_debug_level(atoi(env_parser.get_arg("debug").c_str()));
		}

		//Daemonize
		if (env_parser.is_arg_set("daemonize")) {
			rofl::cdaemon::daemonize(XDPD_PID_FILE, XDPD_LOG_FILE);
			rofl::logging::set_debug_level(atoi(env_parser.get_arg("debug").c_str()));
			rofl::logging::notice << "[xdpd][main] daemonizing successful" << std::endl;
		}

		//Capture control+C
		signal(SIGINT, interrupt_handler);
	}

	//Forwarding module initialization
	if(fwd_module_init() != AFA_SUCCESS){
		ROFL_INFO("Init driver failed\n");	
		exit(-1);
	}

	//Load plugins
	optind=0;
	plugin_manager::init(argc, argv);

	//ciosrv run. Only will stop in Ctrl+C
	ciosrv::run();

	//Printing nice trace
	ROFL_INFO("\nCleaning the house...\n");	

	//Destroy all state
	switch_manager::destroy_all_switches();

	//Let plugin manager destroy all registered plugins
	plugin_manager::destroy();
	
	//Call fwd_module to shutdown
	fwd_module_destroy();
	
	ROFL_INFO("House cleaned!\nGoodbye\n");
	
	csyslog::closelog();

	logging::close();

	rofl::cioloop::shutdown();

	exit(EXIT_SUCCESS);
}




