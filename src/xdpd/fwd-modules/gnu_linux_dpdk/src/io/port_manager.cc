#include "port_manager.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/physical_switch.h>

#include "port_state.h"

#include "../config.h"
#include <rte_common.h> 
#include <rte_malloc.h> 

//Initializes the pipeline structure and launches the port 
static switch_port_t* configure_port(unsigned int port_id){

	switch_port_t* port;
	struct rte_eth_dev_info dev_info;
	char port_name[SWITCH_PORT_MAX_LEN_NAME];
	
	//Get info
	rte_eth_dev_info_get(port_id, &dev_info);

	//Hack to "deduce" the maximum speed of the NIC.
	//As of DPDK v1.4 there is not way to retreive such features from
	//the NIC
	if( strncmp(dev_info.driver_name, "rte_ixgbe", 9) == 0 ){
		/* 10G */
		snprintf (port_name, SWITCH_PORT_MAX_LEN_NAME, "10ge-%u",port_id);
	}else{
		/* 1G */
		snprintf (port_name, SWITCH_PORT_MAX_LEN_NAME, "ge-%u",port_id);
	}

	//Initialize pipeline port
	port = switch_port_init(port_name, true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE);
	if(!port)
		return NULL; 

	//Generate port state
	dpdk_port_state_t* ps = (dpdk_port_state_t*)rte_malloc(NULL,sizeof(dpdk_port_state_t),0);
	
	if(!ps){
		switch_port_destroy(port);
		return NULL;
	}

	//Start port
	if(rte_eth_dev_start(port_id) < 0)
		return NULL; 

	//Set promiscuous mode
	rte_eth_promiscuous_enable(port_id);	

	//Mark link status as automatic
	//TODO	

	//Fill-in dpdk port state
	ps->scheduled = false;
	ps->port_id = port_id;
	port->platform_port_state = (platform_port_state_t*)ps;

	ROFL_INFO("Discovered port %s [%u:%u:%u]\n", port_name, dev_info.pci_dev->addr.domain, dev_info.pci_dev->addr.bus, dev_info.pci_dev->addr.devid );

	return port;
}

/*
* Discovers and initializes (including rofl-pipeline state) DPDK-enabled ports.
*/
rofl_result_t port_manager_discover_system_ports(void){

	uint8_t i, num_of_ports;
	switch_port_t* port;
	num_of_ports = rte_eth_dev_count();
	
	ROFL_INFO("Found %u DPDK-capable interfaces\n", num_of_ports);
	
	for(i=0;i<num_of_ports;++i){
		if(! ( port = configure_port(i) ) ){
			ROFL_ERR("Unable to initialize port-id: %u\n", i);
			return ROFL_FAILURE;
		}

		//Add port to the pipeline
		if( physical_switch_add_port(port) != ROFL_SUCCESS ){
			ROFL_ERR("Unable to add the switch port to physical switch; perhaps there are no more physical port slots available?\n");
			return ROFL_FAILURE;
		}

	}	

	return ROFL_SUCCESS;
}

/*
* Shutdown all ports in the system 
*/
rofl_result_t port_manager_shutdown_ports(void){

	uint8_t i, num_of_ports;
	num_of_ports = rte_eth_dev_count();
	
	for(i=0;i<num_of_ports;++i){
		rte_eth_dev_stop(i);
	}	

	return ROFL_SUCCESS;
}

