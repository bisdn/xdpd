#include "bufferpool.h"
#include "dpdk_datapacket.h"
#include "../config.h"
#include <stdexcept>

using namespace xdpd::gnu_linux;

/* Static member initialization */
bufferpool* bufferpool::instance = NULL;
pthread_mutex_t bufferpool::mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t bufferpool::cond = PTHREAD_COND_INITIALIZER;
 
//Constructor and destructor
bufferpool::bufferpool(void)
{
	long long unsigned int i;
	datapacket_t* dp;
	datapacket_dpdk_t* dp_dpdk;
	bpool_slot_t *pslot;

	cq = new circular_queue<bpool_slot_t>(capacity);
	memset(pool,0,sizeof(pool));

	for(i=0;i<capacity-1;++i){

		//Init datapacket
		dp = (datapacket_t*)malloc(sizeof(datapacket_t));
		
		if(!dp){
			throw std::runtime_error("Unable to allocate bufferpool; out of memory.");
		}

		//Memset datapacket
		memset(dp,0,sizeof(*dp));
		
		//Init datapacketx86
		dp_dpdk = create_datapacket_dpdk(dp);
		if(!dp_dpdk){
			throw std::runtime_error("Unable to allocate bufferpool; out of memory.");
		}

		//Assign the buffer_id
		dp->id = i;			
				
		//Link them
		dp->platform_state = (platform_datapacket_state_t*)dp_dpdk;

		//Add to the pool
		pslot = &pool[i];	
		pslot->status = BUFFERPOOL_SLOT_AVAILABLE;
		pslot->pkt = dp;

		//assign to queue
                if ( cq->non_blocking_write(pslot) != ROFL_SUCCESS ){
                        destroy_datapacket_dpdk(dp_dpdk);
                        free(dp);
                        throw std::runtime_error("Insertion in bufferpool failed at initialization.");
                } 
	}

	//Set size
#ifdef DEBUG
	used = 0;
#endif
}

bufferpool::~bufferpool(){
	
	unsigned long long int i=0;
	bpool_slot_t *pslot;

	while(cq->is_empty()==false){
		pslot = cq->non_blocking_read();
                if (pslot==NULL){
                        continue;
                }
		
		if (pslot->pkt)
			destroy_datapacket_dpdk((datapacket_dpdk_t*)pslot->pkt->platform_state);
		free(pool[i].pkt);
		i++;
	}

	delete cq;

        //check that no buffer was lost
        assert(i == capacity-1);
}



