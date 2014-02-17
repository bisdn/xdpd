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
#include <iostream>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/common/utils/c_logger.h>
#include "../util/likely.h"

//Profiling
#include "../util/time_measurements.h"

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
	//Init 
	static void init(void);

	//Public interface of the pool (static)
	static inline datapacket_t* get_free_buffer_nonblocking(void);

	static inline void release_buffer(datapacket_t* buf);
	
	static void destroy();

	//Only used in debug	
	friend std::ostream&
	operator<< (std::ostream& os, bufferpool const& bp) {
		os << "<bufferpool: ";
			os << "pool-capacity:" << bp.capacity << " ";
			os << "next-index:" << bp.curr_index << " ";
			for (long long unsigned int i = 0; i < bp.capacity; i++) {
				if (bp.pool_status[i] == BUFFERPOOL_SLOT_AVAILABLE)
					os << ".";
				else if (bp.pool_status[i] == BUFFERPOOL_SLOT_IN_USE)
					os << "b";
				else
					os << "u";
				if (((i+1) % 8) == 0)
					os << " ";
				if (((i+1) % 32) == 0)
					os << "  ";
				if (((i+1) % 128) == 0)
					os << std::endl;
			}
		os << ">";
		return os;
	};

	static void dump_state(void);
	static void dump_slots(void);

protected:

	//Singleton instance
	static bufferpool* instance;
	
	//Pool internals
	static const long long unsigned int capacity=IO_BUFFERPOOL_RESERVOIR+IO_BUFFERPOOL_CAPACITY;
	datapacket_t* pool[IO_BUFFERPOOL_RESERVOIR+IO_BUFFERPOOL_CAPACITY];
	bufferpool_slot_state_t pool_status[IO_BUFFERPOOL_RESERVOIR+IO_BUFFERPOOL_CAPACITY];
	long long unsigned int curr_index; //Next item index

#ifdef DEBUG
	long long unsigned int used;
#endif

	//Mutex and cond
	static pthread_mutex_t mutex;
	static pthread_cond_t cond;

	//Constructor and destructor
	bufferpool(void);
	~bufferpool();

	//get instance
	static inline bufferpool* get_instance(void);
};

/*
* Protected static singleton instance
* Note that bufferpool cannot be self initialize
* so we will wait until someone calls init()
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
* Retreives an available buffer.
*/
datapacket_t* bufferpool::get_free_buffer_nonblocking(){

	long long unsigned int i, initial_index;
	
	bufferpool* bp = get_instance();

	i = initial_index = bp->curr_index;

	//Loop all the bufferpool size
	do{
		if(bp->pool_status[i] == BUFFERPOOL_SLOT_AVAILABLE){
			if(__sync_bool_compare_and_swap(&bp->pool_status[i], BUFFERPOOL_SLOT_AVAILABLE, BUFFERPOOL_SLOT_IN_USE) == true){
#ifdef DEBUG
				__sync_fetch_and_add(&bp->used, 1);
#endif
				//Set current index and increment circularly
				if( (i+1) != bp->capacity )
					bp->curr_index = i+1;
				else
					bp->curr_index = 0;

				return bp->pool[i];
			}
		}
	
		//Circular increment
		if( (i++) == bp->capacity ){
			i = 0;
		}

	}while(i != initial_index);

	return NULL;
}

/*
* Releases a previously acquired buffer.
*/
void bufferpool::release_buffer(datapacket_t* buf){

	bufferpool* bp = get_instance();

	unsigned int id = buf->id; 
	
	//Release
	if( unlikely(bp->pool_status[id] != BUFFERPOOL_SLOT_IN_USE) ){
		//Attempting to release an unallocated/unavailable buffer
		ROFL_ERR(FWD_MOD_NAME"[bufferpool] Attempting to release an unallocated/unavailable buffer (pkt:%p). Ignoring..\n",buf);
		assert(0);
	}else{ 
		buf->is_replica = false; //Make sure this flag is 0
		bp->pool_status[id] = BUFFERPOOL_SLOT_AVAILABLE;
#ifdef DEBUG
		__sync_fetch_and_sub(&bp->used, 1);
#endif
	}
}

}// namespace xdpd::gnu_linux 
}// namespace xdpd


#endif /* BUFFERPOOL_H_ */
