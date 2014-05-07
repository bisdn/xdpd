#include "pktin_dispatcher.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/hal/openflow/openflow1x/of1x_cmm.h>

#include "bufferpool.h"
#include "datapacket_storage.h"
#include "dpdk_datapacket.h"

sem_t pktin_sem;
struct rte_ring* pkt_ins;
bool keep_on_pktins;
bool destroying_sw;
static pthread_t pktin_thread;
static sem_t pktin_drained;
static pthread_mutex_t drain_mutex=PTHREAD_MUTEX_INITIALIZER;

using namespace xdpd::gnu_linux;

//MBUF pool
extern struct rte_mempool* pool_direct;

//Process packet_ins
static void* process_packet_ins(void* param){

	datapacket_t* pkt;
	datapacket_dpdk_t* pkt_dpdk;
	hal_result_t rv;
	storeid id;
	of1x_switch_t* sw;	
	datapacket_storage* dps;
	struct rte_mbuf* mbuf;
	int drain_credits = -1; 

	//prepare timeout
	struct timespec timeout;

	while(likely(keep_on_pktins)){

		if(destroying_sw == true){
			if(drain_credits == -1){
				//Recover the number of credits that need to be decremented.
				while(sem_getvalue(&pktin_sem, &drain_credits) != 0);	
			}

			if(drain_credits == 0){
				//We have all credits drained
				//Reset the drain_credits flag and destroying_sw
				drain_credits = -1;
				destroying_sw = false;		
	
				//Wake up the caller of wait_pktin_draining() 
				sem_post(&pktin_drained);
			}
		}
		
		//Wait for incomming pkts
		clock_gettime(CLOCK_REALTIME, &timeout);
    		timeout.tv_sec += 1; 
		if( sem_timedwait(&pktin_sem, &timeout)  == ETIMEDOUT )
			continue;	

		//Dequeue
		if(rte_ring_sc_dequeue(pkt_ins, (void**)&pkt) != 0)
			continue;

		if(drain_credits > 0)
			drain_credits--;

		//Recover platform state
		pkt_dpdk = (datapacket_dpdk_t*)pkt->platform_state;
		mbuf = ((datapacket_dpdk_t*)pkt->platform_state)->mbuf;
		sw = (of1x_switch_t*)pkt->sw;
		dps = (datapacket_storage*)pkt->sw->platform_state;

		ROFL_DEBUG(DRIVER_NAME"[pktin_dispatcher] Processing PKT_IN for packet(%p), mbuf %p, switch %p\n", pkt, mbuf, sw);
		//Store packet in the storage system. Packet is NOT returned to the bufferpool
		id = dps->store_packet(pkt);

		if(unlikely(id == datapacket_storage::ERROR)){
			ROFL_DEBUG(DRIVER_NAME"[pktin_dispatcher] PKT_IN for packet(%p) could not be stored in the storage. Dropping..\n",pkt);

			//Return mbuf to the pool
			rte_pktmbuf_free(mbuf);

			//Return to the bufferpool
			bufferpool::release_buffer(pkt);
			continue;
		}

		//Process packet in
		rv = hal_cmm_process_of1x_packet_in(sw->dpid, 
						pkt_dpdk->pktin_table_id, 	
						pkt_dpdk->pktin_reason, 	
						pkt_dpdk->in_port, 
						id, 	
						get_buffer_dpdk(pkt_dpdk), 
						pkt_dpdk->pktin_send_len, 
						get_buffer_length_dpdk(pkt_dpdk),
						((packet_matches_t*)&pkt->matches)
				);

		if( rv != HAL_SUCCESS ){
			ROFL_DEBUG(DRIVER_NAME"[pktin_dispatcher] PKT_IN for packet(%p) could not be sent to sw:%s controller. Dropping..\n",pkt,sw->name);
			//Take packet out from the storage
			pkt = dps->get_packet(id);

			//Return mbuf to the pool
			rte_pktmbuf_free(mbuf);

			//Return to the bufferpool
			bufferpool::release_buffer(pkt);
		}
	}

	return NULL;
}

void wait_pktin_draining(of_switch_t* sw){

	//
	// Here we have to make sure that we are not returning
	// until all the PKT_INs belonging to the LSI have been
	// processed (dropped by the CMM)
	//
	// Note that *no more packets* will be enqueued in the global
	// PKT_IN queue, since ports shall be descheduled already.
	//
	// Strategy is just to make sure all current pending PKT_INs
	// are processed and return
	//

	//Serialize calls to wait_pktin_draining
	pthread_mutex_lock(&drain_mutex);

	//Signal PKT_IN thread that should unblock us when all current credits
	//have been drained 
	destroying_sw = true;
	sem_wait(&pktin_drained);
	
	//Serialize calls to wait_pktin_draining
	pthread_mutex_unlock(&drain_mutex);
}

// Launch pkt in thread
rofl_result_t pktin_dispatcher_init(){

	keep_on_pktins = true;

	//Syncrhonization between I/O threads and PKT_IN thread
	if(sem_init(&pktin_sem, 0,0) < 0){
		return ROFL_FAILURE;
	}	
	
	//Synchronization between PKT_IN thread and mgmt thread
	if(sem_init(&pktin_drained, 0,0) < 0){
		return ROFL_FAILURE;
	}

	//PKT_IN queue
	pkt_ins = rte_ring_create("PKT_IN_RING", IO_PKT_IN_STORAGE_MAX_BUF, SOCKET_ID_ANY, 0x0);
	
	if(!pkt_ins){
		ROFL_ERR(DRIVER_NAME"[pktin_dispatcher] Unable to create PKT_INs ring queue\n");
		return ROFL_FAILURE;
	}

	//Reset synchronization flag 
	destroying_sw = false;	

	//Launch thread
	//XXX: use rte?
	if(pthread_create(&pktin_thread, NULL, process_packet_ins, NULL)<0){
		ROFL_ERR(DRIVER_NAME"[pktin_dispatcher] pthread_create failed, errno(%d): %s\n", errno, strerror(errno));
		return ROFL_FAILURE;
	}

	return ROFL_SUCCESS;
}

//Stop and destroy packet in dispatcher
rofl_result_t pktin_dispatcher_destroy(){
	
	keep_on_pktins = false;
	pthread_join(pktin_thread,NULL);
	
	//Wait for thread to stop
	//XXX: use rte?
	
	//Destroy ring?
	//XXX: not available??
	
	return ROFL_SUCCESS;
}
