#ifndef RUN_PEX_PLUGIN_H
#define RUN_PEX_PLUGIN_H 

#include "../../plugin_manager.h"

/**
* @file run_pex.h
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief Plugin to crate and start PEX
* 
*/

namespace xdpd {

/**
* @brief Dummy management plugin example
* @ingroup cmm_mgmt_plugins
*/
class runPEX:public plugin {
	
public:
	virtual void init(void);

	virtual std::string get_name(void){
		//Use code-name
		return std::string("RunPEX");
	};

/*	virtual void notify_port_added(const switch_port_snapshot_t* port_snapshot);
	virtual void notify_port_attached(const switch_port_snapshot_t* port_snapshot);
	virtual void notify_port_status_changed(const switch_port_snapshot_t* port_snapshot);
	virtual void notify_port_detached(const switch_port_snapshot_t* port_snapshot);
	virtual void notify_port_deleted(const switch_port_snapshot_t* port_snapshot);
	
	virtual void notify_monitoring_state_changed(const monitoring_snapshot_state_t* monitoring_snapshot);
*/

};

}// namespace xdpd 

#endif /* RUN_PEX_PLUGIN_H_ */


