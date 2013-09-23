/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <rofl/platform/unix/cunixenv.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include "management/switch_manager.h"
#include "management/port_manager.h"
#include "management/plugin_manager.h"

using namespace rofl;
using namespace xdpd;

#define XDPD_LOG_FILE "xdpd.log"

//TODO: Redirect C loggers to the output log

//Handler to stop ciosrv
void interrupt_handler(int dummy=0) {
	//Only stop ciosrv 
	ciosrv::stop();
}

/**
 * XDPD Main routine
 */
int main(int argc, char** argv){

	//Check for root privileges 
	if(geteuid() != 0){
		ROFL_ERR("ERROR: Root permissions are required to run %s\n",argv[0]);	
		exit(EXIT_FAILURE);	
	}

	//Capture control+C
	signal(SIGINT, interrupt_handler);

#if DEBUG && VERBOSE_DEBUG
	//Set verbose debug if necessary
	rofl_set_logging_level(/*cn,*/ DBG_VERBOSE_LEVEL);
#endif
	
	/* Add additional arguments */
	char s_dbg[32];
	memset(s_dbg, 0, sizeof(s_dbg));
	snprintf(s_dbg, sizeof(s_dbg)-1, "%d", (int)csyslog::DBG);

	/* update defaults */
	cunixenv::getInstance().update_default_option("logfile", XDPD_LOG_FILE);

	//Parse
	cunixenv::getInstance().parse_args(argc, argv);

	if (not cunixenv::getInstance().is_arg_set("daemonize")) {
		// only do this in non
		std::string ident(XDPD_LOG_FILE);

		csyslog::initlog(csyslog::LOGTYPE_FILE,
				static_cast<csyslog::DebugLevel>(atoi(cunixenv::getInstance().get_arg("debug").c_str())), // todo needs checking
				cunixenv::getInstance().get_arg("logfile"),
				ident.c_str());
	}
	
	//Forwarding module initialization
	if(fwd_module_init() != AFA_SUCCESS){
		ROFL_INFO("Init driver failed\n");	
		exit(-1);
	}

	//Init the ciosrv.
	ciosrv::init();

	//Load plugins
	plugin_manager::init(argc, argv);

	//ciosrv run. Only will stop in Ctrl+C
	ciosrv::run();

	//Printing nice trace
	ROFL_INFO("\nCleaning the house...\n");	

	//Destroy all state
	switch_manager::destroy_all_switches();

	//ciosrv destroy
	ciosrv::destroy();

	//Call fwd_module to shutdown
	fwd_module_destroy();
	
	ROFL_INFO("House cleaned!\nGoodbye\n");
	
	exit(EXIT_SUCCESS);
}




