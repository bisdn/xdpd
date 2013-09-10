#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>
#include <stdio.h>
#include <assert.h>

//Openflow 1.2 cmm -> controller/hcl

/**
 * @name    cmm_process_of1x_flow_removed
 * @brief   Instructs the HCL to process a FLOW_REMOVED event comming from the DP
 * @ingroup of1x_cmm_async_event_processing
 *
 * @param sw Openflow 1.2 switch pointer that generated the FLOW_REMOVED
 * @param removed_flow_entry Pointer to the flow_entry that has been removed
 */
afa_result_t cmm_process_of1x_flow_removed(const of1x_switch_t* sw, uint8_t reason, of1x_flow_entry_t* removed_flow_entry)
{
	fprintf(stderr,"<%s:%d> HCL has been called\n",__func__,__LINE__);
	return AFA_SUCCESS;
}

/*
* Packet in
*/
afa_result_t cmm_process_of1x_packet_in(const of1x_switch_t* sw, 
					uint8_t table_id, 
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					of1x_packet_matches_t matches){

	fprintf(stderr,"Generated PACKET_IN from datapath\n");
	return AFA_SUCCESS;
}


/**
 * @name    cmm_notify_port_status_changed
 * @brief   Notify port add to HCL
 * @ingroup cmm_management
 */
afa_result_t cmm_notify_port_status_changed(switch_port_t* port)
{
	fprintf(stderr,"<%s:%d> HCL has been called (%s - port_no=%d - Link %s)\n",__func__,__LINE__,port->name,port->of_port_num, port->up ? "up" : "down");
	return AFA_SUCCESS;
}

/**
 * @name    cmm_notify_port_delete
 * @brief   Notifies HCL that port has been deleted from the platform, or cannot be associated to a switch
 * @ingroup cmm_management
 */
afa_result_t cmm_notify_port_delete(switch_port_t* port)
{
	fprintf(stderr,"<%s:%d> HCL has been called (port_no=%d)\n",__func__,__LINE__,port->of_port_num);
	return AFA_SUCCESS;
}

/**
 * @name    cmm_notify_port_add
 * @brief   Notify port add to HCL
 * @ingroup cmm_management
 */
afa_result_t cmm_notify_port_add(switch_port_t* port)
{
	fprintf(stderr,"<%s:%d> HCL has been called (port_no=%d)\n",__func__,__LINE__,port->of_port_num);
	return AFA_SUCCESS;
}

