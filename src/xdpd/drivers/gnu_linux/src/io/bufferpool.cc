#include "bufferpool.h"

#include "datapacketx86.h"
#include "../config.h"

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
	datapacketx86* dpx86;

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
		pool[i].status = BUFFERPOOL_SLOT_AVAILABLE;
		pool[i].pkt = dp;
		if(i<capacity)
			pool[i].next = &pool[i+1];
		else
			pool[i].next = NULL;
			
	}

	//Set head
	free_head = &pool[0];

	//Set size
#ifdef DEBUG
	used = 0;
#endif
}

bufferpool::~bufferpool(){
	
	unsigned long long int i;

	for(i=0;i<capacity;++i){
		delete (datapacketx86*)pool[i].pkt->platform_state;
		free(pool[i].pkt);
	}
}



//
// Buffer pool management
//
void bufferpool::init(){
	
	pthread_mutex_lock(&bufferpool::mutex);		

	if(bufferpool::instance){
		//Double-call to init??
		ROFL_DEBUG(DRIVER_NAME"[bufferpool] Double call to bufferpool init!! Skipping...\n");
		pthread_mutex_unlock(&bufferpool::mutex);
		return;	
	}
	
	ROFL_DEBUG(DRIVER_NAME"[bufferpool] Initializing bufferpool with a capacity of %d buffers...\n",capacity);

	//Init 	
	bufferpool::instance = new bufferpool();
	
	ROFL_DEBUG(DRIVER_NAME"[bufferpool] Initialization was successful\n");

	//Wake consumers
	pthread_cond_broadcast(&bufferpool::cond);

	//Release and go!	
	pthread_mutex_unlock(&bufferpool::mutex);		
}

void bufferpool::destroy(){

	if(get_instance())
		delete get_instance();

	instance = NULL;	
}
