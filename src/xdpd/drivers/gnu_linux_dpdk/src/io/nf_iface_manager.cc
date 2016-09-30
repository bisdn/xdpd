#include "nf_iface_manager.h"
#include <rofl/datapath/hal/cmm.h>
#include <utils/c_logger.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/physical_switch.h>


#include "../config.h"

#include "iface_manager.h"
#include "port_state.h"

#include <assert.h>
#include <rte_common.h>
#include <rte_malloc.h>
#include <rte_errno.h>

#include <fcntl.h>

//fwd decl
extern pthread_rwlock_t iface_manager_rwlock;
extern struct rte_mempool* direct_pools[MAX_CPU_SOCKETS];

switch_port_t* nf_port_mapping[PORT_MANAGER_MAX_PORTS] = {0};
struct rte_ring* port_tx_nf_lcore_queue[PORT_MANAGER_MAX_PORTS] = {NULL};
unsigned int nf_id = 0;

/*
* Handle KNI ports commands
*/
void iface_manager_handle_kni_commands(){

	switch_port_t* port;

	for(int i=0;i<PORT_MANAGER_MAX_PORTS;i++){
		pthread_rwlock_rdlock(&iface_manager_rwlock);
		port = nf_port_mapping[i];
		if(unlikely(port != NULL)){
			if(port->type == PORT_TYPE_NF_EXTERNAL){
				dpdk_kni_port_state *port_state = (dpdk_kni_port_state_t*)port->platform_port_state;
				rte_kni_handle_request(port_state->kni);
			}
		}
		pthread_rwlock_unlock(&iface_manager_rwlock);
	}

}

/*
* Handle external requests to bring up/down a KNI interface
*/
static int kni_config_network_interface(uint8_t port_id, uint8_t if_up){

	switch_port_t* port = nf_port_mapping[port_id];

	dpdk_kni_port_state_t *port_state = (dpdk_kni_port_state_t*)port->platform_port_state;

	if(port_state->just_created){
		port_state->just_created = false;
		return !if_up;
	}

	switch_port_snapshot_t* port_snapshot;

	assert(port != NULL);

	if(!port || !port->platform_port_state)
		return ROFL_FAILURE;

	XDPD_INFO(DRIVER_NAME"[port_manager] Putting the KNI interface \"%s\" %s...\n", port->name,(if_up)?"UP":"DOWN");

	port_snapshot = physical_switch_get_port_snapshot(port->name);
	hal_cmm_notify_port_status_changed(port_snapshot);

	port->up = (if_up == 1)? true : false;

	return !if_up;
}

//Initializes the pipeline structure and launches the (DPDK) NF port
static switch_port_t* configure_nf_port_shmem(const char *nf_name, const char *nf_port){

	switch_port_t* port;
	char queue_name[PORT_QUEUE_MAX_LEN_NAME];

	//Initialize pipeline port
	port = switch_port_init((char*)nf_port, false, PORT_TYPE_NF_SHMEM, PORT_STATE_NONE);
	if(!port)
		return NULL;

	//Generate port state
	dpdk_shmem_port_state_t* ps = (dpdk_shmem_port_state_t*)rte_malloc(NULL,sizeof(dpdk_shmem_port_state_t),0);

	if(!ps){
		switch_port_destroy(port);
		return NULL;
	}

	//Create rofl-pipeline queue state
	if(switch_port_add_queue(port, 0, (char*)&nf_port, IO_IFACE_MAX_PKT_BURST, 0, 0) != ROFL_SUCCESS){
		XDPD_ERR(DRIVER_NAME"[port_manager] Cannot configure queues on device (pipeline): %s\n", port->name);
		assert(0);
		return NULL;
	}

	// Create the state of the NF port

	//queue xDPD -> NF
	snprintf(queue_name, PORT_QUEUE_MAX_LEN_NAME, "%s-to-nf", nf_port);

	ps->to_nf_queue = rte_ring_create(queue_name, IO_TX_LCORE_QUEUE_SLOTS , SOCKET_ID_ANY, RING_F_SC_DEQ);
	if(unlikely( ps->to_nf_queue == NULL )){
		XDPD_ERR(DRIVER_NAME"[port_manager] Cannot create '%s' rte_ring for queue in port: %s\n", queue_name,nf_port);
		assert(0);
		return NULL;
	}

	//queue NF -> xDPD
	snprintf(queue_name, PORT_QUEUE_MAX_LEN_NAME, "%s-to-xdpd", nf_port);

	ps->to_xdpd_queue = rte_ring_create(queue_name, IO_TX_LCORE_QUEUE_SLOTS , SOCKET_ID_ANY, RING_F_SP_ENQ);
	if(unlikely( ps->to_xdpd_queue == NULL )){
		XDPD_ERR(DRIVER_NAME"[port_manager] Cannot create '%s' rte_ring for queue in port: %s\n", queue_name, nf_port);
		assert(0);
		return NULL;
	}

#ifdef ENABLE_DPDK_SECONDARY_SEMAPHORE
	//semaphore
	ps->semaphore = sem_open(nf_name, O_CREAT , 0644, 0);

	if(ps->semaphore == SEM_FAILED){
		XDPD_ERR(DRIVER_NAME"[port_manager] Cannot create semaphore '%s' for queue in port: %s\n", nf_port, nf_port);
		assert(0);
		return NULL;
	}
	ps->counter_from_last_flush = 0;
#endif
	ps->nf_id = nf_id;

	ps->scheduled = false;
	port->platform_port_state = (platform_port_state_t*)ps;

	//Set the port in the nf_port_mapping
	nf_port_mapping[nf_id] = port;

	nf_id++;

	XDPD_INFO(DRIVER_NAME"[port_manager] Created (NF) port '%s'\n", nf_port);

	return port;
}

//Initializes the pipeline structure and launches the (KNI) NF port
static switch_port_t* configure_nf_port_kni(const char *nf_name, const char *nf_port){

	switch_port_t* port;

	//Initialize pipeline port
	port = switch_port_init((char*)nf_port, false, PORT_TYPE_NF_EXTERNAL, PORT_STATE_NONE);
	if(!port)
		return NULL;

	//Generate port state
	dpdk_kni_port_state_t* ps = (dpdk_kni_port_state_t*)rte_malloc(NULL,sizeof(dpdk_kni_port_state_t),0);

	if(!ps){
		switch_port_destroy(port);
		return NULL;
	}

	//Add TX queue to the pipeline

	//Create rofl-pipeline queue state
	if(switch_port_add_queue(port, 0, (char*)&nf_port, IO_IFACE_MAX_PKT_BURST, 0, 0) != ROFL_SUCCESS){
		XDPD_ERR(DRIVER_NAME"[port_manager] Cannot configure queues on device (pipeline): %s\n", port->name);
		assert(0);
		return NULL;
	}

	//Add port_tx_nf_lcore_queue
	port_tx_nf_lcore_queue[nf_id] = rte_ring_lookup (port->name);
	if(!port_tx_nf_lcore_queue[nf_id])
		port_tx_nf_lcore_queue[nf_id] = rte_ring_create(port->name, IO_TX_LCORE_QUEUE_SLOTS , SOCKET_ID_ANY, RING_F_SC_DEQ);

	if(unlikely(port_tx_nf_lcore_queue[nf_id] == NULL )){
		XDPD_ERR(DRIVER_NAME"[iface_manager] Cannot create rte_ring for queue on device: %s\n", port->name);
		assert(0);
		return NULL;
	}

	// Create the state of the NF port

	struct rte_kni_conf conf;
	struct rte_kni_ops ops;

	memset(&conf, 0, sizeof(conf));
	memset(&ops, 0, sizeof(ops));

	sprintf(conf.name,"%s", nf_port);
	conf.mbuf_size = IO_MAX_PACKET_SIZE;

	ops.port_id = nf_id;
	ops.config_network_if = kni_config_network_interface;

	ps->kni = rte_kni_alloc(direct_pools[0], &conf, &ops);

	if (ps->kni == NULL){
		XDPD_ERR(DRIVER_NAME"[port_manager] Cannot create KNI context for port: %s\n",nf_port);
		assert(0);
		return NULL;
	}
	ps->nf_id = nf_id;

	ps->just_created = true;
	ps->scheduled = false;
	port->platform_port_state = (platform_port_state_t*)ps;

	//Set the port in the nf_port_mapping
	nf_port_mapping[nf_id] = port;

	nf_id++;

	XDPD_INFO(DRIVER_NAME"[port_manager] Created (NF) port '%s'\n", nf_port);

	return port;
}

rofl_result_t iface_manager_create_nf_port(const char *nf_name, const char *nf_port, port_type_t nf_port_type){

	switch_port_t* port = NULL;

	XDPD_INFO(DRIVER_NAME"[port_manager] Creating a NF port named '%s'\n",nf_port);

	if(nf_port_type == PORT_TYPE_NF_SHMEM){
		if(! ( port = configure_nf_port_shmem(nf_name,nf_port) ) ){
			XDPD_ERR(DRIVER_NAME"[port_manager] Unable to initialize DPDK NF port %s\n", nf_port);
			return ROFL_FAILURE;
		}
	}else if(nf_port_type == PORT_TYPE_NF_EXTERNAL){
		if(! ( port = configure_nf_port_kni(nf_name,nf_port) ) ){
			XDPD_ERR(DRIVER_NAME"[port_manager] Unable to initialize KNI NF port %s\n", nf_port);
			return ROFL_FAILURE;
		}
	}

	//Add port to the pipeline
	if( physical_switch_add_port(port) != ROFL_SUCCESS ){
		XDPD_ERR(DRIVER_NAME"[port_manager] Unable to add the switch (NF) port to physical switch; perhaps there are no more physical port slots available?\n");
		return ROFL_FAILURE;
	}

	return ROFL_SUCCESS;
}

rofl_result_t iface_manager_destroy_nf_port(const char *port_name){

	switch_port_t *port = physical_switch_get_port_by_name(port_name);

	pthread_rwlock_wrlock(&iface_manager_rwlock);

	if(port->type == PORT_TYPE_NF_SHMEM){
		dpdk_shmem_port_state_t *port_state = (dpdk_shmem_port_state_t*)port->platform_port_state;

		nf_port_mapping[port_state->nf_id] = NULL;

		//According to http://dpdk.info/ml/archives/dev/2014-January/001120.html,
		//rte_rings connot be destroyed

#ifdef ENABLE_DPDK_SECONDARY_SEMAPHORE
		sem_unlink(port_name);
		sem_close(port_state->semaphore);
#endif

		if(physical_switch_remove_port(port_name) != ROFL_SUCCESS){
			XDPD_ERR(DRIVER_NAME"[port_manager] Cannot remove NF port '%s' from the physical switch\n", port->name);
			assert(0);
			pthread_rwlock_unlock(&iface_manager_rwlock);
			return ROFL_FAILURE;
		}
		rte_free(port_state);

	}else if(port->type == PORT_TYPE_NF_EXTERNAL){

		dpdk_kni_port_state_t *port_state = (dpdk_kni_port_state_t*)port->platform_port_state;

		nf_port_mapping[port_state->nf_id] = NULL;

		rte_kni_release(port_state->kni);
		port_state->kni = NULL;

		if(physical_switch_remove_port(port_name) != ROFL_SUCCESS){
			XDPD_ERR(DRIVER_NAME"[port_manager] Cannot remove NF port '%s' from the physical switch\n", port->name);
			assert(0);
			pthread_rwlock_unlock(&iface_manager_rwlock);
			return ROFL_FAILURE;
		}
		rte_free(port_state);
	}

	pthread_rwlock_unlock(&iface_manager_rwlock);

	return ROFL_SUCCESS;
}


rofl_result_t nf_iface_manager_bring_up_port(switch_port_t* port){

	if(port->type == PORT_TYPE_NF_SHMEM)
		return ROFL_SUCCESS;

	if(port->type == PORT_TYPE_NF_EXTERNAL){
		struct ifreq ifr;

		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd == -1){
			XDPD_ERR(DRIVER_NAME"[nf_driver] Cannot bring up KNI NF port\n");
			return ROFL_FAILURE;
		}
		/* get interface name */
		strncpy(ifr.ifr_name, port->name, IFNAMSIZ);

		/* Read interface flags */
		if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0){
			XDPD_ERR(DRIVER_NAME"[nf_driver] Cannot bring up KNI NF port\n");
			return ROFL_FAILURE;
		}

		#ifdef ifr_flags
			# define IRFFLAGS       ifr_flags
		#else   /* Present on kFreeBSD */
			# define IRFFLAGS       ifr_flagshigh
		#endif

		// If interface is down, bring it up
		if (!(ifr.IRFFLAGS & IFF_UP)){
			ifr.IRFFLAGS |= IFF_UP;
			if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0){
				XDPD_ERR(DRIVER_NAME"[nf_driver] Cannot bring up KNI NF port\n");
				return ROFL_FAILURE;
			}
		}

		return ROFL_SUCCESS;
	}

	XDPD_ERR(DRIVER_NAME"[nf_driver] The port type must be PORT_TYPE_NF_SHMEM or PORT_TYPE_NF_EXTERNAL\n");
	return ROFL_FAILURE;
}

rofl_result_t nf_iface_manager_bring_down_port(switch_port_t* port){

	if(port->type == PORT_TYPE_NF_SHMEM)
		return ROFL_SUCCESS;

	if(port->type == PORT_TYPE_NF_EXTERNAL){
		struct ifreq ifr;

		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd == -1){
			XDPD_ERR(DRIVER_NAME"[nf_driver] Cannot bring down KNI NF port\n");
			return ROFL_FAILURE;
		}
		/* get interface name */
		strncpy(ifr.ifr_name, port->name, IFNAMSIZ);

		/* Read interface flags */
		if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0){
			XDPD_ERR(DRIVER_NAME"[nf_driver] Cannot bring down DPDK KNI NF port\n");
			return ROFL_FAILURE;
		}

		#ifdef ifr_flags
			# define IRFFLAGS       ifr_flags
		#else   /* Present on kFreeBSD */
			# define IRFFLAGS       ifr_flagshigh
		#endif

		// If interface is up, bring it down
		if (ifr.IRFFLAGS & IFF_UP){
			ifr.IRFFLAGS &= ~IFF_UP;
			if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0){
				XDPD_ERR(DRIVER_NAME"[nf_driver] Cannot bring down DPDK KNI NF port\n");
				return ROFL_FAILURE;
			}
		}
		return ROFL_SUCCESS;
	}

	XDPD_ERR(DRIVER_NAME"[nf_driver] The port type must be PORT_TYPE_NF_SHMEM or PORT_TYPE_NF_EXTERNAL\n");
	return ROFL_FAILURE;
}
