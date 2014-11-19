#include "openflow13_switch.h"

#include <rofl/datapath/hal/openflow/openflow1x/of1x_driver.h>
#include <rofl/datapath/hal/openflow/openflow1x/of1x_cmm.h>

using namespace xdpd;

/*
* Constructor and destructor for the openflow 1.3 switch
*/
openflow13_switch::openflow13_switch(uint64_t dpid,
				std::string const& dpname,
				unsigned int num_of_tables,
				int* ma_list,
				int reconnect_start_timeout,
				enum rofl::csocket::socket_type_t socket_type,
				cparams const& socket_params) throw (eOfSmVersionNotSupported)
		: openflow_switch(dpid, dpname, OF_VERSION_13, num_of_tables)
{

	if (hal_driver_create_switch((char*)dpname.c_str(),
					     dpid, OF_VERSION_13, num_of_tables, ma_list) != HAL_SUCCESS){
		//WRITELOG(CDATAPATH, ERROR, "of13_endpoint::of13_endpoint() "
		//		"failed to allocate switch instance in HAL, aborting");

		throw eOfSmErrorOnCreation();
	}

	//Initialize the endpoint, and launch control channel
	endpoint = new of13_endpoint(this, reconnect_start_timeout, socket_type, socket_params);

}


openflow13_switch::~openflow13_switch(){
		
	//Destroy listening sockets and ofctl instances
	endpoint->rpc_close_all();		

	//Now safely destroy the endpoint
	delete endpoint;	

	//Destroy forwarding plane state
	hal_driver_destroy_switch_by_dpid(dpid);
}

/* Public interfaces for receving async messages from the driver */
rofl_result_t openflow13_switch::process_packet_in(uint8_t table_id,
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint64_t cookie,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					packet_matches_t* matches){
	
	return ((of13_endpoint*)endpoint)->process_packet_in(table_id,
					reason,
					in_port,
					buffer_id,
					cookie,
					pkt_buffer,
					buf_len,
					total_len,
					matches);
}

rofl_result_t openflow13_switch::process_flow_removed(uint8_t reason, of1x_flow_entry_t* removed_flow_entry){
	return ((of13_endpoint*)endpoint)->process_flow_removed(reason, removed_flow_entry);
}

