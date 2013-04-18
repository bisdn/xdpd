#include <linux/ethtool.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <linux/sockios.h>
//#include <netpacket/packet.h>
#include <netinet/in.h>
//#include <net/if.h>
#include <stdio.h>
#include <unistd.h>

//Prototypes
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/platform/memory.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include "iface_utils.h" 

#include "ports/ioport.h" 
#include "ports/mmap/ioport_mmap.h" 

/*
*
* Port management
*
* All of the functions related to physical port management 
*
*/

/*
* Physical port disovery
*/
static afa_result_t fill_port_admin_and_link_state(switch_port_t* port){

	struct ifreq ifr;
	int sd, rc;

	if ((sd = socket(AF_PACKET, SOCK_RAW, 0)) < 0) {
		return AFA_FAILURE;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, port->name);

	if ((rc = ioctl(sd, SIOCGIFINDEX, &ifr)) < 0) {
		return AFA_FAILURE;
	}

	if ((rc = ioctl(sd, SIOCGIFFLAGS, &ifr)) < 0) {
		close(sd);
		return AFA_FAILURE;
	}

	//Fill values
	port->up = (IFF_UP & ifr.ifr_flags) > 0;

	if( (IFF_RUNNING & ifr.ifr_flags) > 0){
	}else{
		port->state = PORT_STATE_LINK_DOWN; 
	}

	//Close socket	
	close(sd);
	return AFA_SUCCESS;
}

static void fill_port_speeds_capabilities(switch_port_t* port, struct ethtool_cmd* edata){

	uint64_t port_capabilities=0x0;
	uint64_t current_speed=0;

	//Get speed	
	uint32_t speed = ethtool_cmd_speed(edata);

	if(speed >= 10 && edata->duplex == DUPLEX_FULL){
		port_capabilities |= PORT_FEATURE_10MB_FD;
		current_speed = PORT_FEATURE_10MB_FD; 
	}else if (speed >= 10 && edata->duplex == DUPLEX_HALF){
		port_capabilities |= PORT_FEATURE_10MB_HD;
		current_speed = PORT_FEATURE_10MB_HD; 
	}
	
	if(speed >= 100 && edata->duplex == DUPLEX_FULL){
		port_capabilities |= PORT_FEATURE_100MB_FD;
		current_speed = PORT_FEATURE_100MB_FD; 
	}else if (speed >= 100 && edata->duplex == DUPLEX_HALF){
		port_capabilities |= PORT_FEATURE_100MB_HD;
		current_speed = PORT_FEATURE_100MB_HD; 
	}
	
	if(speed >= 1000 && edata->duplex == DUPLEX_FULL){
		port_capabilities |= PORT_FEATURE_1GB_FD;
		current_speed = PORT_FEATURE_1GB_FD; 
	}else if (speed >= 1000 && edata->duplex == DUPLEX_HALF){
		port_capabilities |= PORT_FEATURE_1GB_HD;
		current_speed = PORT_FEATURE_1GB_HD; 
	}

	if(speed >= 10000 && edata->duplex == DUPLEX_FULL){
		port_capabilities |= PORT_FEATURE_10GB_FD;
		current_speed = PORT_FEATURE_10GB_FD; 
	}

	//TODO: properly deduce speeds
	//Filling only with the deduced speed
	switch_port_add_capabilities(&port->curr, (port_features_t)port_capabilities);	
	switch_port_add_capabilities(&port->advertised, (port_features_t)port_capabilities);	
	switch_port_add_capabilities(&port->supported, (port_features_t)port_capabilities);	
	switch_port_add_capabilities(&port->peer, (port_features_t)port_capabilities);	

	//Filling speeds
	switch_port_set_current_speed(port, (port_features_t)current_speed);
	switch_port_set_current_max_speed(port, (port_features_t)current_speed); //TODO: this is not right
}

static switch_port_t* fill_port(int sock, struct ifaddrs* ifa){
	
	int j;
	struct ethtool_cmd edata;
	struct ifreq ifr;
	struct sockaddr_ll *socll;
	switch_port_t* port;	

	//fetch interface info with ethtool
	strcpy(ifr.ifr_name, ifa->ifa_name);
	memset(&edata,0,sizeof(edata));
	edata.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (char *) &edata;

	if (ioctl(sock, SIOCETHTOOL, &ifr)==-1){
		//FIXME change this messages into warnings "Unable to discover mac address of interface %s"
		ROFL_ERR("<%s:%d> %s: ioctl SIOCETHTOOL",__func__,__LINE__,ifr.ifr_name);
	}
	
	//Init the port
	port = switch_port_init(ifa->ifa_name, true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE/*will be overriden afterwards*/);
	if(!port)
		return NULL;

	//get the MAC addr.
	socll = (struct sockaddr_ll *)ifa->ifa_addr;
	ROFL_DEBUG("<%s:%d> Iface %s mac_addr %02X:%02X:%02X:%02X:%02X:%02X \n",__func__,__LINE__,
		ifa->ifa_name,socll->sll_addr[0],socll->sll_addr[1],socll->sll_addr[2],socll->sll_addr[3],
		socll->sll_addr[4],socll->sll_addr[5]);

	for(j=0;j<6;j++)
		port->hwaddr[j] = socll->sll_addr[j];

	//Fill port admin/link state
	if( fill_port_admin_and_link_state(port) == AFA_FAILURE){
		if(strcmp(port->name,"lo") == 0) //Skip loopback but return success
			return port; 
			
		switch_port_destroy(port);
		return NULL;
	}
	
	//Fill speeds and capabilities	
	fill_port_speeds_capabilities(port, &edata);


	//Initialize MMAP-based port
	//Change this line to use another ioport...
	port->platform_port_state = (platform_port_state_t*)new ioport_mmap(port);
	
	return port;
}

/*
 * Looks in the system physical ports and fills up the switch_port_t sructure with them
 *
 */
afa_result_t discover_physical_ports(){
	physical_switch_t* psw;
	int index, sock;
	
	struct ifaddrs *ifaddr, *ifa;
	
	psw = get_physical_switch();
	
	/*real way to find interfaces*/
	//getifaddrs(&ifap); -> there are examples on how to get the ip addresses
	if (getifaddrs(&ifaddr) == -1){
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
	        perror("socket");
			freeifaddrs(ifaddr);
        	exit(EXIT_FAILURE);
    	}
	
	for(ifa = ifaddr; ifa != NULL; ifa=ifa->ifa_next){
		
		if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;
		
		//look for an empty slot
		for(index=0;index<PHYSICAL_SWITCH_MAX_NUM_PHY_PORTS;index++){
			if(psw->physical_ports[index] == NULL)
				break;
		}
		if(index==PHYSICAL_SWITCH_MAX_NUM_PHY_PORTS){
			ROFL_ERR("<%s:%d> All physical port slots are occupied\n",__func__, __LINE__);
			freeifaddrs(ifaddr);
			return AFA_FAILURE;
		}
	#if 0	
		//Slot found
		psw->physical_ports[index] = (switch_port_t*)cutil_malloc_shared(sizeof(switch_port_t));//NOTE this needs to be freed
		
		//fetch interface info with ethtool
		strcpy(ifr.ifr_name, ifa->ifa_name);
		memset(&edata,0,sizeof(edata));
		edata.cmd = ETHTOOL_GSET;
		ifr.ifr_data = (char *) &edata;

		if (ioctl(sock, SIOCETHTOOL, &ifr)==-1){
			//FIXME change this messages into warnings "Unable to discover mac address of interface %s"
			ROFL_ERR("<%s:%d> %s: ",__func__,__LINE__,ifr.ifr_name);
			perror("ioctl SIOCETHTOOL");
		}
	#endif	
		//here we call the function that fills the structure
		if( (psw->physical_ports[index] = fill_port(sock, ifa)) == NULL )
			return AFA_FAILURE;
	}
	
	freeifaddrs(ifaddr);
	
	return AFA_SUCCESS;
}

afa_result_t destroy_port(switch_port_t* port){
	
	ioport* ioport_instance;

	if(!port)
		return AFA_FAILURE;
	
	//Destroy the inner driver-dependant structures
	ioport_instance = (ioport*)port->platform_port_state;	
	delete ioport_instance;

	if(switch_port_destroy(port) == ROFL_FAILURE)
		return AFA_FAILURE;

	return AFA_SUCCESS;
}

/*
* Port attachement/detachment 
*/

/*
* Enable port (direct call to ioport)
*/
afa_result_t enable_port(platform_port_state_t* ioport_instance){

	ioport* port = (ioport*)ioport_instance; 

	if(port->enable() == ROFL_FAILURE)
		return AFA_FAILURE;

	return AFA_SUCCESS;
}

/*
* Disable port (direct call to ioport)
*/
afa_result_t disable_port(platform_port_state_t* ioport_instance){

	class ioport* port = (ioport*)ioport_instance; 

	if(port->disable() == ROFL_FAILURE)
		return AFA_FAILURE;

	return AFA_SUCCESS;
}


