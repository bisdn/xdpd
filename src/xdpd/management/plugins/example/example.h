#ifndef EXAMPLE_PLUGIN_H
#define EXAMPLE_PLUGIN_H 

#include <rofl/common/ciosrv.h>
#include "../../plugin_manager.h"

/**
* @file example.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Simple example of a plugin. Prints some traces on port notification and monitoring events
* Upon reception of SIGUSR1 dumps the state in the console (cout)
* 
*/

namespace xdpd {

/**
* @brief Dummy management plugin example
* @ingroup cmm_mgmt_plugins
*/
class example:
	public plugin,
	public rofl::ciosrv /* Only for timers => print traces on SIGUSR1 */
	
 {
	
public:
	virtual void init(void);

	virtual std::string get_name(void){
		//Use code-name
		return std::string("example");
	};

	virtual void notify_port_added(const switch_port_snapshot_t* port_snapshot);
	virtual void notify_port_attached(const switch_port_snapshot_t* port_snapshot);
	virtual void notify_port_status_changed(const switch_port_snapshot_t* port_snapshot);
	virtual void notify_port_detached(const switch_port_snapshot_t* port_snapshot);
	virtual void notify_port_deleted(const switch_port_snapshot_t* port_snapshot);
	
	virtual void notify_monitoring_state_changed(const monitoring_snapshot_state_t* monitoring_snapshot);

	static bool dump;
private:
	virtual void handle_timeout(int opaque, void *data);
	

};

}// namespace xdpd 

#endif /* EXAMPLE_PLUGIN_H_ */


