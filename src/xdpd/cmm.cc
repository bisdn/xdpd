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
	
	openflow_switch* sw;
	afa_result_t result=AFA_SUCCESS;
	
	if(!port_snapshot)
		return AFA_FAILURE;
	
	//Notify MGMT framework
	//TODO:

	//Notify attached sw
	if(port_snapshot->is_attached_to_sw){
		if( (sw=switch_manager::find_by_dpid(port_snapshot->attached_sw_dpid)) == NULL ){
	
			//Destroy the snapshot
			switch_port_destroy_snapshot(port_snapshot);

			return AFA_SUCCESS;
		}

		result = sw->notify_port_add(port_snapshot);
	}
	
	//Destroy the snapshot
	switch_port_destroy_snapshot(port_snapshot);
		
	return result;
}


afa_result_t cmm_notify_port_delete(switch_port_snapshot_t* port_snapshot){
	
	openflow_switch* sw;
	afa_result_t result = AFA_SUCCESS;
	
	if (!port_snapshot)
		return AFA_FAILURE;

	//Notify MGMT framework
	//TODO:

	//Notify attached sw
	if(port_snapshot->is_attached_to_sw){
		
		if( (sw=switch_manager::find_by_dpid(port_snapshot->attached_sw_dpid)) == NULL ){
			//Destroy the snapshot
			switch_port_destroy_snapshot(port_snapshot);

			return AFA_SUCCESS;	
		}

		result = sw->notify_port_delete(port_snapshot);
	}

	//Destroy the snapshot
	switch_port_destroy_snapshot(port_snapshot);
	
	return result;
}


afa_result_t cmm_notify_port_status_changed(switch_port_snapshot_t* port_snapshot){
	
	openflow_switch* sw;
	afa_result_t result=AFA_SUCCESS;
	
	if (!port_snapshot)
		return AFA_FAILURE;

	//Notify MGMT framework
	//TODO:

	//Notify attached sw
	if(port_snapshot->is_attached_to_sw){
		
		if( (sw=switch_manager::find_by_dpid(port_snapshot->attached_sw_dpid)) == NULL ){
			//Destroy the snapshot
			switch_port_destroy_snapshot(port_snapshot);

			return AFA_SUCCESS;
		}

		result = sw->notify_port_status_changed(port_snapshot); 
	}

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
					of1x_packet_matches_t matches)
{
	openflow_switch* dp=NULL;
	
	dp = switch_manager::find_by_dpid(dpid);

	if(!dp)
		return AFA_FAILURE;

	return dp->process_packet_in(table_id,
					reason,
					in_port,
					buffer_id,
					pkt_buffer,
					buf_len,
					total_len,
					matches);
}

afa_result_t cmm_process_of1x_flow_removed(const of1x_switch_t* sw, uint8_t reason, of1x_flow_entry_t* removed_flow_entry){

	openflow_switch* dp=NULL;
	
	if (!sw) 
		return AFA_FAILURE;
	
	dp = switch_manager::find_by_dpid(sw->dpid);

	if(!dp)
		return AFA_FAILURE;

	return dp->process_flow_removed(reason, removed_flow_entry);
}
