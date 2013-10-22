#include "pktin_dispatcher.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>

#include "bufferpool.h"
#include "datapacketx86.h"
#include "datapacket_storage.h"
#include "dpdk_datapacket.h"

pthread_mutex_t pktin_mutex;
pthread_cond_t pktin_cond;
struct rte_ring* pkt_ins;
bool keep_on_pktins;

using namespace xdpd::gnu_linux;
using namespace xdpd::gnu_linux_dpdk;

//MBUF pool
extern struct rte_mempool* pool_direct;

//Process packet_ins
static void process_packet_ins(){

	datapacket_t* pkt;
	datapacketx86* pkt_x86;
	afa_result_t rv;
	unsigned int pkt_size;
	storeid id;
	of1x_switch_t* sw;	
	datapacket_storage* dps;
	struct rte_mbuf* mbuf;

	//prepare timeout
	struct timespec timeout;

	while(likely(keep_on_pktins)){

		while(1){
			if(rte_ring_sc_dequeue(pkt_ins, (void**)&pkt) != 0)
				break;

			//Recover platform state
			pkt_x86 = ((dpdk_pkt_platform_state_t*)pkt->platform_state)->pktx86;
			mbuf = ((dpdk_pkt_platform_state_t*)pkt->platform_state)->mbuf;
			sw = (of1x_switch_t*)pkt->sw;
			dps = (datapacket_storage*)pkt->sw->platform_state;

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
			pkt_size = pkt_x86->get_buffer_length();
			if(pkt_size > sw->pipeline->miss_send_len)
				pkt_size = sw->pipeline->miss_send_len;
				
			//Process packet in
			rv = cmm_process_of1x_packet_in(sw, 
							pkt_x86->pktin_table_id, 	
							pkt_x86->pktin_reason, 	
							pkt_x86->in_port, 
							id, 	
							pkt_x86->get_buffer(), 
							pkt_size,
							pkt_x86->get_buffer_length(),
							*((of1x_packet_matches_t*)&pkt->matches)
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
	
				
		//Since the queue does not provide a blocking "dequeue"
		//There can be an enqueue here (race condition), but since
		//anyway we have to do a timedout pthread_cond_wait, we don't care
		//Note: be careful with HIGH timeouts here
		
		pthread_mutex_lock(&pktin_mutex);
		pthread_cond_timedwait(&pktin_cond, &pktin_mutex, &timeout);
		pthread_mutex_unlock(&pktin_mutex);
	}
}


// Launch pkt in thread
rofl_result_t pktin_dispatcher_init(){

	keep_on_pktins = true;

	if( pthread_cond_init(&pktin_cond, NULL) < 0 ){
		ROFL_ERR("Unable to create pthread cond\n");
		return ROFL_FAILURE;
	}
	
	if( pthread_mutex_init(&pktin_mutex, NULL) < 0 ){
		ROFL_ERR("Unable to initialize pthread mutex\n");
		return ROFL_FAILURE;
	}	

	//PKT_IN queue
	pkt_ins = rte_ring_create("PKT_IN_RING", IO_PKT_IN_STORAGE_MAX_BUF, SOCKET_ID_ANY, 0x0);
	
	if(!pkt_ins){
		ROFL_ERR("Unable to create PKT_INs ring queue\n");
		return ROFL_FAILURE;
	}

	//Launch thread
	//XXX
	process_packet_ins();

	return ROFL_SUCCESS;
}

//Stop and destroy packet in dispatcher
rofl_result_t pktin_dispatcher_destroy(){
	
	keep_on_pktins = false;
	
	//Wait for thread to stop
	//XXX
	
	//Destroy ring?
	//XXX: not available??
	
	return ROFL_SUCCESS;
}
