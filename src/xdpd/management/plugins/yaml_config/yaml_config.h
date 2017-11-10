/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDPD_PLUGINS_YAML_CONFIG_PLUGIN_H
#define XDPD_PLUGINS_YAML_CONFIG_PLUGIN_H

#include <iostream>
#include <yaml-cpp/yaml.h>
#include "xdpd/common/cunixenv.h"
#include "xdpd/common/exception.h"
#include "../../plugin_manager.h"
#include "scope.h"

//get params
#include "system/system_scope.h"

/**
* @file yaml_plugin.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* based on the "config" plugin by Marc Sune
*
* @brief yaml-cpp based configuration plugin file
*/

namespace xdpd {
namespace plugins {
namespace yaml_config {

//Macro for C logging
#define YAML_PLUGIN_ID "[xdpd][plugins][yaml_config] "

/**
* @brief libyaml based yamluration plugin
* @ingroup cmm_mgmt_plugins
*/
class yaml_config : public plugin {
	
public:
	yaml_config();
	virtual ~yaml_config();

	
	virtual void init(void);

	virtual std::vector<xdpd::coption> get_options(void){
		std::vector<xdpd::coption> vec;

		//Add mandatory -y argument
		vec.push_back(xdpd::coption(false,REQUIRED_ARGUMENT, YAML_FILE_OPT_CODE,  YAML_FILE_OPT_FULL_NAME, "xdpd config file","./xdpd.yaml"));

		return vec;
	};

	virtual std::string get_driver_extra_params(void){
		try {
			return system_scope::get_driver_extra_params(reload_yaml_file());
		} catch (eYamlConfParamNotFound& e) {
			// let other plugins continue to work
			return std::string("");
		}
	}

	virtual std::string get_name(void){
		return std::string("yaml_config");
	};

private:

	/**
	 * @brief	Reloads the yaml configuration file and updates this->cfg.
	 */
	YAML::Node& reload_yaml_file();

private:

	YAML::Node cfg;
	static const std::string YAML_FILE_OPT_FULL_NAME;
	static const char YAML_FILE_OPT_CODE = 'y';
};

}// namespace yaml_config
}// namespace plugins
}// namespace xdpd 

#endif /* XDPD_PLUGINS_YAML_CONFIG_PLUGIN_H */



