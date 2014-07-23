/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "openflow_switch.h"

using namespace rofl;
using namespace xdpd;

openflow_switch::openflow_switch(const uint64_t dpid, const std::string &dpname, const of_version_t version, unsigned int num_of_tables) :
		endpoint(NULL),
		dpid(dpid),
		dpname(dpname),
		version(version),
		num_of_tables(num_of_tables)
{

}

/*
* Port notfications. Process them directly in the endpoint
*/
rofl_result_t openflow_switch::notify_port_attached(const switch_port_t* port){
	return endpoint->notify_port_attached(port);
}
rofl_result_t openflow_switch::notify_port_detached(const switch_port_t* port){
	return endpoint->notify_port_detached(port);
}
rofl_result_t openflow_switch::notify_port_status_changed(const switch_port_t* port){
	return endpoint->notify_port_status_changed(port);
}


/*
* Connecting and disconnecting from a controller entity
*/
void openflow_switch::rpc_connect_to_ctl(enum rofl::csocket::socket_type_t socket_type, cparams const& socket_params){
	rofl::openflow::cofhello_elem_versionbitmap versionbitmap;
	versionbitmap.add_ofp_version(version);
	endpoint->rpc_connect_to_ctl(versionbitmap, socket_type, socket_params);
}

void openflow_switch::rpc_disconnect_from_ctl(enum rofl::csocket::socket_type_t socket_type, cparams const& socket_params){
	//endpoint->rpc_disconnect_from_ctl(socket_type, socket_params);
}

