#include "bufferpool.h"

#include "datapacketx86.h"
#include "../config.h"

using namespace xdpd::gnu_linux;

/* Static member initialization */
bufferpool* bufferpool::instance = NULL;
pthread_mutex_t bufferpool::mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t bufferpool::cond = PTHREAD_COND_INITIALIZER;
 
//Constructor and destructor
bufferpool::bufferpool(void){
	long long unsigned int i;
	datapacket_t* dp;
	datapacketx86* dpx86;
	bpool_slot_t *pslot;
	
	cq = new circular_queue<bpool_slot_t>(capacity);
	memset(pool,0,sizeof(pool));

	for(i=0;i<capacity;++i){

		//Init datapacket
		dp = (datapacket_t*)malloc(sizeof(datapacket_t));
		
		if(!dp){
			throw "Unable to allocate bufferpool; out of memory.";
		}

		//Memset datapacket
		memset(dp,0,sizeof(*dp));
		
		//Init datapacketx86
		try {
			dpx86 = new datapacketx86(dp);
		}catch(std::bad_alloc ex){
			throw "Unable to allocate bufferpool; out of memory.";
		}		

		//Assign the buffer_id
		dp->id = i;
				
		//Link them
		dp->platform_state = (platform_datapacket_state_t*)dpx86;

		//Add to the pool
		pslot = &pool[i];
		pslot->status = BUFFERPOOL_SLOT_AVAILABLE;
		pslot->pkt = dp;
			
		//assign to queue
		if ( cq->non_blocking_write(pslot) != ROFL_SUCCESS ){
			fprintf(stderr, "********************  Insertion Failed!\n" );
			delete dpx86;
			free(dp);
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

	//for(i=0;i<capacity;++i){
	while(cq->is_empty()==false){
		pslot = cq->non_blocking_read();
		if (pslot==NULL){
			continue;
		}
		//checks for pkt==NULL, and platform_state==NULL?
		delete (datapacketx86*)pslot->pkt->platform_state;
		free(pslot->pkt);
		i++;
	}
	
	delete cq;
	
	//check that no buffer was lost
	assert(i == capacity-1);
	
}
