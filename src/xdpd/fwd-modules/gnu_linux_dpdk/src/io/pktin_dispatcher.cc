#include "pktin_dispatcher.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>

#include "bufferpool.h"
#include "datapacket_storage.h"
#include "dpdk_datapacket.h"

sem_t pktin_sem;
struct rte_ring* pkt_ins;
bool keep_on_pktins;
bool destroying_sw;
static pthread_t pktin_thread;
static unsigned int processed_pkt_ins;
static pthread_mutex_t pktin_mutex=PTHREAD_MUTEX_INITIALIZER;

using namespace xdpd::gnu_linux;

//MBUF pool
extern struct rte_mempool* pool_direct;

//Process packet_ins
static void* process_packet_ins(void* param){

	datapacket_t* pkt;
	datapacket_dpdk_t* pkt_dpdk;
	afa_result_t rv;
	unsigned int pkt_size;
	storeid id;
	of1x_switch_t* sw;	
	datapacket_storage* dps;
	struct rte_mbuf* mbuf;

	//prepare timeout
	struct timespec timeout;

	while(likely(keep_on_pktins)){

		//Wait for incomming pkts
		clock_gettime(CLOCK_REALTIME, &timeout);
    		timeout.tv_sec += 1; 
		if( sem_timedwait(&pktin_sem, &timeout)  == ETIMEDOUT )
			continue;	

		//Dequeue
		if(rte_ring_sc_dequeue(pkt_ins, (void**)&pkt) != 0)
			continue;

		//Increment counter (Note: MUST be here)
		processed_pkt_ins++;		

		if(unlikely(destroying_sw == true)){
			//Let mgmt thread capture sem_value and processed_pkt_ins 
			//atomically
			//We have to make sure
			pthread_mutex_unlock(&pktin_mutex);
	
			//Wait until it isgnals us it has retrieved values 
			pthread_mutex_lock(&pktin_mutex);
		}

		//Recover platform state
		pkt_dpdk = (datapacket_dpdk_t*)pkt->platform_state;
		mbuf = ((datapacket_dpdk_t*)pkt->platform_state)->mbuf;
		sw = (of1x_switch_t*)pkt->sw;
		dps = (datapacket_storage*)pkt->sw->platform_state;

		ROFL_DEBUG("Processing PKT_IN for packet(%p), mbuf %p, switch %p\n", pkt, mbuf, sw);
		//Store packet in the storage system. Packet is NOT returned to the bufferpool
		id = dps->store_packet(pkt);

		if(id == datapacket_storage::ERROR){
			ROFL_DEBUG("PKT_IN for packet(%p) could not be stored in the storage. Dropping..\n",pkt);

			//Return mbuf to the pool
			rte_pktmbuf_free(mbuf);

			//Return to the bufferpool
			bufferpool::release_buffer(pkt);
			continue;
		}

		//Normalize size
		pkt_size = get_buffer_length_dpdk(pkt_dpdk);
		if(pkt_size > sw->pipeline.miss_send_len)
			pkt_size = sw->pipeline.miss_send_len;
			
		//Process packet in
		rv = cmm_process_of1x_packet_in(sw->dpid, 
						pkt_dpdk->pktin_table_id, 	
						pkt_dpdk->pktin_reason, 	
						pkt_dpdk->in_port, 
						id, 	
						get_buffer_dpdk(pkt_dpdk), 
						pkt_size,
						get_buffer_length_dpdk(pkt_dpdk),
						((packet_matches_t*)&pkt->matches)
				);

		if( unlikely( rv != AFA_SUCCESS ) ){
			ROFL_DEBUG("PKT_IN for packet(%p) could not be sent to sw:%s controller. Dropping..\n",pkt,sw->name);
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
	// Note that *no more packets* will enqueued in the global
	// PKT_IN queue, since ports shall be descheduled.
	//
	// Strategy is just to make sure all current pending PKT_INs
	// are processed and return
	//

	//Check current credits and current counter
	int curr_pending_credits;
	unsigned int curr_processed_pktins;
	bool overflow;
	
	//Signal PKT_IN thread that we need to read its status atomically
	destroying_sw = true;
	pthread_mutex_lock(&pktin_mutex);

	//Read it
	while(sem_getvalue(&pktin_sem, &curr_pending_credits) != 0);
	curr_processed_pktins = processed_pkt_ins;

	//Prevent other iterations to block
	destroying_sw = false;

	//Release PKT_IN thread
	pthread_mutex_unlock(&pktin_mutex);
	
	//Wait for packets to be drained
	overflow = (curr_processed_pktins+curr_pending_credits) < curr_processed_pktins;  
	curr_processed_pktins += curr_pending_credits;
	while( (overflow && (curr_processed_pktins&0x8000) ) || (curr_processed_pktins < processed_pkt_ins) ){ usleep(250);};	
}

// Launch pkt in thread
rofl_result_t pktin_dispatcher_init(){

	keep_on_pktins = true;

#if 0
	if( pthread_cond_init(&pktin_cond, NULL) < 0 ){
		ROFL_ERR("Unable to create pthread cond\n");
		return ROFL_FAILURE;
	}
	
	if( pthread_mutex_init(&pktin_mutex, NULL) < 0 ){
		ROFL_ERR("Unable to initialize pthread mutex\n");
		return ROFL_FAILURE;
	}	
#endif
	if(sem_init(&pktin_sem, 0,0) < 0){
		return ROFL_FAILURE;
	}	

	//PKT_IN queue
	pkt_ins = rte_ring_create("PKT_IN_RING", IO_PKT_IN_STORAGE_MAX_BUF, SOCKET_ID_ANY, 0x0);
	
	if(!pkt_ins){
		ROFL_ERR("Unable to create PKT_INs ring queue\n");
		return ROFL_FAILURE;
	}

	//Reset global PKT_IN counter (used on wait for draining)
	processed_pkt_ins = 0;
	destroying_sw = false;	

	//Launch thread
	//XXX: use rte?
	if(pthread_create(&pktin_thread, NULL, process_packet_ins, NULL)<0){
		ROFL_ERR("pthread_create failed, errno(%d): %s\n", errno, strerror(errno));
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
