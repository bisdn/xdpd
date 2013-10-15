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
#include <rofl/datapath/afa/cmm.h>
#include "iface_utils.h" 
#include "iomanager.h"

#include "ports/ioport.h" 
#include "ports/mmap/ioport_mmap.h" 
#include "ports/mmap/ioport_mmapv2.h" 

/*
*
* Port management
*
* All of the functions related to physical port management 
*
*/

/**
 * @name update_port_status
 */
rofl_result_t update_port_status(char * name){
	int skfd;
	struct ethtool_value edata;
	struct ifreq ifr;
	switch_port_t *port;
	bool last_link_status;

	
	memset(&edata,0,sizeof(edata));//Make valgrind happy
	
	//Update all ports
	if(update_physical_ports() != ROFL_SUCCESS){
		ROFL_ERR("Update physical ports failed \n");
		assert(0);
	}

	port = fwd_module_get_port_by_name(name);
	
	//If there is no port after the update, just skip
	if(!port)
		return ROFL_FAILURE;
	
	if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 ){
		ROFL_ERR("Socket call error, errno(%d): %s\n", errno, strerror(errno));
		return ROFL_FAILURE;
	}
	
	edata.cmd = ETHTOOL_GLINK;
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name)-1);
	
	ifr.ifr_data = (char *) &edata;
	if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0){
		//This may fail if the interface has been deleted from the system. Just return	
		return ROFL_FAILURE;
	}

	ROFL_INFO("[bg] Interface %s link is %s\n", name,(edata.data ? "up" : "down"));
	
	last_link_status = port->up;

	if(edata.data){
		iomanager::bring_port_up((ioport*)port->platform_port_state);
	}else{
		iomanager::bring_port_down((ioport*)port->platform_port_state);
	}

	//port->forward_packets;
	/*more*/
	
	//port_status message needs to be created if the port id attached to switch
	if(port->attached_sw != NULL && last_link_status != port->up){
		cmm_notify_port_status_changed(port);
	}
	
	return ROFL_SUCCESS;
}

static void fill_port_queues(switch_port_t* port, ioport* io_port){

	unsigned int i;
	char queue_name[PORT_QUEUE_MAX_LEN_NAME];

	//Filling one-by-one the queues that ioport has	
	for(i=0;i<io_port->get_num_of_queues();i++){
		snprintf(queue_name, PORT_QUEUE_MAX_LEN_NAME, "%s%d", "queue", i);
		switch_port_add_queue(port, i, (char*)&queue_name, io_port->get_queue_size(i), 0, 0);
	}

}

/*
* Physical port disovery
*/
static rofl_result_t fill_port_admin_and_link_state(switch_port_t* port){

	struct ifreq ifr;
	int sd, rc;

	if ((sd = socket(AF_PACKET, SOCK_RAW, 0)) < 0) {
		return ROFL_FAILURE;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, port->name);

	if ((rc = ioctl(sd, SIOCGIFINDEX, &ifr)) < 0) {
		return ROFL_FAILURE;
	}

	if ((rc = ioctl(sd, SIOCGIFFLAGS, &ifr)) < 0) {
		close(sd);
		return ROFL_FAILURE;
	}

	//Fill values
	port->up = (IFF_UP & ifr.ifr_flags) > 0;

	if( (IFF_RUNNING & ifr.ifr_flags) > 0){
	}else{
		port->state = PORT_STATE_LINK_DOWN; 
	}

	//Close socket	
	close(sd);
	return ROFL_SUCCESS;
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
		ROFL_WARN("WARNING: unable to retrieve MAC address from iface %s via ioctl SIOCETHTOOL. Information will not be filled\n",ifr.ifr_name);
	}
	
	//Init the port
	port = switch_port_init(ifa->ifa_name, true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE/*will be overriden afterwards*/);
	if(!port)
		return NULL;

	//get the MAC addr.
	socll = (struct sockaddr_ll *)ifa->ifa_addr;
	ROFL_INFO("Discovered interface %s mac_addr %02X:%02X:%02X:%02X:%02X:%02X \n",
		ifa->ifa_name,socll->sll_addr[0],socll->sll_addr[1],socll->sll_addr[2],socll->sll_addr[3],
		socll->sll_addr[4],socll->sll_addr[5]);

	for(j=0;j<6;j++)
		port->hwaddr[j] = socll->sll_addr[j];

	//Fill port admin/link state
	if( fill_port_admin_and_link_state(port) == ROFL_FAILURE){
		if(strcmp(port->name,"lo") == 0) //Skip loopback but return success
			return port; 
			
		switch_port_destroy(port);
		return NULL;
	}
	
	//Fill speeds and capabilities	
	fill_port_speeds_capabilities(port, &edata);

	//Initialize MMAP-based port
	//Change this line to use another ioport...
	ioport* io_port = new ioport_mmapv2(port);
	//iport* io_port = new ioport_mmap(port);

	port->platform_port_state = (platform_port_state_t*)io_port;
	
	//Fill port queues
	fill_port_queues(port, (ioport*)port->platform_port_state);
	
	return port;
}

/*
 * Looks in the system physical ports and fills up the switch_port_t sructure with them
 *
 */
rofl_result_t discover_physical_ports(){
	
	unsigned int i, max_ports;
	switch_port_t* port, **array;
	int sock;
	
	struct ifaddrs *ifaddr, *ifa;
	
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

		//Fill port
		port = fill_port(sock, ifa);

		//Adding the 
		if( physical_switch_add_port(port) != ROFL_SUCCESS ){
			ROFL_ERR("<%s:%d> All physical port slots are occupied\n",__func__, __LINE__);
			freeifaddrs(ifaddr);
			assert(0);
			return ROFL_FAILURE;
		}

	}

	//This MUST be here and NOT in the previous loop, or the ports will be discovered (duplicated)
	//via update_physical_ports. FIXME: this needs to be implemented properly	
	array = physical_switch_get_physical_ports(&max_ports);
	for(i=0; i<max_ports ; i++){
		if(array[i] != NULL){
			//Update status
			if(update_port_status(array[i]->name) != ROFL_SUCCESS){
				ROFL_ERR("<%s:%d> Unable to retrieve link and/or admin status of the interface\n",__func__, __LINE__);
				freeifaddrs(ifaddr);
				assert(0);
				return ROFL_FAILURE;
			}
		}
	}

	freeifaddrs(ifaddr);
	
	return ROFL_SUCCESS;
}




rofl_result_t destroy_port(switch_port_t* port){
	
	ioport* ioport_instance;

	if(!port)
		return ROFL_FAILURE;
	
	//Destroy the inner driver-dependant structures
	ioport_instance = (ioport*)port->platform_port_state;	
	delete ioport_instance;

	//Delete port from the pipeline library
	if(physical_switch_remove_port(port->name) == ROFL_FAILURE)
		return ROFL_FAILURE;

	return ROFL_SUCCESS;
}


/*
 * Discovers platform physical ports and fills up the switch_port_t sructures
 *
 */
rofl_result_t destroy_ports(){

	unsigned int max_ports, i;
	switch_port_t** array;	

	array = physical_switch_get_physical_ports(&max_ports);
	
	for(i=0; i<max_ports ; i++){
		if(array[i] != NULL){
			destroy_port(array[i]);
		}
	}

	//TODO: add virtual and tun

	return ROFL_SUCCESS;
}
/*
* Port attachement/detachment 
*/

/*
* Enable port (direct call to ioport)
*/
rofl_result_t enable_port(platform_port_state_t* ioport_instance){

	ioport* port = (ioport*)ioport_instance; 

	if(port->enable() == ROFL_FAILURE)
		return ROFL_FAILURE;

	return ROFL_SUCCESS;
}

/*
* Disable port (direct call to ioport)
*/
rofl_result_t disable_port(platform_port_state_t* ioport_instance){

	class ioport* port = (ioport*)ioport_instance; 

	if(port->disable() == ROFL_FAILURE)
		return ROFL_FAILURE;

	return ROFL_SUCCESS;
}

/*
 * Discover new platform ports (usually triggered by bg task manager)
 */
rofl_result_t update_physical_ports(){

	switch_port_t *port, **ports;
	int sock;
	struct ifaddrs *ifaddr, *ifa;
	unsigned int i, max_ports;
	std::map<std::string, struct ifaddrs*> system_ifaces;
	std::map<std::string, switch_port_t*> pipeline_ifaces;
	
	ROFL_DEBUG_VERBOSE("Trying to update the list of physical interfaces...\n");	
	
	/*real way to find interfaces*/
	//getifaddrs(&ifap); -> there are examples on how to get the ip addresses
	if (getifaddrs(&ifaddr) == -1){
		return ROFL_FAILURE;	
	}
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		return ROFL_FAILURE;	
    	}
	
	//Call the forwarding module to list the ports
	ports = fwd_module_get_physical_ports(&max_ports);

	if(!ports){
		close(sock);
		return ROFL_FAILURE;	
	}

	//Generate some helpful vectors
	for(i=0;i<max_ports;i++){
		if(ports[i])
			pipeline_ifaces[std::string(ports[i]->name)] = ports[i];	
			
	}
	for(ifa = ifaddr; ifa != NULL; ifa=ifa->ifa_next){
		if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;
			
		system_ifaces[std::string(ifa->ifa_name)] = ifa;	
	}

	//Validate the existance of the ports in the pipeline and remove
	//the ones that are no longer there. If they still exist remove them from ifaces
	for (std::map<std::string, switch_port_t*>::iterator it = pipeline_ifaces.begin(); it != pipeline_ifaces.end(); ++it){
		if (system_ifaces.find(it->first) == system_ifaces.end() ) {
			//Interface has been deleted. Detach and remove
			port = it->second;
			if(!port)
				continue;

			ROFL_INFO("Interface %s has been removed from the system. The interface will now be detached from any logical switch it is attached to (if any), and removed from the list of physical interfaces.\n", it->first.c_str());
			//Detach
			if(port->attached_sw && (fwd_module_detach_port_from_switch(port->attached_sw->dpid, port->name) != AFA_SUCCESS) ){
				ROFL_WARN("WARNING: unable to detach port %s from switch. This can lead to an unknown behaviour\n", it->first.c_str());
				assert(0);
			}
			//Destroy and remove from the list of physical ports
			destroy_port(port);	
		} 
		system_ifaces.erase(it->first);
	}
	
	//Add remaining "new" interfaces (remaining interfaces in system_ifaces map 
	for (std::map<std::string, struct ifaddrs*>::iterator it = system_ifaces.begin(); it != system_ifaces.end(); ++it){
		//Fill port
		port = fill_port(sock,  it->second);
		if(!port){
			ROFL_ERR("Unable to initialize newly discovered interface %s\n", it->first.c_str());
			continue;
		}		

		//Adding the 
		if( physical_switch_add_port(port) != ROFL_SUCCESS ){
			ROFL_ERR("<%s:%d> Unable to add port %s to physical switch. Not enough slots?\n", it->first.c_str());
			freeifaddrs(ifaddr);
			continue;	
		}

	}
	
	ROFL_DEBUG_VERBOSE("Update of interfaces done.\n");	

	freeifaddrs(ifaddr);
	close(sock);
	
	return ROFL_SUCCESS;
}
