/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H 

#include <string> 
#include <vector> 
#include <rofl.h>

/**
* @file plugin_manager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Plugin manager 
* 
*/

namespace xdpd {

/*
* Abstract class of a plugin
*/
class plugin {
	
public:
	virtual void init(int argc, char** argv)=0;
	
	virtual std::string get_name(void)=0;
	virtual ~plugin(){}; 
};


/*
 * Plugin manager manages the initialization of the plugins at bootstrapping time 
 */
class plugin_manager {

public:
	static rofl_result init(int args, char** argv);
	static void register_plugin(plugin* p);

private:
	static void pre_init(void);
	static std::vector<plugin*> plugins;
};


}// namespace rofl

#endif /* PLUGIN_MANAGER_H_ */
