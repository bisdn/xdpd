#include "iface_manager.h"
#include <rofl/datapath/hal/cmm.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/physical_switch.h>

#include "port_state.h"

#include "../config.h"
#include <assert.h> 
#include <rte_common.h> 
#include <rte_malloc.h> 
#include <rte_errno.h> 

extern struct rte_mempool *pool_direct;
switch_port_t* port_mapping[PORT_MANAGER_MAX_PORTS] = {0};
struct rte_ring* port_tx_lcore_queue[PORT_MANAGER_MAX_PORTS][IO_IFACE_NUM_QUEUES] = {{NULL}};

//Initializes the pipeline structure and launches the port 
static switch_port_t* configure_port(unsigned int port_id){

	int ret;
	unsigned int i;
	switch_port_t* port;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_conf port_conf;
	char port_name[SWITCH_PORT_MAX_LEN_NAME];
	char queue_name[PORT_QUEUE_MAX_LEN_NAME];
	
	//Get info
	rte_eth_dev_info_get(port_id, &dev_info);

	//Hack to "deduce" the maximum speed of the NIC.
	//As of DPDK v1.4 there is not way to retreive such features from
	//the NIC
	if( strncmp(dev_info.driver_name, "rte_ixgbe", 9) == 0 ){
		/* 10G */
		snprintf (port_name, SWITCH_PORT_MAX_LEN_NAME, "10ge%u",port_id);
	}else{
		/* 1G */
		snprintf (port_name, SWITCH_PORT_MAX_LEN_NAME, "ge%u",port_id);
	}

	//Initialize pipeline port
	port = switch_port_init(port_name, false, PORT_TYPE_PHYSICAL, PORT_STATE_NONE);
	if(!port)
		return NULL; 

	//Generate port state
	dpdk_port_state_t* ps = (dpdk_port_state_t*)rte_malloc(NULL,sizeof(dpdk_port_state_t),0);
	
	if(!ps){
		switch_port_destroy(port);
		return NULL;
	}

	//Set rx and tx queues
	memset(&port_conf, 0, sizeof(port_conf));
	port_conf.rxmode.max_rx_pkt_len =  IO_MAX_PACKET_SIZE;
	//port_conf.rxmode.hw_ip_checksum = 1;
	//port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IPV4 | ETH_RSS_IPV6;
	port_conf.txmode.mq_mode = ETH_MQ_TX_NONE;
	if ((ret=rte_eth_dev_configure(port_id, 1, IO_IFACE_NUM_QUEUES, &port_conf)) < 0){
		ROFL_ERR(DRIVER_NAME"[iface_manager][%s] Cannot configure device; %s(%d)\n", port->name, rte_strerror(ret), ret);
		assert(0);
		return NULL;
	}

	//Add TX queues to the pipeline
	//Filling one-by-one the queues 
	for(i=0;i<IO_IFACE_NUM_QUEUES;i++){
		
		//Create rofl-pipeline queue state
		snprintf(queue_name, PORT_QUEUE_MAX_LEN_NAME, "%s%d", "queue", i);
		if(switch_port_add_queue(port, i, (char*)&queue_name, IO_IFACE_MAX_PKT_BURST, 0, 0) != ROFL_SUCCESS){
			ROFL_ERR(DRIVER_NAME"[iface_manager] Cannot configure queues on device (pipeline): %s\n", port->name);
			assert(0);
			return NULL;
		}
		
		//Add port_tx_lcore_queue
		snprintf(queue_name, PORT_QUEUE_MAX_LEN_NAME, "%u-q%u", port_id, i);
		port_tx_lcore_queue[port_id][i] = rte_ring_create(queue_name, IO_TX_LCORE_QUEUE_SLOTS , SOCKET_ID_ANY, RING_F_SC_DEQ);
	
		
		if(unlikely( port_tx_lcore_queue[port_id][i] == NULL )){
			ROFL_ERR(DRIVER_NAME"[iface_manager] Cannot create rte_ring for queue on device: %s\n", port->name);
			assert(0);
			return NULL;
		}
	}

	//Fill-in dpdk port state
	ps->queues_set = false;
	ps->scheduled = false;
	ps->port_id = port_id;
	port->platform_port_state = (platform_port_state_t*)ps;

	ROFL_INFO(DRIVER_NAME"[iface_manager] Discovered port %s [%u:%u:%u] id %u\n", port_name, dev_info.pci_dev->addr.domain, dev_info.pci_dev->addr.bus, dev_info.pci_dev->addr.devid, port_id);



	//Set the port in the port_mapping
	port_mapping[port_id] = port;

	return port;
}

rofl_result_t iface_manager_set_queues(switch_port_t* port, unsigned int core_id, unsigned int port_id ){
	
	unsigned int i;
	int ret;
	struct rte_eth_rxconf rx_conf;
	rx_conf.rx_thresh.pthresh = RX_PTHRESH;
	rx_conf.rx_thresh.hthresh = RX_HTHRESH;
	rx_conf.rx_thresh.wthresh = RX_WTHRESH;
	rx_conf.rx_free_thresh = 32;
	
	struct rte_eth_txconf tx_conf;
	tx_conf.tx_thresh.pthresh = TX_PTHRESH;
	tx_conf.tx_thresh.hthresh = TX_HTHRESH;
	tx_conf.tx_thresh.wthresh = TX_WTHRESH;
	tx_conf.tx_free_thresh = 0; /* Use PMD default values */
	tx_conf.tx_rs_thresh = 0; /* Use PMD default values */
	
	//Recover the 	
	dpdk_port_state_t* dpdk_port = (dpdk_port_state_t*)port->platform_port_state;
	
	if(dpdk_port->queues_set)
		return ROFL_SUCCESS;
	
	//Setup RX
	if( (ret=rte_eth_rx_queue_setup(port_id, 0, RTE_TEST_RX_DESC_DEFAULT, rte_eth_dev_socket_id(port_id), &rx_conf, pool_direct)) < 0 ){
		ROFL_ERR(DRIVER_NAME"[iface_manager] Cannot setup RX queue: %s\n", rte_strerror(ret));
		assert(0);
		return ROFL_FAILURE;
	}

	//Setup TX
	for(i=0;i<IO_IFACE_NUM_QUEUES;++i){
		//setup the queue
		if( (ret = rte_eth_tx_queue_setup(port_id, i, RTE_TEST_TX_DESC_DEFAULT, rte_eth_dev_socket_id(port_id), &tx_conf)) < 0 ){
			ROFL_ERR(DRIVER_NAME"[iface_manager] Cannot setup TX queues: %s\n", rte_strerror(ret));
			assert(0);
			return ROFL_FAILURE;
		}

#if 0
		//bind stats IGB not supporting this???
		if( (ret = rte_eth_dev_set_tx_queue_stats_mapping(port_id, i, i)) < 0 ){
			ROFL_ERR(DRIVER_NAME"[iface_manager] Cannot bind TX queue(%u) stats: %s\n", i, rte_strerror(ret));
			assert(0);
			return ROFL_FAILURE;
		}
#endif
	}
	//Start port
	if((ret=rte_eth_dev_start(port_id)) < 0){
		ROFL_ERR(DRIVER_NAME"[iface_manager] Cannot start device %u:  %s\n", port_id, rte_strerror(ret));
		assert(0);
		return ROFL_FAILURE; 
	}

	//Set pipeline state to UP
	if(likely(port_mapping[port_id]!=NULL)){
		port_mapping[port_id]->up = true;
	}

	//Set promiscuous mode
	rte_eth_promiscuous_enable(port_id);

	//Enable multicast
	rte_eth_allmulticast_enable(port_id);
	
	//Reset stats
	rte_eth_stats_reset(port_id);

	//Set as queues setup
	dpdk_port->queues_set=true;
	
	return ROFL_SUCCESS;
}

/*
* Discovers and initializes (including rofl-pipeline state) DPDK-enabled ports.
*/
rofl_result_t iface_manager_discover_system_ports(void){

	uint8_t i, num_of_ports;
	switch_port_t* port;
	num_of_ports = rte_eth_dev_count();
	
	ROFL_INFO(DRIVER_NAME"[iface_manager] Found %u DPDK-capable interfaces\n", num_of_ports);
	
	for(i=0;i<num_of_ports;++i){
		if(! ( port = configure_port(i) ) ){
			ROFL_ERR(DRIVER_NAME"[iface_manager] Unable to initialize port-id: %u\n", i);
			return ROFL_FAILURE;
		}

		//Add port to the pipeline
		if( physical_switch_add_port(port) != ROFL_SUCCESS ){
			ROFL_ERR(DRIVER_NAME"[iface_manager] Unable to add the switch port to physical switch; perhaps there are no more physical port slots available?\n");
			return ROFL_FAILURE;
		}

	}	

	return ROFL_SUCCESS;
}

/*
* Creates a virtual link port pair. TODO: this function is not thread safe
*/
rofl_result_t iface_manager_create_virtual_port_pair(of_switch_t* lsw1, switch_port_t **vport1, of_switch_t* lsw2, switch_port_t **vport2){

	//Names are composed following vlinkX-Y
	//Where X is the virtual link number (0... N-1)
	//Y is the edge 0 (left) 1 (right) of the connectio
	static unsigned int num_of_vlinks=0;
	char port_name[PORT_QUEUE_MAX_LEN_NAME];
	char queue_name[PORT_QUEUE_MAX_LEN_NAME];
	uint64_t port_capabilities=0x0;
	uint16_t randnum = 0;
	unsigned int i;

	//Init the pipeline ports
	snprintf(port_name,PORT_QUEUE_MAX_LEN_NAME, "vlink%u_%u", num_of_vlinks, 0);

	*vport1 = switch_port_init(port_name, true, PORT_TYPE_VIRTUAL, PORT_STATE_NONE);
	snprintf(port_name,PORT_QUEUE_MAX_LEN_NAME, "vlink%u_%u", num_of_vlinks, 1);

	*vport2 = switch_port_init(port_name, true, PORT_TYPE_VIRTUAL, PORT_STATE_NONE);
	
	if(*vport1 == NULL || *vport2 == NULL){
		ROFL_ERR(DRIVER_NAME"[iface_manager] Unable to allocate memory for virtual ports\n");
		assert(0);
		goto PORT_MANAGER_CREATE_VLINK_PAIR_ERROR;
	}

	//Initalize port features(Marking as 1G)
	port_capabilities |= PORT_FEATURE_1GB_FD;
	switch_port_add_capabilities(&(*vport1)->curr, (port_features_t)port_capabilities);	
	switch_port_add_capabilities(&(*vport1)->advertised, (port_features_t)port_capabilities);	
	switch_port_add_capabilities(&(*vport1)->supported, (port_features_t)port_capabilities);	
	switch_port_add_capabilities(&(*vport1)->peer, (port_features_t)port_capabilities);	

	randnum = (uint16_t)rand();
	(*vport1)->hwaddr[0] = ((uint8_t*)&randnum)[0];
	(*vport1)->hwaddr[1] = ((uint8_t*)&randnum)[1];
	randnum = (uint16_t)rand();
	(*vport1)->hwaddr[2] = ((uint8_t*)&randnum)[0];
	(*vport1)->hwaddr[3] = ((uint8_t*)&randnum)[1];
	randnum = (uint16_t)rand();
	(*vport1)->hwaddr[4] = ((uint8_t*)&randnum)[0];
	(*vport1)->hwaddr[5] = ((uint8_t*)&randnum)[1];
	(*vport1)->hwaddr[0] &= 0xfe;

	//Add queues
	for(i=0;i<IO_IFACE_NUM_QUEUES;i++){
		snprintf(queue_name, PORT_QUEUE_MAX_LEN_NAME, "%s%d", "queue", i);
		if(switch_port_add_queue((*vport1), i, (char*)&queue_name, IO_IFACE_MAX_PKT_BURST, 0, 0) != ROFL_SUCCESS){
			ROFL_ERR(DRIVER_NAME"[iface_manager] Cannot configure queues on device (pipeline): %s\n", (*vport1)->name);
			assert(0);
			goto PORT_MANAGER_CREATE_VLINK_PAIR_ERROR;
		}
	}

	switch_port_add_capabilities(&(*vport2)->curr, (port_features_t)port_capabilities);	
	switch_port_add_capabilities(&(*vport2)->advertised, (port_features_t)port_capabilities);	
	switch_port_add_capabilities(&(*vport2)->supported, (port_features_t)port_capabilities);	
	switch_port_add_capabilities(&(*vport2)->peer, (port_features_t)port_capabilities);	

	randnum = (uint16_t)rand();
	(*vport2)->hwaddr[0] = ((uint8_t*)&randnum)[0];
	(*vport2)->hwaddr[1] = ((uint8_t*)&randnum)[1];
	randnum = (uint16_t)rand();
	(*vport2)->hwaddr[2] = ((uint8_t*)&randnum)[0];
	(*vport2)->hwaddr[3] = ((uint8_t*)&randnum)[1];
	randnum = (uint16_t)rand();
	(*vport2)->hwaddr[4] = ((uint8_t*)&randnum)[0];
	(*vport2)->hwaddr[5] = ((uint8_t*)&randnum)[1];
	(*vport2)->hwaddr[0] &= 0xfe;
	
	//Add queues
	for(i=0;i<IO_IFACE_NUM_QUEUES;i++){
		snprintf(queue_name, PORT_QUEUE_MAX_LEN_NAME, "%s%d", "queue", i);
		if(switch_port_add_queue((*vport2), i, (char*)&queue_name, IO_IFACE_MAX_PKT_BURST, 0, 0) != ROFL_SUCCESS){
			ROFL_ERR(DRIVER_NAME"[iface_manager] Cannot configure queues on device (pipeline): %s\n", (*vport2)->name);
			assert(0);
			goto PORT_MANAGER_CREATE_VLINK_PAIR_ERROR;
		}
	}

	//Interlace them
	(*vport2)->platform_port_state = *vport1;	
	(*vport1)->platform_port_state = *vport2;	


	//Add them to the physical switch
	if( physical_switch_add_port(*vport1) != ROFL_SUCCESS ){
		ROFL_ERR(DRIVER_NAME"[iface_manager] Unable to allocate memory for virtual ports\n");
		assert(0);
		goto PORT_MANAGER_CREATE_VLINK_PAIR_ERROR;	

	}
	if( physical_switch_add_port(*vport2) != ROFL_SUCCESS ){
		ROFL_ERR(DRIVER_NAME"[iface_manager] Unable to allocate memory for virtual ports\n");
		assert(0);
		goto PORT_MANAGER_CREATE_VLINK_PAIR_ERROR;	

	}

	//Increment counter and return
	num_of_vlinks++; 

	return ROFL_SUCCESS;

PORT_MANAGER_CREATE_VLINK_PAIR_ERROR:
	if(*vport1)
		switch_port_destroy(*vport1);
	if(*vport2)
		switch_port_destroy(*vport2);
	return ROFL_FAILURE;
}



/*
* Enable port 
*/
rofl_result_t iface_manager_bring_up(switch_port_t* port){

	unsigned int port_id;
	int ret;
	
	if(unlikely(!port))
		return ROFL_FAILURE;

	
	if(port->type != PORT_TYPE_VIRTUAL){
		/*
		*  PHYSICAL
		*/
		port_id = ((dpdk_port_state_t*)port->platform_port_state)->port_id;

		//Start port in RTE
		if(!port->up){
			//Was down; simply start
			if((ret=rte_eth_dev_start(port_id)) < 0){
				ROFL_ERR(DRIVER_NAME"[iface_manager] Cannot start device %u:  %s\n", port_id, rte_strerror(ret));
				assert(0);
				return ROFL_FAILURE; 
			}
		}
	}else{
		/*
		* Virtual link
		*/
		switch_port_t* port_pair = (switch_port_t*)port->platform_port_state;
		//Set link flag on both ports
		if(port_pair->up){
			port->state &= ~PORT_STATE_LINK_DOWN;
			port_pair->state &= ~PORT_STATE_LINK_DOWN;
		}else{
			port->state |= PORT_STATE_LINK_DOWN;
			port_pair->state |= PORT_STATE_LINK_DOWN;
		}
	}
	//Mark the port as being up and return
	port->up = true;
		
	return ROFL_SUCCESS;
}

/*
* Disable port 
*/
rofl_result_t iface_manager_bring_down(switch_port_t* port){

	unsigned int port_id;
	
	if(unlikely(!port))
		return ROFL_FAILURE;
	
	if(port->type != PORT_TYPE_VIRTUAL){
		/*
		*  PHYSICAL
		*/

		port_id = ((dpdk_port_state_t*)port->platform_port_state)->port_id;

		//First mark the port as NOT up, so that cores don't issue
		//RX/TX calls over the port
		port->up = false;

		//Stop port in RTE
		if(port->up){
			//Was  up; stop it
			rte_eth_dev_stop(port_id);
		}
	}else{
		/*
		* Virtual link
		*/
		switch_port_t* port_pair = (switch_port_t*)port->platform_port_state;
		port->up = false;

		//Set links as down	
		port->state |= PORT_STATE_LINK_DOWN;
		port_pair->state |= PORT_STATE_LINK_DOWN;
	}

	return ROFL_SUCCESS;
}


/*
* Shutdown all ports in the system 
*/
rofl_result_t iface_manager_destroy(void){

	uint8_t i, num_of_ports;
	num_of_ports = rte_eth_dev_count();
	
	for(i=0;i<num_of_ports;++i){
		rte_eth_dev_stop(i);
		rte_eth_dev_close(i);
	}	

	return ROFL_SUCCESS;
}

/*
* Update link states 
*/
void iface_manager_update_links(){

	unsigned int i;
	struct rte_eth_link link;
	switch_port_t* port;
	switch_port_snapshot_t* port_snapshot;
	bool last_link_state;
	
	for(i=0;i<PORT_MANAGER_MAX_PORTS;i++){
		
		port = port_mapping[i];
		
		if(unlikely(port != NULL)){
			rte_eth_link_get_nowait(i,&link);
	
			last_link_state = !((port->state& PORT_STATE_LINK_DOWN) > 0); //up =>1

			//Check if there has been a change
			if(unlikely(last_link_state != link.link_status)){
				if(link.link_status)
					//Up
					port->state = port->state & ~(PORT_STATE_LINK_DOWN); 
				else
					//Down
					port->state = port->state | PORT_STATE_LINK_DOWN;
					
				ROFL_DEBUG(DRIVER_NAME"[port-manager] Port %s is %s, and link is %s\n", port->name, ((port->up) ? "up" : "down"), ((link.link_status) ? "detected" : "not detected"));
				
				//Notify CMM port change
				port_snapshot = physical_switch_get_port_snapshot(port->name); 
				if(hal_cmm_notify_port_status_changed(port_snapshot) != HAL_SUCCESS){
					ROFL_DEBUG(DRIVER_NAME"[iface_manager] Unable to notify port status change for port %s\n", port->name);
				}	
			}
		}
	}
}

/*
* Update port stats (pipeline)
*/
void iface_manager_update_stats(){
	
	unsigned int i, j;
	struct rte_eth_stats stats;
	switch_port_t* port;
	
	for(i=0;i<PORT_MANAGER_MAX_PORTS;i++){
		port = port_mapping[i];
		if(unlikely(port != NULL)){

			//Retrieve stats
			rte_eth_stats_get(i, &stats);
			
			//RX	
			port->stats.rx_packets = stats.ipackets;
			port->stats.rx_bytes = stats.ibytes;
			port->stats.rx_errors = stats.ierrors;
				
			//FIXME: collisions and other errors
		
			//TX
			port->stats.tx_packets = stats.opackets;
			port->stats.tx_bytes = stats.obytes;
			port->stats.tx_errors = stats.oerrors;

			//TX-queues
			for(j=0;j<IO_IFACE_NUM_QUEUES;j++){
				port->queues[j].stats.tx_packets = stats.q_opackets[j];
				port->queues[j].stats.tx_bytes = stats.q_obytes[j];
				//port->queues[j].stats.overrun = stats.q_;
			}
		}
	}

}
