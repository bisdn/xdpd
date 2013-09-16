/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OPENFLOW_SWITCH_H
#define OPENFLOW_SWITCH_H 

#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/platform/memory.h>

#include "of_endpoint.h"

/**
* @file openflow_switch.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief openflow_switch is a version-agnostic abstraction of a complete
* (logical) Openflow switch (complete datapath as the spec defines).
* 
*/

namespace xdpd {

class openflow_switch {

protected:

	//Driver context (this is somehow superfluous)
	of_switch_t* ofswitch;
	
	//Endpoint context
	of_endpoint* endpoint;

	openflow_switch(const uint64_t dpid, const std::string &dpname, const of_version_t version);

public:
	//General elements
	uint64_t dpid;
	std::string dpname;
	of_version_t version;

	/**
	 * Destructor
	 */
	virtual ~openflow_switch(void){};

	/**
	* Getter for the switch
	*/
	of_switch_t* get_fwd_module_sw_ref(void){return ofswitch;};

	/**
	*
	* Dispatching of version agnostic messages comming from the driver
	*
	*/
	virtual afa_result_t notify_port_add(switch_port_t* port)=0;
	
	virtual afa_result_t notify_port_delete(switch_port_t* port)=0;
	
	virtual afa_result_t notify_port_status_changed(switch_port_t* port)=0;

};

}// namespace rofl

#endif /* OPENFLOW_SWITCH_H_ */
