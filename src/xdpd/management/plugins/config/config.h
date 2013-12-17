/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONFIG_PLUGIN_H
#define CONFIG_PLUGIN_H 

#include <iostream>
#include <libconfig.h++> 
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/cerror.h>
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
		
	
	virtual void init(int args, char** argv);
	
	virtual std::string get_name(void){
		return std::string("config");
	};

private:
	void parse_config(libconfig::Config* cfg, rofl::cunixenv& env_parser);
};

}// namespace xdpd 

#endif /* CONFIG_PLUGIN_H_ */


