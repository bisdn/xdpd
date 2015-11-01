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
void openflow_switch::rpc_connect_to_ctl(enum xdpd::csocket::socket_type_t socket_type, xdpd::cparams const& socket_params){
	rofl::openflow::cofhello_elem_versionbitmap versionbitmap;
	versionbitmap.add_ofp_version(version);

	// TODO: map socket_type and socket_params to required parameters

	rofl::sockaddr raddr;
	endpoint->add_ctl(0).add_conn(rofl::cauxid(0)).set_raddr(raddr).tcp_connect(versionbitmap, rofl::crofconn::MODE_DATAPATH);

	// TODO: TLS

	//endpoint->add_ctl(endpoint->get_idle_ctlid(), versionbitmap).connect(rofl::cauxid(0), socket_type, socket_params);
}

void openflow_switch::rpc_disconnect_from_ctl(enum xdpd::csocket::socket_type_t socket_type, xdpd::cparams const& socket_params){
	//endpoint->rpc_disconnect_from_ctl(socket_type, socket_params);
}

