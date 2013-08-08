#include "packet_io.h"

#define FWD_MOD_NAME "netfpga10g"

void netpfga_io_read_from_port(switch_port_t* port){

	netfpga_port_t* state = (netfpga_port_t*)port->platform_port_state;

	ROFL_ERR("["FWD_MOD_NAME"] Got a packet from kernel (PKT_IN) in port %s!\n", port->name);

	//Retrieve an empty buffer

	//Non-blocking read of packets
	//read(state->fd, buffer, size);
	//classify it

	//Store the packet in storage	

	//Call CMM cmm_of12_packet_in();
	
	
}
