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

/*
* @name    fwd_module_init
* @brief   Initializes driver. Before using the AFA_DRIVER routines, higher layers must allow driver to initialize itself
* @ingroup fwd_module_management
*/

#define EAL_ARGS 6

afa_result_t fwd_module_init(){

	int ret;
	const char* argv_fake[EAL_ARGS] = {"xdpd", "-c", XSTR(RTE_CORE_MASK), "-n", XSTR(RTE_MEM_CHANNELS), NULL};
	
	
	ROFL_INFO("["FWD_MOD_NAME"] calling fwd_mod_init()\n");
	
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

/*
* @name    fwd_module_destroy
* @brief   Destroy driver state. Allows platform state to be properly released. 
* @ingroup fwd_module_management
*/
afa_result_t fwd_module_destroy(){

	ROFL_INFO("["FWD_MOD_NAME"] calling fwd_mod_destroy()\n");
	
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
	port_manager_shutdown_ports();

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
	
	ROFL_INFO("["FWD_MOD_NAME"] calling create switch. Name: %s, number of tables: %d\n",name, num_of_tables);
	
	sw = (of_switch_t*)of1x_init_switch(name, of_version, dpid, num_of_tables, (enum of1x_matching_algorithm_available*) ma_list);

	if(!sw)
		return NULL; //Error

	//If you would fully use ROFL-pipeline you woudl call then
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
	
	return physical_switch_get_logical_switch_by_dpid(dpid); 
}

/*
* @name    fwd_module_destroy_switch_by_dpid 
* @brief   Instructs the driver to destroy the switch with the specified dpid 
* @ingroup logical_switch_management
*/
afa_result_t fwd_module_destroy_switch_by_dpid(const uint64_t dpid){

	//unsigned int i;
	
	//Try to retrieve the switch
	of_switch_t* sw = physical_switch_get_logical_switch_by_dpid(dpid);
	
	if(!sw)
		return AFA_FAILURE;

	//Desechedule all ports
	//XXX
	
	//Detach ports from switch. Do not feed more packets to the switch
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

/*
* @name    fwd_module_list_platform_ports
* @brief   Retrieve the list of ports of the platform 
* @ingroup port_management
* @retval  Pointer to the first port. 
*/
switch_port_t* fwd_module_list_platform_ports(){
	
	//TODO: This is never used
	
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

	ROFL_INFO("["FWD_MOD_NAME"] Trying to attach port %s. to switch %s (0x%llx)\n", port->name, lsw->name, (long long unsigned int)lsw->dpid);
	
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
	if(cmm_notify_port_add(port)!=AFA_SUCCESS){
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
afa_result_t fwd_module_connect_switches(uint64_t dpid_lsi1, switch_port_t** port1, uint64_t dpid_lsi2, switch_port_t** port2){

	ROFL_INFO("["FWD_MOD_NAME"] calling connect_switches()\n");
	//XXX
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

	ROFL_INFO("["FWD_MOD_NAME"] calling detach_port_from_switch()\n");
	//XXX

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

	ROFL_INFO("["FWD_MOD_NAME"] calling detach_port_from_switch_at_port_num()\n");
	
	//XXX
	return AFA_FAILURE;
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

	ROFL_INFO("["FWD_MOD_NAME"] calling enable_port()\n");
	
	//XXX
	//return AFA_FAILURE;
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

	ROFL_INFO("["FWD_MOD_NAME"] calling disable_port()\n");
	
	//XXX
	return AFA_FAILURE;
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

	ROFL_INFO("["FWD_MOD_NAME"] calling enable_port_by_num()\n");
	
	//XXX
	return AFA_FAILURE;
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

	ROFL_INFO("["FWD_MOD_NAME"] calling disable_port_by_num()\n");
	
	//XXX
	return AFA_FAILURE;
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
