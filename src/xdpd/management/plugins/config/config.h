/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONFIG_PLUGIN_H
#define CONFIG_PLUGIN_H 

#include <iostream>
#include <libconfig.h++> 
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/croflexception.h>
#include "../../plugin_manager.h"
#include "scope.h"

/**
* @file config_plugin.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief libconfig based configuration plugin file
*/

namespace xdpd {

/**
* @brief libconfig based configuration plugin
* @ingroup cmm_mgmt_plugins
*/
class config : public plugin {
	
public:
	config();
	virtual ~config();
		
	
	virtual void init(void);
	
	virtual std::vector<rofl::coption> get_options(void){
		std::vector<rofl::coption> vec;

		//Add mandatory -c argument
		vec.push_back(rofl::coption(false,REQUIRED_ARGUMENT, CONFIG_FILE_OPT_CODE,  CONFIG_FILE_OPT_FULL_NAME, "xDPd config file","./example.cfg"));
	
		return vec;
	};
	
	virtual std::string get_driver_extra_params(void){
		//XXX: properly read it 
		return std::string("");
	}
	
	virtual std::string get_name(void){
		return std::string("config");
	};

private:
	void get_config_file_contents(libconfig::Config* cfg);
	static const std::string CONFIG_FILE_OPT_FULL_NAME;
	static const char CONFIG_FILE_OPT_CODE = 'c';
};

}// namespace xdpd 

#endif /* CONFIG_PLUGIN_H_ */


