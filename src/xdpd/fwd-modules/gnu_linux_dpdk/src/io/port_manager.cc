#include "port_manager.h"
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/physical_switch.h>

/**
* Discovers and initializes (including rofl-pipeline state) DPDK-enabled ports.
*/
rofl_result_t port_manager_discover_system_ports(void){

	uint8_t i, num_of_ports;
	switch_port_t* port;
	struct rte_eth_dev_info dev_info;
	char port_name[SWITCH_PORT_MAX_LEN_NAME];
	num_of_ports = rte_eth_dev_count();
	
	for(i=0;i<num_of_ports;++i){
		
		//Get info
		rte_eth_dev_info_get (i, &dev_info);

		//Generate the interface name		
		//if(dev_info.link_speed == ETH_LINK_SPEED_10000)
		//	snprintf (port_name, SWITCH_PORT_MAX_LEN_NAME, "10ge-%u",i);
		//else if(dev_info.link_speed == ETH_LINK_SPEED_1000)
			snprintf (port_name, SWITCH_PORT_MAX_LEN_NAME, "ge-%u",i);
		//else
		//	snprintf (port_name, SWITCH_PORT_MAX_LEN_NAME, "fe-%u",i);

		port = switch_port_init(port_name, true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE);
		if(!port)
			return ROFL_FAILURE;
	
		//Start port
		if(rte_eth_dev_start(i) < 0)
			return ROFL_FAILURE;
		//Mark link status as automatic
	}	

	return ROFL_SUCCESS;
}


