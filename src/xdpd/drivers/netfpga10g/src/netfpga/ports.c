#include "ports.h"
#include <stdio.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>


#include <net/if.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#include <asm/types.h>
#include <linux/sockios.h>
#include <unistd.h>
#include <errno.h>

#define FWD_MOD_NAME "netfpga10g"


rofl_result_t netfpga_destroy_port(switch_port_t* port){

	netfpga_port_t* nport =(netfpga_port_t*)port->platform_port_state ;

	pcap_close(nport->pcap_fd);

	if(nport){
		free(nport);
		return ROFL_SUCCESS;
		}
	else	{
		ROFL_DEBUG("netfpga_destroy_port() called without initialized netfpga port");
		return ROFL_FAILURE;
	
		}	
}

static rofl_result_t netfpga_init_port(switch_port_t* port){




	struct ifreq interface;
	netfpga_port_t* nport = (netfpga_port_t*)malloc(sizeof(*nport));

	
	char *useless;

	
	
		//fprintf(stderr, "device=  %s  \n",dev);
	useless = pcap_lookupdev(port->name); //test if device exist// gives char pointer, why not pcap_if_t?
	if (useless == NULL) {
		ROFL_ERR( "Couldn't find device: error= %s; no permission to listen on interface or other failure  \n", port->name);
		return ROFL_FAILURE;	
	}
	ROFL_DEBUG("Device :%s  found\n", port->name);

		
	char errbuf[PCAP_ERRBUF_SIZE];

	
	nport->pcap_fd = pcap_open_live(port->name, BUFSIZ, 1, 0, errbuf);//wait until the packet arrive, NO TIMEOUT
	if (nport->pcap_fd == NULL) {
		 ROFL_ERR( "Couldn't open device %s : %s\n",port->name, errbuf);
		 return ROFL_FAILURE;
	}

	nport->fd = pcap_get_selectable_fd(nport->pcap_fd);
	nport->test=25;	
	ROFL_DEBUG("pcap_open_live: socket opened \n ");
	



	ROFL_DEBUG("Ports.c creating socket over %s inerface\n", port->name);
	strncpy(interface.ifr_ifrn.ifrn_name, port->name, IFNAMSIZ/*&SWITCH_PORT_MAX_LEN_NAME*/);
	

	int flags;

	/* Set non-blocking mode. */
	flags = fcntl(nport->fd, F_GETFL, 0);
	if(fcntl(nport->fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		return ROFL_FAILURE;
	}

	//Store in platform state and return
	port->platform_port_state = (platform_port_state_t*) nport;	

	return ROFL_SUCCESS;
} 

//Discover ports and initialize, raw sockets and I/O thread
rofl_result_t netfpga_discover_ports(){
	unsigned int i;	
	switch_port_t* port;
	char iface_name[NETFPGA_INTERFACE_NAME_LEN] = "0"; //nfX\0

	//Just add stuff
	for(i=0; i< NETFPGA_NUM_PORTS; ++i){
		//Compose name nf0...nf3
		snprintf(iface_name, NETFPGA_INTERFACE_NAME_LEN, NETFPGA_INTERFACE_BASE_NAME"%d", i);
		
		ROFL_DEBUG("["FWD_MOD_NAME"] Attempting to discover %s\n", iface_name);

		//Initialize pipeline structure	
		port = switch_port_init(iface_name, true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE);

		//Init NetFPGA state (platform specific state)
		if(netfpga_init_port(port) != ROFL_SUCCESS) //checked
			return ROFL_FAILURE;
		
		//Add to available ports
		if( physical_switch_add_port(port) != ROFL_SUCCESS )// in rofl-core
			return ROFL_FAILURE;
		
	}


	return ROFL_SUCCESS;		
}

rofl_result_t netfpga_attach_ports(of_switch_t* sw){

	unsigned int i, of_port_num;
	switch_port_t* port;
	char iface_name[NETFPGA_INTERFACE_NAME_LEN] = "0"; //nfX\0

	//Just attach 
	for(i=0; i< NETFPGA_NUM_PORTS; ++i){
		//Compose name nf0...nf3
		snprintf(iface_name, NETFPGA_INTERFACE_NAME_LEN, NETFPGA_INTERFACE_BASE_NAME"%d", i);
		
		ROFL_DEBUG("["FWD_MOD_NAME"] Attempting to attach %s\n", iface_name);
	
		//FIXME: interfaces should be anyway checked, and set link up.. but anyway. First implementation	
		port = physical_switch_get_port_by_name(iface_name);
	
		//Do the attachment	
		if(physical_switch_attach_port_to_logical_switch(port, sw, &of_port_num) == ROFL_FAILURE)
			return ROFL_FAILURE;
		
	
	}
	return ROFL_SUCCESS;
}

