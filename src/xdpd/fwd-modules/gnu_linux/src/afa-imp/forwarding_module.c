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
 * GNU/Linux forwarding_module dispatching routines. This file contains primary AFA driver hooks
 * for CMM to call forwarding module specific functions (e.g. bring up port, or create logical switch).
 * Openflow version dependant hooks are under openflow/ folder. 
*/


#include <stdio.h>
#include <net/if.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/afa/cmm.h>
#include <rofl/datapath/pipeline/platform/memory.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow12/of12_switch.h>
#include "../util/ringbuffer_c_wrapper.h"
#include "../processing/processingmanager.h"
#include "../io/bufferpool_c_wrapper.h"
#include "../io/iomanager_c_wrapper.h"
#include "../bg_taskmanager.h"

#include "../io/iface_utils.h"
#include "../ls_internal_state.h"

//only for Test
#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>


#define NUM_ELEM_INIT_BUFFERPOOL 2048 //This is cache for fast port addition

//FIXME: implement it properly. Single portgroup ID emulation 
static int iomanager_grp_id;

/*
* @name    fwd_module_init
* @brief   Initializes driver. Before using the AFA_DRIVER routines, higher layers must allow driver to initialize itself
* @ingroup fwd_module_management
*/
afa_result_t fwd_module_init(){
	
	//Init the ROFL-PIPELINE phyisical switch
	if(physical_switch_init() != ROFL_SUCCESS)
		return AFA_FAILURE;
	
	if(discover_physical_ports() != ROFL_SUCCESS)
		return AFA_FAILURE;
	
	//create bufferpool
	bufferpool_init_wrapper(NUM_ELEM_INIT_BUFFERPOOL);
	
	//create a port_group NOTE so far we will only have one of these
	// the managment of port_groups creation, destroy, port adding and removing will be done later
	unsigned int num_of_threads = 1;

	if((iomanager_grp_id = iomanager_create_group_wrapper(num_of_threads)) != ROFL_SUCCESS){
		return AFA_FAILURE;
	}

	//Initialize Background Tasks Manager
	if(launch_background_tasks_manager() != ROFL_SUCCESS){
		return AFA_FAILURE;
	}
	
	return AFA_SUCCESS; 
}

/*
* @name    fwd_module_destroy
* @brief   Destroy driver state. Allows platform state to be properly released. 
* @ingroup fwd_module_management
*/
afa_result_t fwd_module_destroy(){

	//destroy group ports
	iomanager_delete_group_wrapper(iomanager_grp_id);
	
	//Stop the bg manager
	stop_background_tasks_manager();

	//Destroy interfaces
	destroy_ports();

	//Destroy physical switch (including ports)
	physical_switch_destroy();
	
	// destroy bufferpool
	bufferpool_destroy_wrapper();
	
	return AFA_SUCCESS; 
}

/*
* Switch management functions
*/

/*
* @name    fwd_module_create_switch 
* @brief   Instruct driver to create an OF logical switch 
* @ingroup logical_switch_management
* @retval  Pointer to of_switch_t instance 
*/
of_switch_t* fwd_module_create_switch(char* name, uint64_t dpid, of_version_t of_version, unsigned int num_of_tables, int* ma_list){
	
	of_switch_t* sw;
	
	switch(of_version){
		case OF_VERSION_12: 
			sw = (of_switch_t*)of12_init_switch(name, dpid, num_of_tables, (enum of12_matching_algorithm_available*) ma_list);
			break;

		//Add more here..
			
		default: 
			return NULL;
	}	

	//Launch switch processing threads
	if(start_ls_workers_wrapper(sw) < 0){
		
		ROFL_ERR("<%s:%d> error initializing workers from processing manager. Destroying switch...\n",__func__,__LINE__);
		of_destroy_switch(sw);
		return NULL;
	}
	
	//Add switch to the bank	
	physical_switch_add_logical_switch(sw);
	
	return sw;
}

/*
* @name    fwd_module_get_switch_by_dpid 
* @brief   Retrieve the switch with the specified dpid  
* @ingroup logical_switch_management
* @retval  Pointer to of_switch_t instance or NULL 
*/
of_switch_t* fwd_module_get_switch_by_dpid(uint64_t dpid){
	
	//Call directly the bank
	return physical_switch_get_logical_switch_by_dpid(dpid); 
}

/*
* @name    fwd_module_destroy_switch_by_dpid 
* @brief   Instructs the driver to destroy the switch with the specified dpid 
* @ingroup logical_switch_management
*/
afa_result_t fwd_module_destroy_switch_by_dpid(const uint64_t dpid){

	int i;
	
	//Try to retrieve the switch
	of_switch_t* sw = physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(!sw)
		return AFA_FAILURE;

	//Stop all ports and remove it from being scheduled by I/O first
	for(i=0;i<LOGICAL_SWITCH_MAX_LOG_PORTS;i++) //FIXME: use sw->max_logical_ports
	{
		if(sw->logical_ports[i].attachment_state == LOGICAL_PORT_STATE_ATTACHED && sw->logical_ports[i].port){
			//Take it out from the group
			if(iomanager_remove_port_from_group_wrapper(iomanager_grp_id, sw->logical_ports[i].port->platform_port_state) == ROFL_FAILURE){
				//TODO: put trace here?
			}

		}
	}
	
	//Detach ports from switch. Do not feed more packets to the switch
	if(physical_switch_detach_all_ports_from_logical_switch(sw)!=ROFL_SUCCESS)
		return AFA_FAILURE;
	
	//stop the threads here (it is blocking)
	if(stop_ls_workers_wrapper(sw)!= ROFL_SUCCESS)
		ROFL_ERR("<%s:%d> error stopping workers from processing manager\n",__func__,__LINE__);
	
	//Remove switch from the switch bank
	if(physical_switch_remove_logical_switch(sw)!=ROFL_SUCCESS)
		return AFA_FAILURE;
	
	return AFA_SUCCESS;
}

/*
* Port management 
*/

/*
* @name    fwd_module_list_platform_ports
* @brief   Retrieve the list of ports of the platform 
* @ingroup port_management
* @retval  Pointer to the first port. 
*/
switch_port_t* fwd_module_list_platform_ports(){
	/* TODO FIXME */
	return NULL;
}

/*
 * @name fwd_module_get_port_by_name
 * @brief Get a reference to the port by its name 
 * @ingroup port_management
 */
switch_port_t* fwd_module_get_port_by_name(const char *name){
	return physical_switch_get_port_by_name(name);
}

/*
* @name    fwd_module_get_physical_ports_ports
* @brief   Retrieve the list of the physical ports of the switch
* @ingroup port_management
* @retval  Pointer to the first port. 
*/
switch_port_t** fwd_module_get_physical_ports(unsigned int* num_of_ports){
	return physical_switch_get_physical_ports(num_of_ports);
}

/*
* @name    fwd_module_get_virtual_ports
* @brief   Retrieve the list of virtual ports of the platform
* @ingroup port_management
* @retval  Pointer to the first port. 
*/
switch_port_t** fwd_module_get_virtual_ports(unsigned int* num_of_ports){
	return physical_switch_get_virtual_ports(num_of_ports);
}

/*
* @name    fwd_module_get_tunnel_ports
* @brief   Retrieve the list of tunnel ports of the platform
* @ingroup port_management
* @retval  Pointer to the first port. 
*/
switch_port_t** fwd_module_get_tunnel_ports(unsigned int* num_of_ports){
	return physical_switch_get_tunnel_ports(num_of_ports);
}
/*
* @name    fwd_module_attach_physical_port_to_switch
* @brief   Attemps to attach a system's port to switch, at of_port_num if defined, otherwise in the first empty OF port number.
* @ingroup management
*
* @param dpid Datapath ID of the switch to attach the ports to
* @param name Port name (system's name)
* @param of_port_num If *of_port_num is non-zero, try to attach to of_port_num of the logical switch, otherwise try to attach to the first available port and return the result in of_port_num
*/
afa_result_t fwd_module_attach_port_to_switch(uint64_t dpid, const char* name, unsigned int* of_port_num){

	switch_port_t* port;
	of_switch_t* lsw;

	//Check switch existance
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw){
		return AFA_FAILURE;
	}
	
	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);
	if(!port)
		return AFA_FAILURE;

	//Try first to add the port to the portgroup
	if(iomanager_add_port_to_group_wrapper(iomanager_grp_id, port->platform_port_state) == ROFL_FAILURE)
		return AFA_FAILURE;
	
	//Attaching the port
	if(*of_port_num == 0){
		//no port specified, we assign the first available
		if(physical_switch_attach_port_to_logical_switch(port,lsw,of_port_num) == ROFL_FAILURE){
			iomanager_remove_port_from_group_wrapper(iomanager_grp_id, port->platform_port_state);	
			return AFA_FAILURE;
		}
	}else{

		if(physical_switch_attach_port_to_logical_switch_at_port_num(port,lsw,*of_port_num) == ROFL_FAILURE){
			iomanager_remove_port_from_group_wrapper(iomanager_grp_id, port->platform_port_state);	
			return AFA_FAILURE;
		}
	}
	
	//notify port attached
	if(cmm_notify_port_add(port)!=AFA_SUCCESS){
		return AFA_FAILURE;
	}
	
	return AFA_SUCCESS;
}

/*
* @name    fwd_module_detach_port_from_switch
* @brief   Detaches a port from the switch 
* @ingroup port_management
*
* @param dpid Datapath ID of the switch to detach the ports
* @param name Port name (system's name)
*/
afa_result_t fwd_module_detach_port_from_switch(uint64_t dpid, const char* name){

	of_switch_t* lsw;
	switch_port_t* port;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return AFA_FAILURE;

	port = physical_switch_get_port_by_name(name);

	//Check if the port does exist and is really attached to the dpid
	if( !port || port->attached_sw->dpid != dpid)
		return AFA_FAILURE;

	//Remove port from the portgroup
	if(iomanager_remove_port_from_group_wrapper(iomanager_grp_id, port->platform_port_state) == ROFL_FAILURE)
		return AFA_FAILURE;

	if(physical_switch_detach_port_from_logical_switch(port,lsw) == ROFL_FAILURE)
		return AFA_FAILURE;
	
	//notify port dettached
	if(cmm_notify_port_delete(port)!=AFA_SUCCESS){
		return AFA_FAILURE;
	}

	return AFA_SUCCESS; 
}

/*
* @name    fwd_module_detach_port_from_switch_at_port_num
* @brief   Detaches port_num of the logical switch identified with dpid 
* @ingroup port_management
*
* @param dpid Datapath ID of the switch to detach the ports
* @param of_port_num Number of the port (OF number) 
*/
afa_result_t fwd_module_detach_port_from_switch_at_port_num(uint64_t dpid, const unsigned int of_port_num){

	of_switch_t* lsw;
	switch_port_t* port;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return AFA_FAILURE;

	//Check if the port does exist.
	if(!of_port_num || of_port_num >= LOGICAL_SWITCH_MAX_LOG_PORTS)
		return AFA_FAILURE;

	port = lsw->logical_ports[of_port_num].port;
	
	if(physical_switch_detach_port_from_logical_switch(port, lsw)==ROFL_FAILURE)
		return AFA_FAILURE;
	
	if(cmm_notify_port_delete(port)!=AFA_SUCCESS)
		return AFA_FAILURE;
		
	return AFA_SUCCESS;
}


//Port admin up/down stuff

/*
* Port administrative management actions (ifconfig up/down like)
*/

/*
* @name    fwd_module_enable_port
* @brief   Brings up a system port. If the port is attached to an OF logical switch, this also schedules port for I/O and triggers PORTMOD message. 
* @ingroup port_management
*
* @param name Port system name 
*/
afa_result_t fwd_module_enable_port(const char* name){

	switch_port_t* port;

	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);

	if(!port || !port->platform_port_state)
		return AFA_FAILURE;

	//Bring it up
	if(port->attached_sw){
		//Port is attached and belonging to a port group. Instruct I/O manager to start the port
		if(iomanager_bring_port_up_wrapper(port->platform_port_state)!=ROFL_SUCCESS)
			return AFA_SUCCESS;
	}else{
		//The port is not attached. Only bring it up (ifconfig up)
		if(enable_port(port->platform_port_state)==ROFL_FAILURE)
			return AFA_FAILURE;
	}

	if(cmm_notify_port_status_changed(port)!=AFA_SUCCESS)
		return AFA_FAILURE;
	
	return AFA_SUCCESS;
}

/*
* @name    fwd_module_disable_port
* @brief   Shutdowns (brings down) a system port. If the port is attached to an OF logical switch, this also de-schedules port and triggers PORTMOD message. 
* @ingroup port_management
*
* @param name Port system name 
*/
afa_result_t fwd_module_disable_port(const char* name){

	switch_port_t* port;
	
	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);
	if(!port || !port->platform_port_state)
		return AFA_FAILURE;

	//Bring it down
	if(port->attached_sw){
		//Port is attached and belonging to a port group. Instruct I/O manager to stop the port
		if( iomanager_bring_port_down_wrapper(port->platform_port_state)==ROFL_FAILURE)
			return AFA_FAILURE;
	}else{
		//The port is not attached. Only bring it down (ifconfig down)
		if(disable_port(port->platform_port_state)==ROFL_FAILURE)
			return AFA_FAILURE;
	}

	if(cmm_notify_port_status_changed(port)!=AFA_SUCCESS)
		return AFA_FAILURE;
	
	return AFA_SUCCESS;
}

/*
* @name    fwd_module_enable_port_by_num
* @brief   Brings up a port from an OF logical switch (and the underlying physical interface). This function also triggers the PORTMOD message 
* @ingroup port_management
*
* @param dpid DatapathID 
* @param port_num OF port number
*/
afa_result_t fwd_module_enable_port_by_num(uint64_t dpid, unsigned int port_num){

	of_switch_t* lsw;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return AFA_FAILURE;

	//Check if the port does exist and is really attached to the dpid
	if( !lsw->logical_ports[port_num].port || lsw->logical_ports[port_num].attachment_state != LOGICAL_PORT_STATE_ATTACHED || lsw->logical_ports[port_num].port->attached_sw->dpid != dpid)
		return AFA_FAILURE;

	//Call I/O manager to bring it up
	if(iomanager_bring_port_up_wrapper(lsw->logical_ports[port_num].port->platform_port_state) == ROFL_FAILURE)
		return AFA_FAILURE;
	
	if(cmm_notify_port_status_changed(lsw->logical_ports[port_num].port)!=AFA_SUCCESS)
		return AFA_FAILURE;
	
	return AFA_SUCCESS;
}

/*
* @name    fwd_module_disable_port_by_num
* @brief   Brings down a port from an OF logical switch (and the underlying physical interface). This also triggers the PORTMOD message.
* @ingroup port_management
*
* @param dpid DatapathID 
* @param port_num OF port number
*/
afa_result_t fwd_module_disable_port_by_num(uint64_t dpid, unsigned int port_num){

	of_switch_t* lsw;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return AFA_FAILURE;

	//Check if the port does exist and is really attached to the dpid
	if( !lsw->logical_ports[port_num].port || lsw->logical_ports[port_num].attachment_state != LOGICAL_PORT_STATE_ATTACHED || lsw->logical_ports[port_num].port->attached_sw->dpid != dpid)
		return AFA_FAILURE;

	//Call I/O manager to bring it down
	if(iomanager_bring_port_down_wrapper(lsw->logical_ports[port_num].port->platform_port_state) == ROFL_FAILURE)
		return AFA_FAILURE;
	
	if(cmm_notify_port_status_changed(lsw->logical_ports[port_num].port)!=AFA_SUCCESS)
		return AFA_FAILURE;
	
	return AFA_SUCCESS;
}

/**
 * @brief get a list of available matching algorithms
 * @ingroup fwd_module_management
 *
 * @param of_version
 * @param name_list
 * @param count
 * @return
 */
afa_result_t fwd_module_list_matching_algorithms(of_version_t of_version, const char * const** name_list, int *count){
	return of_get_switch_matching_algorithms(of_version, name_list, count);
}
