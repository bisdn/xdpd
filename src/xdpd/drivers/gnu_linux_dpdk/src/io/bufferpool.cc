#include "bufferpool.h"
#include "dpdk_datapacket.h"

using namespace xdpd::gnu_linux;

/* Static member initialization */
bufferpool* bufferpool::instance = NULL;
pthread_mutex_t bufferpool::mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t bufferpool::cond = PTHREAD_COND_INITIALIZER;
 
//Constructor and destructor
bufferpool::bufferpool(long long unsigned int pool_items)
	: pool(pool_items),
	pool_status(pool_items)
{

	long long unsigned int i;
	datapacket_t* dp;
	//datapacketx86* dpx86;
	datapacket_dpdk_t* dp_dpdk;	

	for(i=0;i<pool_items;++i){

		//Init datapacket
		dp = (datapacket_t*)malloc(sizeof(datapacket_t));
	
		if(!dp){
			//Mark as unavailable
			pool[i] = NULL;
			pool_status[i] = BUFFERPOOL_SLOT_UNAVAILABLE;

			//Skip
			continue;
		}
		
		dp_dpdk = create_datapacket_dpdk(dp);
		
		if(!dp_dpdk){
			//Mark as unavailable
			pool[i] = NULL;
			pool_status[i] = BUFFERPOOL_SLOT_UNAVAILABLE;
			
			free(dp);
			//Skip
			continue;
		}

		//Memset datapacket
		memset(dp,0,sizeof(*dp));

		//Assign the buffer_id
		dp_dpdk->internal_buffer_id = i;			
		dp_dpdk->buffer_id = 0; //Mark as 0
		
		//Link them
		dp->platform_state = (platform_datapacket_state_t*)dp_dpdk;

		//Add to the pool	
		pool[i] = dp;
		pool_status[i] = BUFFERPOOL_SLOT_AVAILABLE;
	
	}

	//Set size
	pool_size = pool_items;
	next_index = 0;
#ifdef DEBUG
	used = 0;
#endif
}

bufferpool::~bufferpool(){
	
	unsigned long long int i;
	datapacket_dpdk_t* dp_dpdk;

	for(i=0;i<pool.size();++i){
		if(pool[i]){
			dp_dpdk = (datapacket_dpdk_t*)pool[i]->platform_state;
			destroy_datapacket_dpdk(dp_dpdk);
			free(pool[i]);
		}	
	}
}



//
// Buffer pool management
//
void bufferpool::init(long long unsigned int capacity){
	

	pthread_mutex_lock(&bufferpool::mutex);		

	//Add the RESERVED_SLOTS to the capacity 
	capacity += RESERVED_SLOTS;

	if(bufferpool::instance){
		//Double-call to init??
		ROFL_DEBUG(DRIVER_NAME"[bufferpool] Double call to bufferpool init!! Skipping...\n");
		pthread_mutex_unlock(&bufferpool::mutex);
		return;	
	}
	
	ROFL_DEBUG(DRIVER_NAME"[bufferpool] Initializing bufferpool with a capacity of %d buffers\n",capacity);

	//Init 	
	bufferpool::instance = new bufferpool(capacity);

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

/*
* Resizes current buffer pool to new_capacity, to the final size of new_capacity+RESERVED_SLOTS
*/
void bufferpool::increase_capacity(long long unsigned int new_capacity){
	assert(0);
}
