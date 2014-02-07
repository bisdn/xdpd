#include <stdio.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/afa/cmm.h>
#include <rofl/datapath/pipeline/platform/memory.h>
#include <rofl/datapath/pipeline/physical_switch.h>

//only for Test
#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "../io/bufferpool.h"
#include "../bg_taskmanager.h"
#include "../netfpga/netfpga.h"
#include "../netfpga/ports.h"
#include "../config.h"

using namespace xdpd::gnu_linux;

//Static pipeline switch instance
static of_switch_t* sw=NULL;

/*
* @name    fwd_module_init
* @brief   Initializes driver. Before using the AFA_DRIVER routines, higher layers must allow driver to initialize itself
* @ingroup fwd_module_management
*/
afa_result_t fwd_module_init(){

	ROFL_INFO("["FWD_MOD_NAME"] calling fwd_mod_init()\n");
	ROFL_ERR("["FWD_MOD_NAME"] !!!!!!!!! WARNING: NetFPGA 10G forwarding module is experimental. Be advised. !!!!!!!!!\n");
	
	//If using ROFL-PIPELINE, the physical switch must be inited
	if(physical_switch_init() != ROFL_SUCCESS)
		return AFA_FAILURE;

	//Discover platform ports;
	if(netfpga_discover_ports() != ROFL_SUCCESS){
		ROFL_ERR("["FWD_MOD_NAME"] Unable to discover physical ports!\n");
		return AFA_FAILURE;	
	}

	//Init 10G NetFPGA
	if(netfpga_init() != ROFL_SUCCESS){
		ROFL_ERR("["FWD_MOD_NAME"] calling netfpga_init() failed!\n");
		//FIXME: Clear state and exit
		return AFA_FAILURE;	
	}

	//Init bufferpool
	bufferpool::init();

	//Initialize some form of background task manager
	launch_background_tasks_manager();
	
	//And initialize or setup any other state your platform needs...	
	
	return AFA_SUCCESS; 
}

/*
* @name    fwd_module_destroy
* @brief   Destroy driver state. Allows platform state to be properly released. 
* @ingroup fwd_module_management
*/
afa_result_t fwd_module_destroy(){

	//In this function you allow the platform
	//to be properly cleaning its own state
	stop_background_tasks_manager();

	//Gently destroy (release) 10G NetFPGA
	if(netfpga_destroy() != ROFL_SUCCESS){
		ROFL_DEBUG("["FWD_MOD_NAME"] calling netfpga_destroy() failed!\n");
	}

	//If using the pipeline you should call
	physical_switch_destroy();

	//Destroy bufferpool
	bufferpool::destroy();

	ROFL_INFO("["FWD_MOD_NAME"] calling fwd_mod_destroy()\n");
	
	return AFA_SUCCESS; 
}

/*
* Switch management functions
*/

/**
* @brief   Checks if an LSI with the specified dpid exists 
* @ingroup logical_switch_management
*/
bool fwd_module_switch_exists(uint64_t dpid){
	return physical_switch_get_logical_switch_by_dpid(dpid) != NULL;
}

/**
* @brief   Retrieve the list of LSIs dpids
* @ingroup logical_switch_management
* @retval  List of available dpids, which MUST be deleted using dpid_list_destroy().
*/
dpid_list_t* fwd_module_get_all_lsi_dpids(void){
	return physical_switch_get_all_lsi_dpids();  
}

/**
 * @name fwd_module_get_switch_snapshot_by_dpid 
 * @brief Retrieves a snapshot of the current state of a switch port, if the port name is found. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup logical_switch_management
 * @retval  Pointer to of_switch_snapshot_t instance or NULL 
 */
of_switch_snapshot_t* fwd_module_get_switch_snapshot_by_dpid(uint64_t dpid){
	return physical_switch_get_logical_switch_snapshot(dpid);
}



/*
* @name    fwd_module_create_switch 
* @brief   Instruct driver to create an OF logical switch 
* @ingroup logical_switch_management
* @retval  Pointer to of_switch_t instance 
*/
afa_result_t fwd_module_create_switch(char* name, uint64_t dpid, of_version_t of_version, unsigned int num_of_tables, int* ma_list){
	
	//We only accept one logical switch in this forwarding module
	if(sw){
		ROFL_ERR("["FWD_MOD_NAME"] ERROR: NetFPGA 10G forwarding module only supports 1 logical switch! Exiting...\n");
		exit(EXIT_FAILURE);
	}

	ROFL_INFO("["FWD_MOD_NAME"] calling create switch. Name: %s\n",name);

	if(num_of_tables > 1){
		ROFL_ERR("["FWD_MOD_NAME"] ERROR: NetFPGA 10G forwarding module only supports 1 table! Exiting...\n");
		exit(EXIT_FAILURE);
	}
	
	switch(of_version){
		case OF_VERSION_12: 
			sw = (of_switch_t*)of1x_init_switch(name, of_version, dpid, num_of_tables, (enum of1x_matching_algorithm_available*) ma_list);
			break;

		//Add more here..
			
		default: 
			return AFA_FAILURE;
	}	

	//Adding switch to the bank
	physical_switch_add_logical_switch(sw);

	if(netfpga_attach_ports(sw) != ROFL_SUCCESS){
		//Something went wrong. Abort all
		ROFL_ERR("["FWD_MOD_NAME"] NetFPGA ports could NOT be discovered... I must abort execution...\n");
		exit(EXIT_FAILURE);
		;
	}

	//Warn user
	ROFL_ERR("["FWD_MOD_NAME"] All NetFPGA physical ports are attached (nf0..nf3). Subsequent calls to attach_port will be silently ignored...\n");
	
	return AFA_SUCCESS;
}


/*
* @name    fwd_module_destroy_switch_by_dpid 
* @brief   Instructs the driver to destroy the switch with the specified dpid 
* @ingroup logical_switch_management
*/
afa_result_t fwd_module_destroy_switch_by_dpid(const uint64_t dpid){

	if(!sw)
		return AFA_FAILURE;

	if(sw->dpid != dpid)
		return AFA_FAILURE;
	
	ROFL_INFO("["FWD_MOD_NAME"] calling destroy_switch_by_dpid()\n");

	//XXX: do something with the hw
	
	//Detach ports from switch. Do not feed more packets to the switch
	if(physical_switch_detach_all_ports_from_logical_switch(sw)!=ROFL_SUCCESS)
		return AFA_FAILURE;
	
	//Remove switch from the switch bank
	if(physical_switch_remove_logical_switch(sw)!=ROFL_SUCCESS)
		return AFA_FAILURE;

	//Set pointer sw pointer so that it can be recreated in the future
	sw=NULL;	
	
	return AFA_SUCCESS;
}

/*
* Port management 
*/

/**
* @brief   Checks if a port with the specified name exists 
* @ingroup port_management 
*/
bool fwd_module_port_exists(const char *name){
	return physical_switch_get_port_by_name(name) != NULL; 
}

/**
* @brief   Retrieve the list of names of the available ports of the platform. You may want to 
* 	   call fwd_module_get_port_snapshot_by_name(name) to get more information of the port 
* @ingroup port_management
* @retval  List of available port names, which MUST be deleted using switch_port_name_list_destroy().
*/
switch_port_name_list_t* fwd_module_get_all_port_names(void){
	return physical_switch_get_all_port_names(); 
}

/**
 * @name fwd_module_get_port_by_name
 * @brief Retrieves a snapshot of the current state of a switch port, if the port name is found. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup port_management
 */
switch_port_snapshot_t* fwd_module_get_port_snapshot_by_name(const char *name){
	return physical_switch_get_port_snapshot(name); 
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
	//Skip
	return AFA_SUCCESS;
}

/**
* @name    fwd_module_connect_switches
* @brief   Attemps to connect two logical switches via a virtual port. Forwarding module may or may not support this functionality. 
* @ingroup management
*
* @param dpid_lsi1 Datapath ID of the LSI1
* @param dpid_lsi2 Datapath ID of the LSI2 
*/
afa_result_t fwd_module_connect_switches(uint64_t dpid_lsi1, switch_port_snapshot_t** port1, uint64_t dpid_lsi2, switch_port_snapshot_t** port2){
	return AFA_FAILURE;
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
	//Skip
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

	//Skip
	
	return AFA_SUCCESS;
}


//Port admin up/down stuff

/*
* Port administrative management actions (ifconfig up/down like)
*/

/*
* @name    fwd_module_bring_port_up
* @brief   Brings up a system port. If the port is attached to an OF logical switch, this also schedules port for I/O and triggers PORTMOD message. 
* @ingroup port_management
*
* @param name Port system name 
*/
afa_result_t fwd_module_bring_port_up(const char* name){

	ROFL_INFO("["FWD_MOD_NAME"] calling enable_port()\n");
	
	//FIXME: todo
	
	return AFA_SUCCESS;
}

/*
* @name    fwd_module_bring_port_down
* @brief   Shutdowns (brings down) a system port. If the port is attached to an OF logical switch, this also de-schedules port and triggers PORTMOD message. 
* @ingroup port_management
*
* @param name Port system name 
*/
afa_result_t fwd_module_bring_port_down(const char* name){

	ROFL_INFO("["FWD_MOD_NAME"] calling disable_port()\n");
	
	return AFA_SUCCESS;
}

/*
* @name    fwd_module_bring_port_up_by_num
* @brief   Brings up a port from an OF logical switch (and the underlying physical interface). This function also triggers the PORTMOD message 
* @ingroup port_management
*
* @param dpid DatapathID 
* @param port_num OF port number
*/
afa_result_t fwd_module_bring_port_up_by_num(uint64_t dpid, unsigned int port_num){

	ROFL_INFO("["FWD_MOD_NAME"] calling enable_port_by_num()\n");
	
	//FIXME: todo
	
	//return AFA_SUCCESS;
	return AFA_FAILURE;
}

/*
* @name    fwd_module_bring_port_down_by_num
* @brief   Brings down a port from an OF logical switch (and the underlying physical interface). This also triggers the PORTMOD message.
* @ingroup port_management
*
* @param dpid DatapathID 
* @param port_num OF port number
*/
afa_result_t fwd_module_bring_port_down_by_num(uint64_t dpid, unsigned int port_num){

	ROFL_INFO("["FWD_MOD_NAME"] calling disable_port_by_num()\n");
	
	//FIXME: todo
	
	//return AFA_SUCCESS;
	return AFA_FAILURE;
}

/**
 * @brief Retrieve a snapshot of the monitoring state. If rev is 0, or the current monitoring 
 * has changed (monitoring->rev != rev), a new snapshot of the monitoring state is made. Warning: this 
 * is expensive.
 * @ingroup fwd_module_management
 *
 * @param rev Last seen revision. Set to 0 to always get a new snapshot 
 * @return A snapshot of the monitoring state that MUST be destroyed using monitoring_destroy_snapshot() or NULL if there have been no changes (same rev)
 */ 
monitoring_snapshot_state_t* fwd_module_get_monitoring_snapshot(uint64_t rev){

	monitoring_state_t* mon = physical_switch_get_monitoring();

	if( rev == 0 || monitoring_has_changed(mon, &rev) ) 
		return monitoring_get_snapshot(mon);

	return NULL;
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
	if(of_get_switch_matching_algorithms(of_version, name_list, count) != ROFL_SUCCESS)
		return AFA_FAILURE;
	
	return AFA_SUCCESS;
}
