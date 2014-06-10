/*
 * @section LICENSE
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * @author: msune, akoepsel, tjungel, valvarez, 
 * 
 * @section DESCRIPTION 
 * 
 * GNU/Linux driver dispatching routines. This file contains primary HAL driver hooks
 * for CMM to call driver specific functions (e.g. bring up port, or create logical switch).
 * Openflow version dependant hooks are under openflow/ folder. 
*/


#include <stdio.h>
#include <rofl/datapath/hal/driver.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/hal/cmm.h>
#include <rofl/datapath/pipeline/platform/memory.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include "../processing/processingmanager.h"
#include "../io/bufferpool.h"
#include "../io/iomanager.h"
#include "../bg_taskmanager.h"

#include "../io/iface_utils.h"
#include "../io/pktin_dispatcher.h"
#include "../processing/ls_internal_state.h"

//only for Test
#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>

using namespace xdpd::gnu_linux;

//Driver static info
#define GNU_LINUX_CODE_NAME "gnu-linux"
#define GNU_LINUX_VERSION VERSION 
#define GNU_LINUX_DESC \
"GNU/Linux user-space driver.\n\nThe GNU/Linux driver is a user-space driver and serves as a reference implementation. It contains all the necessary bits and pieces to process packets in software, including a complete I/O subsystem written in C/C++. Access to network interfaces (NICs) is done via PACKET_MMAP.\n\nAlthough this driver does not provide cutting-edge performance, still provides a reasonable level of throughput\n\nFeatures:\n - Supports the following OpenFlow versions: v1.0, v1.2, v1.3.X\n - Supports multiple Logical Switch Instances (LSIs)\n - Supports virtual links between LSIs\n - Supports vast majority of network protocols defined by OpenFlow + extensions (GTP, PPP/PPPoE).\n\nMore details here:\n\nhttp://www.xdpd.org"

#define GNU_LINUX_USAGE  "" //We don't support extra params
#define GNU_LINUX_EXTRA_PARAMS "" //We don't support extra params

/*
* @name    hal_driver_init
* @brief   Initializes driver. Before using the HAL_DRIVER routines, higher layers must allow driver to initialize itself
* @ingroup driver_management
*/
hal_result_t hal_driver_init(const char* extra_params){

	ROFL_INFO(DRIVER_NAME" Initializing driver...\n");
	
	//Init the ROFL-PIPELINE phyisical switch
	if(physical_switch_init() != ROFL_SUCCESS)
		return HAL_FAILURE;
	

	//create bufferpool
	bufferpool::init();

	if(discover_physical_ports() != ROFL_SUCCESS)
		return HAL_FAILURE;

	//Initialize the iomanager
	iomanager::init();

	//Initialize Background Tasks Manager
	if(launch_background_tasks_manager() != ROFL_SUCCESS){
		return HAL_FAILURE;
	}

	return HAL_SUCCESS; 
}

/**
* @name    hal_driver_get_info
* @brief   Get the information of the driver (code-name, version, usage...)
* @ingroup driver_management
*/
void hal_driver_get_info(driver_info_t* info){
	//Fill-in driver_info_t
	strncpy(info->code_name, GNU_LINUX_CODE_NAME, DRIVER_CODE_NAME_MAX_LEN);
	strncpy(info->version, GNU_LINUX_VERSION, DRIVER_VERSION_MAX_LEN);
	strncpy(info->description, GNU_LINUX_DESC, DRIVER_DESCRIPTION_MAX_LEN);
	strncpy(info->usage, GNU_LINUX_USAGE, DRIVER_USAGE_MAX_LEN);
	strncpy(info->extra_params, GNU_LINUX_EXTRA_PARAMS, DRIVER_EXTRA_PARAMS_MAX_LEN);
}


/*
* @name    hal_driver_destroy
* @brief   Destroy driver state. Allows platform state to be properly released. 
* @ingroup driver_management
*/
hal_result_t hal_driver_destroy(){

	unsigned int i, max_switches;
	of_switch_t** switch_list;

	//Initialize the iomanager (Stop feeding packets)
	iomanager::destroy();

	//Stop all logical switch instances (stop processing packets)
	switch_list = physical_switch_get_logical_switches(&max_switches);
	for(i=0;i<max_switches;++i){
		if(switch_list[i] != NULL){
			hal_driver_destroy_switch_by_dpid(switch_list[i]->dpid);
		}
	}

	//Stop the bg manager
	stop_background_tasks_manager();

	//Destroy interfaces
	destroy_ports();

	//Destroy physical switch (including ports)
	physical_switch_destroy();
	
	// destroy bufferpool
	bufferpool::destroy();

	//Print stats if any
	TM_DUMP_MEASUREMENTS();
	
	ROFL_INFO(DRIVER_NAME" driver destroyed.\n");
	
	return HAL_SUCCESS; 
}

/*
* Switch management functions
*/
/**
* @brief   Checks if an LSI with the specified dpid exists 
* @ingroup logical_switch_management
*/
bool hal_driver_switch_exists(uint64_t dpid){
	return physical_switch_get_logical_switch_by_dpid(dpid) != NULL;
}

/**
* @brief   Retrieve the list of LSIs dpids
* @ingroup logical_switch_management
* @retval  List of available dpids, which MUST be deleted using dpid_list_destroy().
*/
dpid_list_t* hal_driver_get_all_lsi_dpids(void){
	return physical_switch_get_all_lsi_dpids();  
}

/**
 * @name hal_driver_get_switch_snapshot_by_dpid 
 * @brief Retrieves a snapshot of the current state of a switch port, if the port name is found. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup logical_switch_management
 * @retval  Pointer to of_switch_snapshot_t instance or NULL 
 */
of_switch_snapshot_t* hal_driver_get_switch_snapshot_by_dpid(uint64_t dpid){
	return physical_switch_get_logical_switch_snapshot(dpid);
}


/*
* @name    hal_driver_create_switch 
* @brief   Instruct driver to create an OF logical switch 
* @ingroup logical_switch_management
* @retval  Pointer to of_switch_t instance 
*/
hal_result_t hal_driver_create_switch(char* name, uint64_t dpid, of_version_t of_version, unsigned int num_of_tables, int* ma_list){
	
	of_switch_t* sw;
	
	sw = (of_switch_t*)of1x_init_switch(name, of_version, dpid, num_of_tables, (enum of1x_matching_algorithm_available*) ma_list);

	if(unlikely(!sw))
		return HAL_FAILURE;

	//Create RX ports
	processingmanager::create_rx_pgs(sw);

	//Add switch to the bank	
	physical_switch_add_logical_switch(sw);
	
	return HAL_SUCCESS;
}

/*
* @name    hal_driver_destroy_switch_by_dpid 
* @brief   Instructs the driver to destroy the switch with the specified dpid 
* @ingroup logical_switch_management
*/
hal_result_t hal_driver_destroy_switch_by_dpid(const uint64_t dpid){

	unsigned int i;
	
	//Try to retrieve the switch
	of_switch_t* sw = physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(!sw)
		return HAL_FAILURE;

	//Stop all ports and remove it from being scheduled by I/O first
	for(i=0;i<sw->max_ports;i++){
		if(sw->logical_ports[i].attachment_state == LOGICAL_PORT_STATE_ATTACHED && sw->logical_ports[i].port){
			//Take it out from the group
			if( iomanager::remove_port((ioport*)sw->logical_ports[i].port->platform_port_state) != ROFL_SUCCESS ){
				ROFL_ERR(DRIVER_NAME" WARNING! Error removing port %s from the iomanager for the switch: %s. This can leave the port unusable in the future.\n", sw->logical_ports[i].port->name, sw->name);
				assert(0);
			}

		}
	}
	
	//Create RX ports
	processingmanager::destroy_rx_pgs(sw);	 

	//Drain existing packet ins
	drain_packet_ins(sw);

	//Detach ports from switch. Do not feed more packets to the switch
	if(physical_switch_detach_all_ports_from_logical_switch(sw)!=ROFL_SUCCESS)
		return HAL_FAILURE;
	
	//Remove switch from the switch bank
	if(physical_switch_remove_logical_switch(sw)!=ROFL_SUCCESS)
		return HAL_FAILURE;
	
	return HAL_SUCCESS;
}

/*
* Port management 
*/

/**
* @brief   Checks if a port with the specified name exists 
* @ingroup port_management 
*/
bool hal_driver_port_exists(const char *name){
	return physical_switch_get_port_by_name(name) != NULL; 
}

/**
* @brief   Retrieve the list of names of the available ports of the platform. You may want to 
* 	   call hal_driver_get_port_snapshot_by_name(name) to get more information of the port 
* @ingroup port_management
* @retval  List of available port names, which MUST be deleted using switch_port_name_list_destroy().
*/
switch_port_name_list_t* hal_driver_get_all_port_names(void){
	return physical_switch_get_all_port_names(); 
}

/**
 * @name hal_driver_get_port_by_name
 * @brief Retrieves a snapshot of the current state of a switch port, if the port name is found. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup port_management
 */
switch_port_snapshot_t* hal_driver_get_port_snapshot_by_name(const char *name){
	return physical_switch_get_port_snapshot(name); 
}

/**
 * @name hal_driver_get_port_by_num
 * @brief Retrieves a snapshot of the current state of the port of the Logical Switch Instance with dpid at port_num, if exists. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup port_management
 * @param dpid DatapathID 
 * @param port_num Port number
 */
switch_port_snapshot_t* hal_driver_get_port_snapshot_by_num(uint64_t dpid, unsigned int port_num){
	
	of_switch_t* lsw;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return NULL; 

	//Check if the port does exist.
	if(!port_num || port_num >= LOGICAL_SWITCH_MAX_LOG_PORTS || !lsw->logical_ports[port_num].port)
		return NULL;

	return physical_switch_get_port_snapshot(lsw->logical_ports[port_num].port->name); 
}


/*
* @name    hal_driver_attach_physical_port_to_switch
* @brief   Attemps to attach a system's port to switch, at of_port_num if defined, otherwise in the first empty OF port number.
* @ingroup management
*
* @param dpid Datapath ID of the switch to attach the ports to
* @param name Port name (system's name)
* @param of_port_num If *of_port_num is non-zero, try to attach to of_port_num of the logical switch, otherwise try to attach to the first available port and return the result in of_port_num
*/
hal_result_t hal_driver_attach_port_to_switch(uint64_t dpid, const char* name, unsigned int* of_port_num){

	switch_port_t* port;
	of_switch_t* lsw;

	//Check switch existance
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw){
		return HAL_FAILURE;
	}
	
	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);
	if(!port)
		return HAL_FAILURE;

	//Update pipeline state
	if(*of_port_num == 0){
		//no port specified, we assign the first available
		if(physical_switch_attach_port_to_logical_switch(port,lsw,of_port_num) == ROFL_FAILURE){
			assert(0);
			return HAL_FAILURE;
		}
	}else{

		if(physical_switch_attach_port_to_logical_switch_at_port_num(port,lsw,*of_port_num) == ROFL_FAILURE){
			assert(0);
			return HAL_FAILURE;
		}
	}
	
	//Add it to the iomanager
	if(iomanager::add_port((ioport*)port->platform_port_state) != ROFL_SUCCESS){
		return HAL_FAILURE;	
	}

	return HAL_SUCCESS;
}

/**
* @name    hal_driver_connect_switches
* @brief   Attempts to connect two logical switches via a virtual port. Driver may or may not support this functionality. On success, the two ports must be functional and process packets and the driver MUST inform the CMM of the new ports via two separate port_add messages, with the appropriate information of attachment of the ports. 
* @ingroup management
*
* @param dpid_lsi1 Datapath ID of the LSI1
* @param port1 A pointer to a snapshot of the virtual port attached to the LS1 that MUST be destroyed using switch_port_destroy_snapshot()
* @param dpid_lsi2 Datapath ID of the LSI2
* @param port1 A pointer to a snapshot of the virtual port attached to the LS2 that MUST be destroyed using switch_port_destroy_snapshot()
*/
hal_result_t hal_driver_connect_switches(uint64_t dpid_lsi1, switch_port_snapshot_t** port1, uint64_t dpid_lsi2, switch_port_snapshot_t** port2){

	of_switch_t *lsw1, *lsw2;
	switch_port_snapshot_t *port1_not, *port2_not;
	ioport *vport1, *vport2;
	unsigned int port_num = 0; //We don't care about of the port

	//Check existance of the dpid
	lsw1 = physical_switch_get_logical_switch_by_dpid(dpid_lsi1);
	lsw2 = physical_switch_get_logical_switch_by_dpid(dpid_lsi2);

	if(!lsw1 || !lsw2){
		assert(0);
		return HAL_FAILURE;
	}
	
	//Create virtual port pair
	if(create_virtual_port_pair(lsw1, &vport1, lsw2, &vport2) != ROFL_SUCCESS){
		assert(0);
		return HAL_FAILURE;
	}

	//Attach both ports
	if(hal_driver_attach_port_to_switch(dpid_lsi1, vport1->of_port_state->name, &port_num) != HAL_SUCCESS){
		assert(0);
		return HAL_FAILURE;
	}
	port_num=0;
	if(hal_driver_attach_port_to_switch(dpid_lsi2, vport2->of_port_state->name, &port_num) != HAL_SUCCESS){
		assert(0);
		return HAL_FAILURE;
	}

	//Notify port add as requested by the API
	port1_not = physical_switch_get_port_snapshot(vport1->of_port_state->name);
	port2_not = physical_switch_get_port_snapshot(vport2->of_port_state->name);
	if(!port1_not || !port2_not){
		assert(0);
		return HAL_FAILURE;
	}
	
	hal_cmm_notify_port_add(port1_not);
	hal_cmm_notify_port_add(port2_not);


	//Enable interfaces (start packet transmission)
	if(hal_driver_bring_port_up(vport1->of_port_state->name) != HAL_SUCCESS || hal_driver_bring_port_up(vport2->of_port_state->name) != HAL_SUCCESS){
		ROFL_ERR(DRIVER_NAME" ERROR: unable to bring up vlink ports.\n");
		assert(0);
		return HAL_FAILURE;
	}

	//Set switch ports and return
	*port1 = physical_switch_get_port_snapshot(vport1->of_port_state->name);
	*port2 = physical_switch_get_port_snapshot(vport2->of_port_state->name);
	
	assert(*port1 != NULL);
	assert(*port2 != NULL);

	return HAL_SUCCESS; 
}

/*
* @name    hal_driver_detach_port_from_switch
* @brief   Detaches a port from the switch 
* @ingroup port_management
*
* @param dpid Datapath ID of the switch to detach the ports
* @param name Port name (system's name)
*/
hal_result_t hal_driver_detach_port_from_switch(uint64_t dpid, const char* name){

	of_switch_t* lsw;
	switch_port_t* port;
	switch_port_snapshot_t *port_snapshot=NULL, *port_pair_snapshot=NULL;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return HAL_FAILURE;

	port = physical_switch_get_port_by_name(name);

	//Check if the port does exist and is really attached to the dpid
	if( !port || !port->attached_sw || port->attached_sw->dpid != dpid)
		return HAL_FAILURE;

	//Snapshoting the port *before* it is detached 
	port_snapshot = physical_switch_get_port_snapshot(port->name); 
	
	if(!port_snapshot){
		assert(0);
		return HAL_FAILURE;
	}
	
	//Remove it from the iomanager (do not feed more packets)
	if(iomanager::remove_port((ioport*)port->platform_port_state) != ROFL_SUCCESS){
		ROFL_ERR(DRIVER_NAME" Error removing port %s from the iomanager. The port may become unusable...\n",port->name);
		assert(0);
		goto FWD_MODULE_DETACH_ERROR;	
	}
	
	//Detach it
	if(physical_switch_detach_port_from_logical_switch(port,lsw) != ROFL_SUCCESS){
		ROFL_ERR(DRIVER_NAME" Error detaching port %s.\n",port->name);
		assert(0);
		goto FWD_MODULE_DETACH_ERROR;	
	}
	
	//For virtual ports, remove counter port
	if(port->type == PORT_TYPE_VIRTUAL){
		switch_port_t* port_pair = get_vlink_pair(port); 

		if(!port_pair){
			ROFL_ERR(DRIVER_NAME" Error detaching a virtual link port. Could not find the counter port of %s.\n",port->name);
			assert(0);
			goto FWD_MODULE_DETACH_ERROR;
		}
	
		//Recover the snapshot *before* detachment 
		port_pair_snapshot = physical_switch_get_port_snapshot(port_pair->name);
	
		//Remove it from the iomanager (do not feed more packets)
		if(iomanager::remove_port((ioport*)port_pair->platform_port_state) != ROFL_SUCCESS){
			ROFL_ERR(DRIVER_NAME" Error removing port %s from the iomanager. The port may become unusable...\n",port->name);
			assert(0);
			goto FWD_MODULE_DETACH_ERROR;
		}
	
		if(!port_pair->attached_sw || physical_switch_detach_port_from_logical_switch(port_pair,port_pair->attached_sw) != ROFL_SUCCESS){
			ROFL_ERR(DRIVER_NAME" Error detaching port-pair %s from the sw.\n",port_pair->name);
			assert(0);
			goto FWD_MODULE_DETACH_ERROR;
		}

		//notify port detached and deleted
		hal_cmm_notify_port_delete(port_pair_snapshot);
		
		//Remove ioports
		delete (ioport*)port->platform_port_state;
		delete (ioport*)port_pair->platform_port_state;

		//Remove from the pipeline and delete
		if(physical_switch_remove_port(port->name) != ROFL_SUCCESS){
			ROFL_ERR(DRIVER_NAME" Error removing port from the physical_switch. The port may become unusable...\n");
			assert(0);
			return HAL_FAILURE;
			
		}
		
		if(physical_switch_remove_port(port_pair->name) != ROFL_SUCCESS){
			ROFL_ERR(DRIVER_NAME" Error removing port from the physical_switch. The port may become unusable...\n");
			assert(0);
			goto FWD_MODULE_DETACH_ERROR;
			
		}

		//notify port detached and deleted
		hal_cmm_notify_port_delete(port_snapshot);
	}else{
		//There was no notification so release
		switch_port_destroy_snapshot(port_snapshot);
	}
	
	return HAL_SUCCESS; 

FWD_MODULE_DETACH_ERROR:

	if(port_snapshot)
		switch_port_destroy_snapshot(port_snapshot);	
	if(port_pair_snapshot)
		switch_port_destroy_snapshot(port_pair_snapshot);	

	return HAL_FAILURE;		
}


/*
* @name    hal_driver_detach_port_from_switch_at_port_num
* @brief   Detaches port_num of the logical switch identified with dpid 
* @ingroup port_management
*
* @param dpid Datapath ID of the switch to detach the ports
* @param of_port_num Number of the port (OF number) 
*/
hal_result_t hal_driver_detach_port_from_switch_at_port_num(uint64_t dpid, const unsigned int of_port_num){

	of_switch_t* lsw;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return HAL_FAILURE;

	//Check if the port does exist.
	if(!of_port_num || of_port_num >= LOGICAL_SWITCH_MAX_LOG_PORTS || !lsw->logical_ports[of_port_num].port)
		return HAL_FAILURE;

	return hal_driver_detach_port_from_switch(dpid, lsw->logical_ports[of_port_num].port->name);
}


//Port admin up/down stuff

/*
* Port administrative management actions (ifconfig up/down like)
*/

/*
* @name    hal_driver_bring_port_up
* @brief   Brings up a system port. If the port is attached to an OF logical switch, this also schedules port for I/O and triggers PORTMOD message. 
* @ingroup port_management
*
* @param name Port system name 
*/
hal_result_t hal_driver_bring_port_up(const char* name){

	switch_port_t* port;
	switch_port_snapshot_t* port_snapshot;

	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);

	if(!port || !port->platform_port_state)
		return HAL_FAILURE;

	//Bring it up
	if(port->attached_sw){
		//Port is attached and belonging to a port group. Instruct I/O manager to start the port
		if(iomanager::bring_port_up((ioport*)port->platform_port_state)!=ROFL_SUCCESS)
			return HAL_FAILURE;
	}else{
		//The port is not attached. Only bring it up (ifconfig up)
		if( ( (ioport*)port->platform_port_state)->up() != ROFL_SUCCESS )
			return HAL_FAILURE;
	}

	//Notify only if its virtual; otherwise bg will do it for us
	if(port->type == PORT_TYPE_VIRTUAL){
		port_snapshot = physical_switch_get_port_snapshot(port->name); 
		hal_cmm_notify_port_status_changed(port_snapshot);
	}
		
	return HAL_SUCCESS;
}

/*
* @name    hal_driver_bring_port_down
* @brief   Shutdowns (brings down) a system port. If the port is attached to an OF logical switch, this also de-schedules port and triggers PORTMOD message. 
* @ingroup port_management
*
* @param name Port system name 
*/
hal_result_t hal_driver_bring_port_down(const char* name){

	switch_port_t* port;
	switch_port_snapshot_t* port_snapshot;
	
	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);
	if(!port || !port->platform_port_state)
		return HAL_FAILURE;

	//Bring it down
	if(port->attached_sw){
		//Port is attached and belonging to a port group. Instruct I/O manager to stop the port
		if( iomanager::bring_port_down((ioport*)port->platform_port_state)!=ROFL_SUCCESS)
			return HAL_FAILURE;
	}else{
		//The port is not attached. Only bring it down (ifconfig down)
		if( ( (ioport*)port->platform_port_state)->down() != ROFL_SUCCESS )
			return HAL_FAILURE;
	}

	//Notify only if its virtual; otherwise bg will do it for us
	if(port->type == PORT_TYPE_VIRTUAL){
		port_snapshot = physical_switch_get_port_snapshot(port->name); 
		hal_cmm_notify_port_status_changed(port_snapshot);
	}

	return HAL_SUCCESS;
}

/*
* @name    hal_driver_bring_port_up_by_num
* @brief   Brings up a port from an OF logical switch (and the underlying physical interface). This function also triggers the PORTMOD message 
* @ingroup port_management
*
* @param dpid DatapathID 
* @param port_num OF port number
*/
hal_result_t hal_driver_bring_port_up_by_num(uint64_t dpid, unsigned int port_num){

	of_switch_t* lsw;
	switch_port_snapshot_t* port_snapshot;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return HAL_FAILURE;

	//Check if the port does exist and is really attached to the dpid
	if( !lsw->logical_ports[port_num].port || lsw->logical_ports[port_num].attachment_state != LOGICAL_PORT_STATE_ATTACHED || lsw->logical_ports[port_num].port->attached_sw->dpid != dpid)
		return HAL_FAILURE;

	//Call I/O manager to bring it up
	if(iomanager::bring_port_up((ioport*)lsw->logical_ports[port_num].port->platform_port_state) != ROFL_SUCCESS)
		return HAL_FAILURE;
	
	//Notify only if its virtual; otherwise bg will do it for us
	if(lsw->logical_ports[port_num].port->type == PORT_TYPE_VIRTUAL){
		port_snapshot = physical_switch_get_port_snapshot(lsw->logical_ports[port_num].port->name); 
		hal_cmm_notify_port_status_changed(port_snapshot);
	}
	
	return HAL_SUCCESS;
}

/*
* @name    hal_driver_bring_port_down_by_num
* @brief   Brings down a port from an OF logical switch (and the underlying physical interface). This also triggers the PORTMOD message.
* @ingroup port_management
*
* @param dpid DatapathID 
* @param port_num OF port number
*/
hal_result_t hal_driver_bring_port_down_by_num(uint64_t dpid, unsigned int port_num){

	of_switch_t* lsw;
	switch_port_snapshot_t* port_snapshot;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return HAL_FAILURE;

	//Check if the port does exist and is really attached to the dpid
	if( !lsw->logical_ports[port_num].port || lsw->logical_ports[port_num].attachment_state != LOGICAL_PORT_STATE_ATTACHED || lsw->logical_ports[port_num].port->attached_sw->dpid != dpid)
		return HAL_FAILURE;

	//Call I/O manager to bring it down
	if(iomanager::bring_port_down((ioport*)lsw->logical_ports[port_num].port->platform_port_state) != ROFL_SUCCESS)
		return HAL_FAILURE;
		
	//Notify only if its virtual; otherwise bg will do it for us
	if(lsw->logical_ports[port_num].port->type == PORT_TYPE_VIRTUAL){
		port_snapshot = physical_switch_get_port_snapshot(lsw->logical_ports[port_num].port->name); 
		hal_cmm_notify_port_status_changed(port_snapshot);
	}
	
	return HAL_SUCCESS;
}

/**
 * @brief Retrieve a snapshot of the monitoring state. If rev is 0, or the current monitoring 
 * has changed (monitoring->rev != rev), a new snapshot of the monitoring state is made. Warning: this 
 * is expensive.
 * @ingroup hal_driver_management
 *
 * @param rev Last seen revision. Set to 0 to always get a new snapshot 
 * @return A snapshot of the monitoring state that MUST be destroyed using monitoring_destroy_snapshot() or NULL if there have been no changes (same rev)
 */ 
monitoring_snapshot_state_t* hal_driver_get_monitoring_snapshot(uint64_t rev){

	monitoring_state_t* mon = physical_switch_get_monitoring();

	if( rev == 0 || monitoring_has_changed(mon, &rev) ) 
		return monitoring_get_snapshot(mon);

	return NULL;
}

/**
 * @brief get a list of available matching algorithms
 * @ingroup hal_driver_management
 *
 * @param of_version
 * @param name_list
 * @param count
 * @return
 */
hal_result_t hal_driver_list_matching_algorithms(of_version_t of_version, const char * const** name_list, int *count){
	return (hal_result_t)of_get_switch_matching_algorithms(of_version, name_list, count);
}
