/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BUFFERPOOL_META_H
#define BUFFERPOOL_META_H 

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/common/utils/c_logger.h>

/**
* @file bufferpool_meta.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Data packet buffer pool 
*
*/

//Define SYNC_FETCH_ADD if not overwritten
#ifndef SYNC_FETCH_ADD
	#define SYNC_FETCH_ADD __sync_fetch_and_add
#endif 
//Define SYNC_FETCH_SUB if not overwritten
#ifndef SYNC_FETCH_SUB
	#define SYNC_FETCH_SUB __sync_fetch_and_sub
#endif 


//Define CAS if not overwritten
#ifndef SYNC_BOOL_CAS
	#define SYNC_BOOL_CAS __sync_bool_compare_and_swap
#endif

//Release hook
#ifndef BUFFERPOOL_PKT_RELEASE_HOOK	
	#define BUFFERPOOL_PKT_RELEASE_HOOK(PKT) do{}while(0)
#endif

namespace xdpd {
namespace gnu_linux {

typedef enum{
	BUFFERPOOL_SLOT_UNAVAILABLE=0,
	BUFFERPOOL_SLOT_AVAILABLE=1,
	BUFFERPOOL_SLOT_IN_USE=2
}bpool_slot_status_t;

typedef struct bpool_slot{
	bpool_slot_status_t status;
	datapacket_t* pkt;	
	struct bpool_slot* next;	
}bpool_slot_t;

/**
* @brief I/O subsystem datapacket buffer pool management class
*
* @ingroup driver_gnu_linux_io
*/
class bufferpool{

public:
	//Init 
	static inline void init(void);

	//Public interface of the pool (static)
	static inline datapacket_t* get_buffer(void);

	static inline void release_buffer(datapacket_t* buf);
	
	static inline void destroy();

	//Only used in debug	
	friend std::ostream&
	operator<< (std::ostream& os, bufferpool const& bp) {
		os << "<bufferpool "<<&bp<<", ";
			os << "capacity:" << bp.capacity << " ";
#ifdef DEBUG
			os << "used:" << bp.used << " ";
#endif
		os << ">";
		return os;
	};

	static inline void dump(void);

protected:

	//Singleton instance
	static bufferpool* instance;
	
	//Pool internals
	static const long long unsigned int capacity=IO_BUFFERPOOL_RESERVOIR+IO_BUFFERPOOL_CAPACITY;
	bpool_slot_t pool[IO_BUFFERPOOL_RESERVOIR+IO_BUFFERPOOL_CAPACITY];
	bpool_slot_t* free_head; 
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
datapacket_t* bufferpool::get_buffer(){

	bpool_slot_t *tmp, *next;
	bufferpool *bp = get_instance();

BUFFERPOOL_META_GET_RETRY:

	tmp = bp->free_head;		
	if(!tmp)
		return NULL;
	
	//Calculate
	next = tmp->next;
	
	//Set the new head 
	if( unlikely(SYNC_BOOL_CAS(&bp->free_head, tmp, next) == false) )
		goto BUFFERPOOL_META_GET_RETRY;

#ifdef DEBUG
	SYNC_FETCH_ADD(&bp->used, 1);

	//Double check
	if( unlikely(tmp->status != BUFFERPOOL_SLOT_AVAILABLE) ){
		//Attempting to release an unallocated/unavailable buffer
		ROFL_ERR(DRIVER_NAME"[bufferpool] Corrupted pool, buffer (pkt:%p).\n", tmp->pkt);
		assert(0);
	}
	
	tmp->status = BUFFERPOOL_SLOT_IN_USE;
#endif
		
	return tmp->pkt;
}

/*
* Releases a previously acquired buffer.
*/
void bufferpool::release_buffer(datapacket_t* pkt){

	bpool_slot_t *prev_head, *new_head;
	unsigned int id = pkt->id; 

	//Get pool instance	
	bufferpool* bp = get_instance();

	//Recover the slot
	new_head = &bp->pool[id];

	//Set is replica = false, if set	
#ifdef BUFFERPOOL_CLEAR_IS_REPLICA
	//Make sure this is set to 0	
	pkt->is_replica = false;
#endif

#ifdef DEBUG
	//Perform basic checkings
	if( unlikely(new_head->status != BUFFERPOOL_SLOT_IN_USE) ){
		//Attempting to release an unallocated/unavailable buffer
		ROFL_ERR(DRIVER_NAME"[bufferpool] Attempting to release an unallocated/unavailable buffer (pkt:%p). Ignoring..\n",pkt);
		assert(0);
	}
	new_head->status = BUFFERPOOL_SLOT_AVAILABLE;
	SYNC_FETCH_SUB(&bp->used, 1);
#endif

	//Call release hook
	BUFFERPOOL_PKT_RELEASE_HOOK(pkt);

	//Proceed to return the buffer to the head of the pool
BUFFERPOOL_META_RELEASE_RETRY:
	prev_head = bp->free_head;
	new_head->next = prev_head;
	
	//Set the new head 
	if( unlikely(SYNC_BOOL_CAS(&bp->free_head, prev_head, new_head) == false) )
		goto BUFFERPOOL_META_RELEASE_RETRY;
}

/*
* Dump the state of the buffer
*/
void bufferpool::dump(){
	bufferpool* bp = get_instance();
	(void)bp;
#ifdef DEBUG
	ROFL_DEBUG("bufferpool at %p, capacity: %llu, used: %llu\n", bp, bp->capacity, bp->used);
#else
	ROFL_DEBUG("bufferpool at %p, capacity: %llu\n", bp, bp->capacity);
#endif //DEBUG 
}


/*
* Buffer pool management
*/
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
	
}// namespace xdpd::gnu_linux 
}// namespace xdpd


#endif /* BUFFERPOOL_META_H_ */
