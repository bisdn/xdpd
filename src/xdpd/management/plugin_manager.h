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
* @brief Plugin management API file.
*/

namespace xdpd {

/**
* @brief Abstract class of a management plugin. 
*
* The concept of a management plugin in xdpd is NOT a "hot" pluggable module as usually
* refered to as "plugin", but a module that can be loaded at compile time (compiled, indeed).
* @ingroup cmm_mgmt
*/
class plugin {
	
public:
	/**
	* @brief Initializes the plugin
	*
	* @description The init method must be overwritten by the
	* derived plugin and must contain the initialization
	* of the plugin itself. The method shall return immediately
	* after bootstrapping of the plugin. If the plugin needs to continue
	* the execution during the lifetime of xdpd, a thread must be spawned 
	* within this method. Please consider using ciosrv APIs to avoid wasting
	* of pthreads.
	*
	* argc and argc are xdpd's command-line parameters. The plugin can, at its wish
	* define its own arguments and parse them, usually using optargs or cunixenv
	* APIs. Plugin Manager will reset argc for each plugin.
	*
	* It is up t the plugins not to clash with other plugins arguments.
	*
	* @param argc Number of arguments that xdpd received
	* @param argv argv's of xdpd.
	*/
	virtual void init(int argc, char** argv)=0;
	
	/**
	* Returns the plugin name
	*/
	virtual std::string get_name(void)=0;

	virtual ~plugin(){}; 
};


/**
 * @brief The plugin manager orchestrates the initialization of the plugins at bootstrap time 
* @ingroup cmm_mgmt
 */
class plugin_manager {

public:
	/**
	* Initializes all the compiled plugins
	*/
	static rofl_result init(int args, char** argv);
	
	/**
	* Destroys registered plugins 
	*/
	static rofl_result destroy(void);
	

	/*
	* Registers a plugin must be called on pre_init()
	*/
	static void register_plugin(plugin* p);

private:
	/**
	* Pre-init hook where plugins can and must be added (registered)
	*/
	static void pre_init(void);

	/**
	* Registered plugins vector
	*/	
	static std::vector<plugin*> plugins;
};


}// namespace xdpd
#endif /* PLUGIN_MANAGER_H_ */
