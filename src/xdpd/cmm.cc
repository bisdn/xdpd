#include <rofl/datapath/afa/cmm.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>
#include <rofl/datapath/pipeline/physical_switch.h>

#include "management/switch_manager.h"
#include "management/port_manager.h"
#include "management/plugin_manager.h"

using namespace xdpd;

/*
* Dispatching of platform related messages comming from the fwd_module 
*/

afa_result_t cmm_notify_port_add(switch_port_snapshot_t* port_snapshot){
	
	afa_result_t result=AFA_SUCCESS;
	
	if(!port_snapshot)
		return AFA_FAILURE;

	//Notify port manager
	port_manager::notify_port_add(port_snapshot);

	//Notify attached sw
	if(port_snapshot->is_attached_to_sw)
		//Note that this typecast is valid because afa_result_t and rofl_result_t have intentionally and explicitely the same definition
		result = (afa_result_t)switch_manager::__notify_port_attachment((const switch_port_snapshot_t*)port_snapshot);
	
	//Notify MGMT framework
	plugin_manager::notify_port_add((const switch_port_snapshot_t*)port_snapshot);	

	//Destroy the snapshot
	switch_port_destroy_snapshot(port_snapshot);
		
	return result;
}

afa_result_t cmm_notify_port_delete(switch_port_snapshot_t* port_snapshot){
	
	afa_result_t result = AFA_SUCCESS;
	
	if (!port_snapshot)
		return AFA_FAILURE;

	//Notify port manager
	port_manager::notify_port_delete(port_snapshot);

	//Notify attached sw
	if(port_snapshot->is_attached_to_sw)
		//Note that this typecast is valid because afa_result_t and rofl_result_t have intentionally and explicitely the same definition
		result = (afa_result_t)switch_manager::__notify_port_detachment((const switch_port_snapshot_t*)port_snapshot);

	//Notify MGMT framework
	plugin_manager::notify_port_delete((const switch_port_snapshot_t*)port_snapshot);	

	//Destroy the snapshot
	switch_port_destroy_snapshot(port_snapshot);
	
	return result;
}

afa_result_t cmm_notify_port_status_changed(switch_port_snapshot_t* port_snapshot){
	
	afa_result_t result = AFA_SUCCESS;
	
	if (!port_snapshot)
		return AFA_FAILURE;

	//Notify port manager
	port_manager::notify_port_status_changed(port_snapshot);

	//Notify attached sw
	if(port_snapshot->is_attached_to_sw)
		//Note that this typecast is valid because afa_result_t and rofl_result_t have intentionally and explicitely the same definition
		result = (afa_result_t)switch_manager::__notify_port_status_changed((const switch_port_snapshot_t*)port_snapshot);

	//Notify MGMT framework
	plugin_manager::notify_port_status_changed((const switch_port_snapshot_t*)port_snapshot);	

	//Destroy the snapshot
	switch_port_destroy_snapshot(port_snapshot);

	return result;
}

afa_result_t cmm_notify_monitoring_state_changed(monitoring_snapshot_state_t* monitoring_snapshot){

	afa_result_t result = AFA_SUCCESS;
	
	if (!monitoring_snapshot)
		return AFA_FAILURE;

	//Notify MGMT framework
	plugin_manager::notify_monitoring_state_changed((const monitoring_snapshot_state_t*)monitoring_snapshot);	

	//Destroy the snapshot
	monitoring_destroy_snapshot(monitoring_snapshot);

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
	
	//Note that this typecast is valid because afa_result_t and rofl_result_t have intentionally and explicitely the same definition
	return (afa_result_t)switch_manager::__process_of1x_packet_in(dpid, table_id, reason, in_port, buffer_id, pkt_buffer, buf_len, total_len, matches);	
}

afa_result_t cmm_process_of1x_flow_removed(uint64_t dpid, uint8_t reason, of1x_flow_entry_t* removed_flow_entry){
	//Note that this typecast is valid because afa_result_t and rofl_result_t have intentionally and explicitely the same definition
	return (afa_result_t)switch_manager::__process_of1x_flow_removed(dpid, reason, removed_flow_entry);
}
