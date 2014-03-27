/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H 

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/common/utils/c_logger.h>
#include "../util/likely.h"
#include "dpdk_datapacket.h"

/**
* @file bufferpool.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Data packet buffer pool management 
*
*/

namespace xdpd {
namespace gnu_linux {


typedef enum{
	BUFFERPOOL_SLOT_UNAVAILABLE=0,
	BUFFERPOOL_SLOT_AVAILABLE=1,
	BUFFERPOOL_SLOT_IN_USE=2
}bufferpool_slot_state_t;

/**
* @brief I/O subsystem datapacket buffer pool management class
*
* @ingroup fm_gnu_linux_io
*/
class bufferpool{

public:
	/**
	* RESERVED_SLOTS are the slots not meant to be used by ports, but likely
	* used in other situations (such PACKET_INs).
	* Ports instead should increase_capacity of the bufferpool once they are
	* scheduled in the I/O (if the pool is not properly sized yet). 
	*/
	static const unsigned int RESERVED_SLOTS = 1024*2; //2048 items

	/**
	* Initialization default size of the buffer pool; must be >= RESERVED_SIZE
	* For performance make this power of 2.	
	*/
	static const unsigned int DEFAULT_SIZE = RESERVED_SLOTS*2;

	//Init 
	static void init(long long unsigned int capacity=DEFAULT_SIZE);
	static void increase_capacity(long long unsigned int new_capacity);

	//Public interface of the pool (static)
	static inline datapacket_t* get_free_buffer(bool blocking = true);
	static inline datapacket_t* get_free_buffer_nonblocking() {
		return get_free_buffer(false);
	}

	static inline void release_buffer(datapacket_t* buf);
	
	static void destroy();

	
protected:

	//Singleton instance
	static bufferpool* instance;
	
	//Pool internals
	std::vector<datapacket_t*> pool;	
	std::vector<bufferpool_slot_state_t> pool_status;	
	long long unsigned int pool_size; //This might be different from pool.size during initialization/resizing
	long long unsigned int next_index; //Next item index

#ifdef DEBUG
	long long unsigned int used;
#endif

	//Mutex and cond
	static pthread_mutex_t mutex;
	static pthread_cond_t cond;

	//Constructor and destructor
	bufferpool(long long unsigned int pool_items=DEFAULT_SIZE);
	~bufferpool();

	//get instance
	static inline bufferpool* get_instance(void);
};

/*
* Protected static singleton instance
*/
bufferpool* bufferpool::get_instance(void){

	//Be lock-less once initialized	
	if(unlikely(bufferpool::instance == NULL)){
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

	long long unsigned int i, initial_index;
	
	
	bufferpool* bp = get_instance();

	i = initial_index = bp->next_index;

	//Loop all the bufferpool size
	for(;;){ 

		//Trying to minimize locking.	
		if(bp->pool_status[i] == BUFFERPOOL_SLOT_AVAILABLE){
			
			//Take mutex
			pthread_mutex_lock(&bufferpool::mutex);		
	
			//Recheck
			if(likely(bp->pool_status[i] == BUFFERPOOL_SLOT_AVAILABLE)){
				//Mark as in-use		
				bp->pool_status[i] = BUFFERPOOL_SLOT_IN_USE;
	
				if( unlikely( (bp->next_index+1) == bp->pool_size) )
					bp->next_index = 0;
				else
					bp->next_index++;

#ifdef DEBUG
				bp->used++;
#endif
	
				//Release
				pthread_mutex_unlock(&bufferpool::mutex);		
			
				//return buffer 
				return bp->pool[i];
			}else{
				//There has been an undesired scheduling of threads 
				pthread_mutex_unlock(&bufferpool::mutex);		
			}
		}
	
		//Circular increment
		if( unlikely( (i+1) == bp->pool_size) )
			i = 0;
		else
			i++;
		
		//Wait in cond variable before restarting from the first item of the pool
		//if blocking, otherwise return NULL (no buffers) 
		if ( unlikely(i == initial_index) ) {
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

	unsigned int id = ((datapacket_dpdk_t*)buf->platform_state)->internal_buffer_id;
	
	//Release
	if( unlikely(bp->pool_status[id] != BUFFERPOOL_SLOT_IN_USE) ){
		//Attempting to release an unallocated/unavailable buffer
		ROFL_ERR(DRIVER_NAME"[bufferpool] Attempting to release an unallocated/unavailable buffer (pkt:%p). Ignoring..\n",buf);
		assert(0);
	}else{ 
		buf->is_replica = false; //Make sure this flag is 0
		bp->pool_status[id] = BUFFERPOOL_SLOT_AVAILABLE;
		pthread_cond_broadcast(&bufferpool::cond);

#ifdef DEBUG
		pthread_mutex_lock(&bufferpool::mutex);		
		bp->used--;
		pthread_mutex_unlock(&bufferpool::mutex);
#endif
	}
}

}// namespace xdpd::gnu_linux 
}// namespace xdpd


#endif /* BUFFERPOOL_H_ */
