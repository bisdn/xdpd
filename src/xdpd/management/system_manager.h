/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H 

#include <list>
#include <stdint.h>
#include <rofl.h>
#include <rofl/common/croflexception.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/platform/unix/cunixenv.h>

/**
* @file system_manager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief System management API file.
*/

namespace xdpd {


//System manager exceptions
class eSystemBase		: public rofl::RoflException {};	// base error class for all system_manager related errors
class eSystemGeneralError	: public eSystemBase {};
class eSystemUnknownError	: public eSystemBase {};
class eSystemLogLevelSetviaCL	: public eSystemBase {};
class eSystemLogInvalidLevel	: public eSystemBase {};

/**
* @brief System management API.
*
* The system manager API is a C++ interface that can be consumed
* by the add-on management modules for general system operations.
* @ingroup cmm_mgmt
*/
class system_manager {

public:

	/**
	* Initialize system's managment API facility
	*/
	static void init(int argc, char** argv);

	//
	// General information
	//
	
	/**
	* Get xDPd version number, build number and other information
	*/
	static std::string get_version(void);

	/**
	* Returns true if is a test run (no real execution only validation)
	*/
	static bool is_test_run(void){
		return is_option_set((std::string&)XDPD_TEST_RUN_OPT_FULL_NAME);	
	};

	//
	// xDPd arguments
	//
	
	/**
	* Returns true/false if a certain command line option_name 's value (if present).
	*/
	static bool is_option_set(const std::string& option_name);
	/**
	* Get command line option_name's value (if present).
	*/
	static std::string get_option_value(const std::string& option_name);

	//
	// xDPd logging
	//

	/**
	* Set the logging debug level for xDPd. Note that when -d is used in the CLI
	* The call to set_logging_debug_level() will throw eSystemLogLevelSetviaCL exception
	* since CLI options have always preference over runtime API.
	*
	* Use logging::EMERG, logging::ALERT ... defined in rofl/common/logging.h
	*/
	static void set_logging_debug_level(unsigned int level);	

	//
	// Platform driver  
	//
	
	/**
	* Retrieve the forwarding module code-name (e.g. gnu-linux).
	*
	* This method will always a valid code-name
	*/
	static std::string get_fwd_module_code_name(void){
		return std::string(info.code_name);
	}	
	
	/**
	* Retrieve the forwarding module usage(help)
	*/
	static std::string get_fwd_module_usage(void){
		return std::string(info.usage);
	}	
		
	/**
	* Retrieve the forwarding module version and build number.
	*/
	static std::string get_fwd_module_version(void){
		return std::string(info.version);
	}	

	/**
	* Retrieve the forwarding module description 
	*/
	static std::string get_fwd_module_description(void){
		return std::string(info.description);
	}	
	
	/**
	* Retrieve the forwarding module extra parameters that has
	* been instantiated with. 
	*
	* @warning Result will return only the parameters recognised
	* by the forwarding module, which can be different from the 
	* ones sent. Also note that the syntax of this parameters is
	* deliverately fwd_module specific.
	*/
	static std::string get_fwd_module_extra_params(void){
		return std::string(info.extra_params);
	}	

private:

	//Prevent double initializations
	static bool inited;	

	//Cache platform driver information
	static fwd_module_info_t info;

	//Command line options
	static rofl::cunixenv* env_parser;

	//Constants
	static const std::string XDPD_CLOG_FILE;
	static const std::string XDPD_LOG_FILE;
	static const std::string XDPD_PID_FILE;
	static const unsigned int XDPD_DEFAULT_DEBUG_LEVEL;

	static const std::string XDPD_TEST_RUN_OPT_FULL_NAME;
	static const std::string XDPD_EXTRA_PARAMS_OPT_FULL_NAME;

	//Other helper internal functions
	static void init_command_line_options(void);
	static std::string get_driver_extra_params(void); 
	static void dump_help(void); 
};

}// namespace xdpd 

#endif /* SYSTEM_MANAGER_H_ */
