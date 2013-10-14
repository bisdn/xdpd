#include <stdio.h>
#include <unistd.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/afa/cmm.h>
#include <rofl/datapath/pipeline/platform/memory.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>

//only for Test
#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "../config.h"
#include "../io/port_manager.h"
#include "../processing/processing.h"

//DPDK includes
#include <rte_common.h> 
#include <rte_eal.h> 
#include <rte_launch.h> 
#include <rte_mempool.h> 
#include <rte_mbuf.h> 
#include <rte_ethdev.h> 

extern int optind; 
struct rte_mempool *pool_direct = NULL, *pool_indirect = NULL;

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

	if (pool_indirect == NULL)
		rte_panic("Cannot init indirect mbuf pool\n");

	//Initialize pipeline
	if(physical_switch_init() != ROFL_SUCCESS)
		return AFA_FAILURE;

	//Discover and initialize rofl-pipeline state
	if(port_manager_discover_system_ports() != ROFL_SUCCESS)
		return AFA_FAILURE;

	//Initialize processing
	if(processing_init() != ROFL_SUCCESS)
		return AFA_FAILURE;
	
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

	//Destroy pipeline platform state
	physical_switch_destroy();

	//Shutdown ports
	
	
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

	//In software switches, you may have to launch threads that
	//do the pipeline processing of the packets
	
	//If you would fully use ROFL-pipeline you woudl call then
	//physical_switch_add_logical_switch(sw);
	
	return sw;
}

/*
* @name    fwd_module_get_switch_by_dpid 
* @brief   Retrieve the switch with the specified dpid  
* @ingroup logical_switch_management
* @retval  Pointer to of_switch_t instance or NULL 
*/
of_switch_t* fwd_module_get_switch_by_dpid(uint64_t dpid){
	
	ROFL_INFO("["FWD_MOD_NAME"] calling get_switch_by_dpid()\n");
	
	//Call directly the bank
	//If using the ROFL-pipeline library you would likely call
	//physical_switch_get_logical_switch_by_dpid(dpid); 
	
	return NULL;
}

/*
* @name    fwd_module_destroy_switch_by_dpid 
* @brief   Instructs the driver to destroy the switch with the specified dpid 
* @ingroup logical_switch_management
*/
afa_result_t fwd_module_destroy_switch_by_dpid(const uint64_t dpid){

	ROFL_INFO("["FWD_MOD_NAME"] calling destroy_switch_by_dpid()\n");
	
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
	
	ROFL_INFO("["FWD_MOD_NAME"] calling get_port_by_name()\n");
	
	return NULL;
}

/*
* @name    fwd_module_get_physical_ports_ports
* @brief   Retrieve the list of the physical ports of the switch
* @ingroup port_management
* @retval  Pointer to the first port. 
*/
switch_port_t** fwd_module_get_physical_ports(unsigned int* num_of_ports){
	
	ROFL_INFO("["FWD_MOD_NAME"] calling get_physical_ports()\n");

	return NULL;
}

/*
* @name    fwd_module_get_virtual_ports
* @brief   Retrieve the list of virtual ports of the platform
* @ingroup port_management
* @retval  Pointer to the first port. 
*/
switch_port_t** fwd_module_get_virtual_ports(unsigned int* num_of_ports){
	
	ROFL_INFO("["FWD_MOD_NAME"] calling get_virtual_ports()\n");
	
	return NULL;
}

/*
* @name    fwd_module_get_tunnel_ports
* @brief   Retrieve the list of tunnel ports of the platform
* @ingroup port_management
* @retval  Pointer to the first port. 
*/
switch_port_t** fwd_module_get_tunnel_ports(unsigned int* num_of_ports){
	
	ROFL_INFO("["FWD_MOD_NAME"] calling get_tunnel_ports()\n");
	
	return NULL;
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

	ROFL_INFO("["FWD_MOD_NAME"] calling attach_port_to_switch()\n");
	
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

	ROFL_INFO("["FWD_MOD_NAME"] calling detach_port_from_switch()\n");

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

	ROFL_INFO("["FWD_MOD_NAME"] calling detach_port_from_switch_at_port_num()\n");
	
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

	ROFL_INFO("["FWD_MOD_NAME"] calling enable_port()\n");
	
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

	ROFL_INFO("["FWD_MOD_NAME"] calling enable_port_by_num()\n");
	
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

	ROFL_INFO("["FWD_MOD_NAME"] calling disable_port_by_num()\n");
	
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

	if(of_get_switch_matching_algorithms(of_version, name_list, count) == ROFL_SUCCESS)
		return AFA_SUCCESS;

	return AFA_FAILURE;

}
