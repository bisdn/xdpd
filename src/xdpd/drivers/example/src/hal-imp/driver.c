#include <stdio.h>
#include <net/if.h>
#include <rofl/datapath/hal/driver.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/hal/cmm.h>
#include <rofl/datapath/pipeline/platform/memory.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>

//only for Test
#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>

//Driver static info
#define EXAMPLE_CODE_NAME "example"
#define EXAMPLE_VERSION VERSION 
#define EXAMPLE_DESC  "Example driver. This is an empty non-functional driver that can be used as a blank canvas to port new platforms."
#define EXAMPLE_USAGE  "use format: \"key1=value1;key2=value2;\"\n\t\t\t\t\t- key1: explanation of key1\n\t\t\t\t\t- key2: explanation of key2" //Do not terminate it with \n

//Here we are going to store the parameters that we got
//and that we were able to parse (understand) and execute
char extra_params[DRIVER_EXTRA_PARAMS_MAX_LEN];


/*
* @name    hal_driver_init
* @brief   Initializes driver. Before using the HAL_DRIVER routines, higher layers must allow driver to initialize itself
* @ingroup driver_management
*/
hal_result_t hal_driver_init(const char* _extra_params){

	
	ROFL_INFO("["DRIVER_NAME"] calling hal_driver_init()\n");
	
	//If using ROFL-PIPELINE, the physical switch must be inited
	//if(physical_switch_init() != ROFL_SUCCESS)
	//	return HAL_FAILURE;

	//Likely here you are going to discover platform ports;
	//If using ROFL_pipeline you would then add them (physical_switch_add_port()

	//Initialize some form of background task manager
	
	//And initialize or setup any other state your platform needs...	
	
	ROFL_INFO("["DRIVER_NAME"] This driver is an empty example. xDPd will not be able to bootstrap any configuration (LSIs) since it is not a functional driver, so it will throw a deliverately uncaught exception\n");
	
	strncpy(extra_params, _extra_params, DRIVER_EXTRA_PARAMS_MAX_LEN);
	return HAL_SUCCESS; 
}

/**
* @name    hal_driver_get_info
* @brief   Get the information of the driver (code-name, version, usage...)
* @ingroup driver_management
*/
void hal_driver_get_info(driver_info_t* info){
	//Fill-in driver_info_t
	strncpy(info->code_name, EXAMPLE_CODE_NAME, DRIVER_CODE_NAME_MAX_LEN);
	strncpy(info->version, EXAMPLE_VERSION, DRIVER_VERSION_MAX_LEN);
	strncpy(info->description, EXAMPLE_DESC, DRIVER_DESCRIPTION_MAX_LEN);
	strncpy(info->usage, EXAMPLE_USAGE, DRIVER_USAGE_MAX_LEN);
	strncpy(info->extra_params, extra_params, DRIVER_EXTRA_PARAMS_MAX_LEN);
}


/*
* @name    hal_driver_destroy
* @brief   Destroy driver state. Allows platform state to be properly released. 
* @ingroup driver_management
*/
hal_result_t hal_driver_destroy(){

	//In this function you allow the platform
	//to be properly cleaning its own state

	//If using the pipeline you should call
	//physical_switch_destroy();

	ROFL_INFO("["DRIVER_NAME"] calling hal_driver_destroy()\n");
	
	return HAL_SUCCESS; 
}

/*
* Switch management functions
*/
bool hal_driver_switch_exists(uint64_t dpid){
	return NULL;
}

/**
* @brief   Retrieve the list of LSIs dpids
* @ingroup logical_switch_management
* @retval  List of available dpids, which MUST be deleted using dpid_list_destroy().
*/
dpid_list_t* hal_driver_get_all_lsi_dpids(void){
	return NULL;
}

/**
 * @name hal_driver_get_switch_snapshot_by_dpid 
 * @brief Retrieves a snapshot of the current state of a switch port, if the port name is found. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup logical_switch_management
 * @retval  Pointer to of_switch_snapshot_t instance or NULL 
 */
of_switch_snapshot_t* hal_driver_get_switch_snapshot_by_dpid(uint64_t dpid){
	return NULL;
}


/*
* @name    hal_driver_create_switch 
* @brief   Instruct driver to create an OF logical switch 
* @ingroup logical_switch_management
* @retval  Pointer to of_switch_t instance 
*/
hal_result_t hal_driver_create_switch(char* name, uint64_t dpid, of_version_t of_version, unsigned int num_of_tables, int* ma_list){
	
	ROFL_INFO("["DRIVER_NAME"] calling create switch. Name: %s, number of tables: %d\n",name, num_of_tables);
	
	return HAL_SUCCESS;
}

/*
* @name    hal_driver_destroy_switch_by_dpid 
* @brief   Instructs the driver to destroy the switch with the specified dpid 
* @ingroup logical_switch_management
*/
hal_result_t hal_driver_destroy_switch_by_dpid(const uint64_t dpid){

	ROFL_INFO("["DRIVER_NAME"] calling destroy_switch_by_dpid()\n");
	
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
	return false;
}

/**
* @brief   Retrieve the list of names of the available ports of the platform. You may want to 
* 	   call hal_driver_get_port_snapshot_by_name(name) to get more information of the port 
* @ingroup port_management
* @retval  List of available port names, which MUST be deleted using switch_port_name_list_destroy().
*/
switch_port_name_list_t* hal_driver_get_all_port_names(void){
	return NULL;
}

/**
 * @name hal_driver_get_port_by_name
 * @brief Retrieves a snapshot of the current state of a switch port, if the port name is found. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup port_management
 */
switch_port_snapshot_t* hal_driver_get_port_snapshot_by_name(const char *name){
	return NULL;
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

	ROFL_INFO("["DRIVER_NAME"] calling attach_port_to_switch()\n");
	
	return HAL_SUCCESS;
}

/**
* @name    hal_driver_connect_switches
* @brief   Attemps to connect two logical switches via a virtual port. Driver may or may not support this functionality. 
* @ingroup management
*
* @param dpid_lsi1 Datapath ID of the LSI1
* @param dpid_lsi2 Datapath ID of the LSI2 
*/
hal_result_t hal_driver_connect_switches(uint64_t dpid_lsi1, switch_port_snapshot_t** port1, uint64_t dpid_lsi2, switch_port_snapshot_t** port2){
	
	ROFL_INFO("["DRIVER_NAME"] calling connect_switches()\n");
	
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

	ROFL_INFO("["DRIVER_NAME"] calling detach_port_from_switch()\n");

	return HAL_SUCCESS; 
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

	ROFL_INFO("["DRIVER_NAME"] calling detach_port_from_switch_at_port_num()\n");
	
	return HAL_SUCCESS;
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

	ROFL_INFO("["DRIVER_NAME"] calling bring_port_up()\n");
	
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

	ROFL_INFO("["DRIVER_NAME"] calling bring_port_down()\n");
	
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

	ROFL_INFO("["DRIVER_NAME"] calling bring_port_up_by_num()\n");
	
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

	ROFL_INFO("["DRIVER_NAME"] calling bring_port_down_by_num()\n");
	
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

	if(of_get_switch_matching_algorithms(of_version, name_list, count) == ROFL_SUCCESS)
		return HAL_SUCCESS;
	return HAL_FAILURE;
}
