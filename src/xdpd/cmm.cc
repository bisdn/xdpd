#include "management/switch_manager.h"
#include "management/port_manager.h"
#include "openflow/openflow_switch.h"
#include <rofl/datapath/afa/cmm.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>
#include <rofl/datapath/pipeline/physical_switch.h>

using namespace xdpd;

/*
* Dispatching of platform related messages comming from the fwd_module 
*/

afa_result_t cmm_notify_port_add(switch_port_snapshot_t* port_snapshot){
	
	afa_result_t result=AFA_SUCCESS;
	
	if(!port_snapshot)
		return AFA_FAILURE;
	
	//Notify MGMT framework
	//TODO:

	//Notify attached sw
	if(port_snapshot->is_attached_to_sw)
		result = switch_manager::__notify_port_add(port_snapshot);
	
	//Destroy the snapshot
	switch_port_destroy_snapshot(port_snapshot);
		
	return result;
}

afa_result_t cmm_notify_port_delete(switch_port_snapshot_t* port_snapshot){
	
	afa_result_t result = AFA_SUCCESS;
	
	if (!port_snapshot)
		return AFA_FAILURE;

	//Notify MGMT framework
	//TODO:

	//Notify attached sw
	if(port_snapshot->is_attached_to_sw)
		result = switch_manager::__notify_port_delete(port_snapshot);

	//Destroy the snapshot
	switch_port_destroy_snapshot(port_snapshot);
	
	return result;
}

afa_result_t cmm_notify_port_status_changed(switch_port_snapshot_t* port_snapshot){
	
	afa_result_t result = AFA_SUCCESS;
	
	if (!port_snapshot)
		return AFA_FAILURE;

	//Notify MGMT framework
	//TODO:

	//Notify attached sw
	if(port_snapshot->is_attached_to_sw)
		result = switch_manager::__notify_port_status_changed(port_snapshot);

	//Destroy the snapshot
	switch_port_destroy_snapshot(port_snapshot);

	return result;
}

/*
* Driver CMM Openflow calls. Demultiplexing to the appropiate openflow_switch instance.
*/ 
afa_result_t cmm_process_of1x_packet_in(uint64_t dpid,
					uint8_t table_id,
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					packet_matches_t* matches){
	return switch_manager::__process_of1x_packet_in(dpid, table_id, reason, in_port, buffer_id, pkt_buffer, buf_len, total_len, matches);	
}

afa_result_t cmm_process_of1x_flow_removed(uint64_t dpid, uint8_t reason, of1x_flow_entry_t* removed_flow_entry){
	return switch_manager::__process_of1x_flow_removed(dpid, reason, removed_flow_entry);
}
