#include "bufferpool.h"
#include <rofl/common/utils/c_logger.h>
#include "datapacketx86.h"

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
	datapacketx86* dpx86;

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

		//Memset datapacket
		memset(dp,0,sizeof(dp));
		
		//Init datapacketx86
		try {
			dpx86 = new datapacketx86();
		}catch(std::bad_alloc ex){

			//Mark as unavailable
			pool[i] = NULL;
			pool_status[i] = BUFFERPOOL_SLOT_UNAVAILABLE;
			
			free(dp);
			continue;		
		}		

		//Assign the buffer_id
		dpx86->internal_buffer_id = i;			
		dpx86->buffer_id = 0; //Mark as 0
		
		//Link them
		dp->platform_state = (platform_datapacket_state_t*)dpx86;

		//Add to the pool	
		pool[i] = dp;
		pool_status[i] = BUFFERPOOL_SLOT_AVAILABLE;
	
	}

	//Set size
	pool_size = pool_items;
}

bufferpool::~bufferpool(){
	
	unsigned long long int i;

	for(i=0;i<pool.size();++i){
		if(pool[i]){
			delete (datapacketx86*)pool[i]->platform_state;
			free(pool[i]);
		}	
	}
}

/*
* Protected static singleton instance
*/
inline bufferpool* bufferpool::get_instance(void){

	//Be lock-less once initialized	
	if(!bufferpool::instance){
		//Wait init
		pthread_mutex_lock(&bufferpool::mutex);		
		pthread_cond_wait(&bufferpool::cond,&bufferpool::mutex); 
		pthread_mutex_unlock(&bufferpool::mutex);		
	}
	return bufferpool::instance;
}

//Public interface of the pool

/*
* Retreives an available buffer. This method is BLOCKING
*/
datapacket_t* bufferpool::get_free_buffer(bool blocking){

	long long unsigned int i;
	
	bufferpool* bp = get_instance();

	//Trying to minimize locking.	
	for(i=0;; (i+1 == bp->pool_size)? i=0 : ++i){ // fixme this has to be improved since its always searching a free packet from the beginning --> queue
		if(bp->pool_status[i] == BUFFERPOOL_SLOT_AVAILABLE){
			
			//Take mutex
			pthread_mutex_lock(&bufferpool::mutex);		
	
			//Recheck
			if(bp->pool_status[i] != BUFFERPOOL_SLOT_AVAILABLE){
				//There has been an undesired scheduling of threads 
				pthread_mutex_unlock(&bufferpool::mutex);		
				continue;	
			}
			
			//Mark as in-use		
			bp->pool_status[i] = BUFFERPOOL_SLOT_IN_USE;
		
			//Release
			pthread_mutex_unlock(&bufferpool::mutex);		
			
			//return buffer 
			return bp->pool[i];
		}
		
		//Wait in cond variable before restarting from the first item of the pool 
		if (i+1 == bp->pool_size) {
			if (blocking) {
				pthread_mutex_lock(&bufferpool::mutex);
				pthread_cond_wait(&bufferpool::cond,&bufferpool::mutex);
				pthread_mutex_unlock(&bufferpool::mutex);
			} else {
				return NULL;
			}
		}
	}
}

/*
* Releases a previously acquired buffer.
*/
void bufferpool::release_buffer(datapacket_t* buf){

	bufferpool* bp = get_instance();

	unsigned int id = ((datapacketx86*)buf->platform_state)->internal_buffer_id;
	
	//Release
	if(bp->pool_status[id] != BUFFERPOOL_SLOT_IN_USE){
		//Attempting to release an unallocated/unavailable buffer
		ROFL_ERR("Attempting to release an unallocated/unavailable buffer. Ignoring..");
	}else{ 
		buf->is_replica = false; //Make sure this flag is 0
		bp->pool_status[id] = BUFFERPOOL_SLOT_AVAILABLE;
		pthread_cond_broadcast(&bufferpool::cond);
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
		ROFL_DEBUG("Double call to bufferpool init!! Skipping...\n");
		pthread_mutex_unlock(&bufferpool::mutex);
		return;	
	}
	
	ROFL_DEBUG("Initializing bufferpool with a capacity of %d buffers\n",capacity);

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

	long long unsigned int i;
	bufferpool* bp = get_instance();
	bool errors = false;
	
	datapacket_t* dp;
	datapacketx86* dpx86;

	//Add the RESERVED_SLOTS to the new_capacity 
	new_capacity += RESERVED_SLOTS;

	ROFL_DEBUG("Attemtping to resize bufferpool to %d buffers (current %d)\n", new_capacity, bp->pool_size);

	//Check if current capacity is enough
	if(new_capacity <= bp->pool_size)
		return;	
	
	ROFL_DEBUG("Resizing bufferpool to %d...", new_capacity);

	//Resize vectors
	try {
		bp->pool.resize(new_capacity);
		bp->pool_status.resize(new_capacity);
	}catch(std::bad_alloc ex){
		ROFL_DEBUG(" KO. Unable to resize pool vectors.\n");
		return; 
	}		

	//Create buffers	
	for(i=bp->pool_size;i<new_capacity;i++){

		//Init datapacket
		dp = (datapacket_t*)malloc(sizeof(datapacket_t));
		
		if(!dp){
			//Mark as unavailable
			bp->pool[i] = NULL;
			bp->pool_status[i] = BUFFERPOOL_SLOT_UNAVAILABLE;

			//Skip
			errors = true;
			continue;
		}

		//Init datapacketx86
		try {
			dpx86 = new datapacketx86();
		}catch(std::bad_alloc ex){
			//Mark as unavailable
			bp->pool[i] = NULL;
			bp->pool_status[i] = BUFFERPOOL_SLOT_UNAVAILABLE;
			
			free(dp);
			errors = true;
			continue;		
		}		

		//Assign the buffer_id
		dpx86->internal_buffer_id = i;			
		
		//Link them
		dp->platform_state = (platform_datapacket_state_t*)dpx86;

		//Add to the pool	
		bp->pool[i] = dp;
		bp->pool_status[i] = BUFFERPOOL_SLOT_AVAILABLE;
	}
	
	//Finally allow to use extended capacity
	bp->pool_size = new_capacity;
	
	
	if(errors)
		ROFL_DEBUG(" errors while allocating memory (out of memory?) \n");
	else
		ROFL_DEBUG("OK\n", new_capacity);
}
