/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H 

#include <string> 
#include <vector> 
#include <rofl.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <rofl/datapath/pipeline/monitoring.h>

/**
* @file plugin_manager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Plugin management API file.
*/

namespace xdpd {

//fwd decl
class plugin_manager;

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
	* Returns the plugin name
	*/
	virtual std::string get_name(void)=0;

	//
	//Event notification callback sub-API
	//

	/**
	* Callback to receive 'new port in the system' events. If the port appears as attached to an LSI, this implicitly means that the port is new and that has been already attached to the specified LSI. 
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	* @warning: This call MUST BE non-blocking 
	*/
	virtual void notify_port_added(const switch_port_snapshot_t* port_snapshot){};
		
	/**
	* Callback to receive 'port attached an LSI' events
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	* @warning: This call MUST BE non-blocking 
	*/
	virtual void notify_port_attached(const switch_port_snapshot_t* port_snapshot){};
		
	/**
	* Callback to receive 'change in the state of a system port' events
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	* @warning: This call MUST BE non-blocking 
	*/
	virtual void notify_port_status_changed(const switch_port_snapshot_t* port_snapshot){};	
		
	/**
	* Callback to receive 'port detached in the system' events   
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	* @warning: This call MUST BE non-blocking 
	*/
	virtual void notify_port_detached(const switch_port_snapshot_t* port_snapshot){};
	
	/**
	* Callback to receive 'port deletion in the system' events. If the port appears as attached to an LSI, this implicitly means that the port has been deleted and that has been already detached from the specified LSI.  
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	* @warning: This call MUST BE non-blocking 
	*/
	virtual void notify_port_deleted(const switch_port_snapshot_t* port_snapshot){};
	
	/**
	* Callback for 'monitoring state changed' events
	* @warning: monitoring_snapshot MUST NOT be written or destroyed, use monitoring_clone_snapshot() in case of need.
	* @warning: This call MUST BE non-blocking 
	*/
	virtual void notify_monitoring_state_changed(const monitoring_snapshot_state_t* monitoring_snapshot){};

protected:
	friend class plugin_manager; //Let plugin_manager call plugin's init and destroy 
	
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
	
	/**
	* Get the list of plugins 
	*/
	static std::vector<plugin*> get_plugins(void){ 
		return plugins; 
	}

	/**
	* Get the list of plugins 
	*/
	static plugin* get_plugin_by_name(std::string name);

	/**
	* Registers a plugin must be called on pre_init()
	*/
	static void register_plugin(plugin* p);

	//Plugin notification sub-API
	
	/**
	* Notify plugins of a new port on the system
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	*/
	static void __notify_port_added(const switch_port_snapshot_t* port_snapshot);	
		
	/**
	* Notify plugins of an attachment of a port to a LSI
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	*/
	static void __notify_port_attached(const switch_port_snapshot_t* port_snapshot);	
		
	/**
	* Notify plugins of a change in the state of a system port 
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	*/
	static void __notify_port_status_changed(const switch_port_snapshot_t* port_snapshot);	
		
	/**
	* Notify plugins of a detachment of a port from an LSI
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	*/
	static void __notify_port_detached(const switch_port_snapshot_t* port_snapshot);	
		
	/**
	* Notify plugins of the deletion of a port of the system
	* @warning: port_snapshot MUST NOT be written or destroyed, use switch_port_clone_snapshot() in case of need.
	*/
	static void __notify_port_deleted(const switch_port_snapshot_t* port_snapshot);	
	
	/**
	* Notify plugins of a changed in the monitoring state of the platform
	* @warning: monitoring_snapshot MUST NOT be written or destroyed, use monitoring_clone_snapshot() in case of need.
	*/
	static void __notify_monitoring_state_changed(const monitoring_snapshot_state_t* monitoring_snapshot);

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
