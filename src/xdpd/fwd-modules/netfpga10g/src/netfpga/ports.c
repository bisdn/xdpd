#include "ports.h"
#include <stdio.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/switch_port.h>

#define FWD_MOD_NAME "netfpga10g"

//Discover ports and put them in the pipeline
rofl_result_t netfpga_discover_ports_and_attach(of_switch_t* sw){
	
	unsigned int i, of_port_num;
	switch_port_t* port;
	char iface_name[NETFPGA_INTERFACE_NAME_LEN] = "0"; //nfX\0

	//Just add stuff
	for(i=0; i< NETFPGA_NUM_PORTS; ++i){
		//Compose name nf0...nf3
		snprintf(iface_name, NETFPGA_INTERFACE_NAME_LEN, NETFPGA_INTERFACE_BASE_NAME"%d", i);
		
		ROFL_DEBUG("["FWD_MOD_NAME"] Attempting to discover and attach %s\n", iface_name);
	
		//FIXME: interfaces should be anyway checked, and set link up.. but anyway. First implementation	
		port = switch_port_init(iface_name, true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE);
	
		if(physical_switch_attach_port_to_logical_switch(port, sw, &of_port_num) == ROFL_FAILURE)
			return ROFL_FAILURE;
		
	}

	return ROFL_SUCCESS;		
}
