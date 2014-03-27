#define __STDC_LIMIT_MACROS
#include <stdio.h>
#include <unistd.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/afa/cmm.h>
#include <rofl/datapath/pipeline/platform/memory.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>

#include "../config.h"

//DPDK includes
#include <rte_config.h> 
#include <rte_common.h> 
#include <rte_eal.h> 
#include <rte_errno.h> 
#include <rte_launch.h> 
#include <rte_mempool.h> 
#include <rte_mbuf.h> 
#include <rte_ethdev.h> 

//only for Test
#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "../bg_taskmanager.h"
#include "../io/bufferpool.h"
#include "../io/port_manager.h"
#include "../io/pktin_dispatcher.h"
#include "../processing/processing.h"

extern int optind; 
struct rte_mempool *pool_direct = NULL, *pool_indirect = NULL;

using namespace xdpd::gnu_linux;

//Some useful macros
#define STR(a) #a
#define XSTR(a) STR(a)

#define GNU_LINUX_DPDK_CODE_NAME "gnu-linux-dpdk"
#define GNU_LINUX_DPDK_VERSION VERSION 
#define GNU_LINUX_DPDK_DESC \
"GNU/Linux DPDK driver. TODO: improve"
#define GNU_LINUX_DPDK_USAGE  "" //We don't support extra params
#define GNU_LINUX_DPDK_EXTRA_PARAMS "" //We don't support extra params


/*
* @name    fwd_module_init
* @brief   Initializes driver. Before using the AFA_DRIVER routines, higher layers must allow driver to initialize itself
* @ingroup fwd_module_management
*/

#define EAL_ARGS 6

afa_result_t fwd_module_init(const char* extra_params){

	int ret;
	const char* argv_fake[EAL_ARGS] = {"xdpd", "-c", XSTR(RTE_CORE_MASK), "-n", XSTR(RTE_MEM_CHANNELS), NULL};
	
	
	ROFL_INFO(FWD_MOD_NAME" Initializing...\n");
	
        /* init EAL library */
	optind=1;
	ret = rte_eal_init(EAL_ARGS-1, (char**)argv_fake);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "rte_eal_init failed");
	optind=1;

	/* create the mbuf pools */
	pool_direct = rte_mempool_create("pool_direct", NB_MBUF,
			MBUF_SIZE, 32,
			sizeof(struct rte_pktmbuf_pool_private),
			rte_pktmbuf_pool_init, NULL,
			rte_pktmbuf_init, NULL,
			SOCKET0, 0);

	if (pool_direct == NULL)
		rte_panic("Cannot init direct mbuf pool\n");


	pool_indirect = rte_mempool_create("pool_indirect", NB_MBUF,
			sizeof(struct rte_mbuf), 32,
			0,
			NULL, NULL,
			rte_pktmbuf_init, NULL,
			SOCKET0, 0);
	
	/* init driver(s) */
	if (rte_pmd_init_all() < 0)
		rte_exit(EXIT_FAILURE, "Cannot init pmd\n");
	if (rte_eal_pci_probe() < 0)
		rte_exit(EXIT_FAILURE, "Cannot probe PCI\n");

	if (pool_indirect == NULL)
		rte_panic("Cannot init indirect mbuf pool\n");

	//Init bufferpool
	bufferpool::init(IO_BUFFERPOOL_RESERVOIR);

	//Initialize pipeline
	if(physical_switch_init() != ROFL_SUCCESS)
		return AFA_FAILURE;

	//Discover and initialize rofl-pipeline state
	if(port_manager_discover_system_ports() != ROFL_SUCCESS)
		return AFA_FAILURE;

	//Initialize processing
	if(processing_init() != ROFL_SUCCESS)
		return AFA_FAILURE;

	//Initialize PKT_IN
	if(pktin_dispatcher_init() != ROFL_SUCCESS)
		return AFA_FAILURE;

	//Initialize Background Tasks Manager
	if(launch_background_tasks_manager() != ROFL_SUCCESS){
		return AFA_FAILURE;
	}

	return AFA_SUCCESS; 
}

/**
* @name    fwd_module_get_info
* @brief   Get the information of the forwarding_module (code-name, version, usage...)
* @ingroup fwd_module_management
*/
void fwd_module_get_info(fwd_module_info_t* info){
	//Fill-in fwd_module_info_t
	strncpy(info->code_name, GNU_LINUX_DPDK_CODE_NAME, FWD_MOD_CODE_NAME_MAX_LEN);
	strncpy(info->version, GNU_LINUX_DPDK_VERSION, FWD_MOD_VERSION_MAX_LEN);
	strncpy(info->description, GNU_LINUX_DPDK_DESC, FWD_MOD_DESCRIPTION_MAX_LEN);
	strncpy(info->usage, GNU_LINUX_DPDK_USAGE, FWD_MOD_USAGE_MAX_LEN);
	strncpy(info->extra_params, GNU_LINUX_DPDK_EXTRA_PARAMS, FWD_MOD_EXTRA_PARAMS_MAX_LEN);
}

/*
* @name    fwd_module_destroy
* @brief   Destroy driver state. Allows platform state to be properly released. 
* @ingroup fwd_module_management
*/
afa_result_t fwd_module_destroy(){

	ROFL_INFO(FWD_MOD_NAME" Destroying...\n");
	
	//Cleanup processing. This must be the first thing to do
	processing_destroy();

	//Initialize PKT_IN
	pktin_dispatcher_destroy();

	//Stop the bg manager
	stop_background_tasks_manager();

	//Destroy pipeline platform state
	physical_switch_destroy();

	// destroy bufferpool
	bufferpool::destroy();
	
	//Shutdown ports
	port_manager_destroy();

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
	
	of_switch_t* sw;
	
	ROFL_INFO(FWD_MOD_NAME" Creating switch. Name: %s, number of tables: %d\n",name, num_of_tables);
	
	sw = (of_switch_t*)of1x_init_switch(name, of_version, dpid, num_of_tables, (enum of1x_matching_algorithm_available*) ma_list);

	if(!sw)
		return AFA_FAILURE;

	//If you would fully use ROFL-pipeline you woudl call then
	physical_switch_add_logical_switch(sw);
	
	return AFA_SUCCESS;
}


/*
* @name    fwd_module_destroy_switch_by_dpid 
* @brief   Instructs the driver to destroy the switch with the specified dpid 
* @ingroup logical_switch_management
*/
afa_result_t fwd_module_destroy_switch_by_dpid(const uint64_t dpid){

	unsigned int i;
	
	//Try to retrieve the switch
	of_switch_t* sw = physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(!sw)
		return AFA_FAILURE;

	//Desechedule all ports. Do not feed more packets to the switch
	for(i=0;i<sw->max_ports;i++){

		if(sw->logical_ports[i].attachment_state == LOGICAL_PORT_STATE_ATTACHED && sw->logical_ports[i].port)
	
			processing_deschedule_port(sw->logical_ports[i].port);
	}	


	//Make sure all buffered PKT_INs are dropped
	wait_pktin_draining(sw);

	//Detach ports from switch. 
	if(physical_switch_detach_all_ports_from_logical_switch(sw)!=ROFL_SUCCESS)
		return AFA_FAILURE;

	//Remove switch from the switch bank
	if(physical_switch_remove_logical_switch(sw)!=ROFL_SUCCESS)
		return AFA_FAILURE;
	
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

/**
 * @name fwd_module_get_port_by_num
 * @brief Retrieves a snapshot of the current state of the port of the Logical Switch Instance with dpid at port_num, if exists. The snapshot MUST be deleted using switch_port_destroy_snapshot()
 * @ingroup port_management
 * @param dpid DatapathID 
 * @param port_num Port number
 */
switch_port_snapshot_t* fwd_module_get_port_snapshot_by_num(uint64_t dpid, unsigned int port_num){
	
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
	switch_port_snapshot_t* port_snapshot;
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

	ROFL_INFO(FWD_MOD_NAME" Trying to attach port %s. to switch %s (0x%llx)\n", port->name, lsw->name, (long long unsigned int)lsw->dpid);
	
	//Update pipeline state
	if(*of_port_num == 0){
		//no port specified, we assign the first available
		if(physical_switch_attach_port_to_logical_switch(port,lsw,of_port_num) == ROFL_FAILURE){
			assert(0);
			return AFA_FAILURE;
		}
	}else{

		if(physical_switch_attach_port_to_logical_switch_at_port_num(port,lsw,*of_port_num) == ROFL_FAILURE){
			assert(0);
			return AFA_FAILURE;
		}
	}

	//Schedule the port in the I/O subsystem
	if(processing_schedule_port(port) != ROFL_SUCCESS){
		assert(0);
		return AFA_FAILURE;
	}		

	//notify port attached
	port_snapshot = physical_switch_get_port_snapshot(port->name); 
	if(cmm_notify_port_add(port_snapshot)!=AFA_SUCCESS){
		//return AFA_FAILURE; //Ignore
	}
	
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

	//TODO: implemented
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
	of_switch_t* lsw;
	switch_port_t* port;
	switch_port_snapshot_t *port_snapshot=NULL, *port_pair_snapshot=NULL;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return AFA_FAILURE;

	port = physical_switch_get_port_by_name(name);

	//Check if the port does exist and is really attached to the dpid
	if( !port || !port->attached_sw || port->attached_sw->dpid != dpid)
		return AFA_FAILURE;

	//Snapshoting the port *before* it is detached 
	port_snapshot = physical_switch_get_port_snapshot(port->name); 
	
	//TODO: vlink ports
	//Deschedule port
	processing_deschedule_port(port);
	
	//Detach it
	if(physical_switch_detach_port_from_logical_switch(port,lsw) != ROFL_SUCCESS){
		ROFL_ERR(FWD_MOD_NAME" Error detaching port %s.\n",port->name);
		assert(0);
		goto FWD_MODULE_DETACH_ERROR;	
	}
	
	//notify port dettached
	cmm_notify_port_delete(port_snapshot);

	return AFA_SUCCESS; 

FWD_MODULE_DETACH_ERROR:
	if(port_snapshot)
		switch_port_destroy_snapshot(port_snapshot);	
	if(port_pair_snapshot)
		switch_port_destroy_snapshot(port_pair_snapshot);	

	return AFA_FAILURE; 
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
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return AFA_FAILURE;

	//Check if the port does exist.
	if(!of_port_num || of_port_num >= LOGICAL_SWITCH_MAX_LOG_PORTS || !lsw->logical_ports[of_port_num].port)
		return AFA_FAILURE;

	return fwd_module_detach_port_from_switch(dpid, lsw->logical_ports[of_port_num].port->name);
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

	switch_port_t* port;
	switch_port_snapshot_t* port_snapshot;

	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);

	if(!port || !port->platform_port_state)
		return AFA_FAILURE;

	//Bring it up
	port_manager_enable(port);

	port_snapshot = physical_switch_get_port_snapshot(port->name); 
	cmm_notify_port_status_changed(port_snapshot);
	
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

	switch_port_t* port;
	switch_port_snapshot_t* port_snapshot;
	
	//Check if the port does exist
	port = physical_switch_get_port_by_name(name);
	if(!port || !port->platform_port_state)
		return AFA_FAILURE;

	//Bring it down
	port_manager_disable(port);

	port_snapshot = physical_switch_get_port_snapshot(port->name); 
	cmm_notify_port_status_changed(port_snapshot);
	
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

	of_switch_t* lsw;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return AFA_FAILURE;

	//Check if the port does exist and is really attached to the dpid
	if( !lsw->logical_ports[port_num].port || lsw->logical_ports[port_num].attachment_state != LOGICAL_PORT_STATE_ATTACHED || lsw->logical_ports[port_num].port->attached_sw->dpid != dpid)
		return AFA_FAILURE;

	return fwd_module_bring_port_up(lsw->logical_ports[port_num].port->name);
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

	of_switch_t* lsw;
	
	lsw = physical_switch_get_logical_switch_by_dpid(dpid);
	if(!lsw)
		return AFA_FAILURE;

	//Check if the port does exist and is really attached to the dpid
	if( !lsw->logical_ports[port_num].port || lsw->logical_ports[port_num].attachment_state != LOGICAL_PORT_STATE_ATTACHED || lsw->logical_ports[port_num].port->attached_sw->dpid != dpid)
		return AFA_FAILURE;

	return fwd_module_bring_port_down(lsw->logical_ports[port_num].port->name);
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
	return (afa_result_t)of_get_switch_matching_algorithms(of_version, name_list, count);
}
