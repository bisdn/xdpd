/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H 

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <rofl/datapath/pipeline/common/datapacket.h>

/**
* @file bufferpool.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Data packet buffer pool management 
*
*/

typedef enum{
	BUFFERPOOL_SLOT_UNAVAILABLE=0,
	BUFFERPOOL_SLOT_AVAILABLE=1,
	BUFFERPOOL_SLOT_IN_USE=2
}bufferpool_slot_state_t;

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
	static datapacket_t* get_free_buffer(bool blocking = true);
	static inline datapacket_t* get_free_buffer_nonblocking() {
		return get_free_buffer(false);
	}

	static void release_buffer(datapacket_t* buf);
	
	static void destroy();
	
protected:

	//Singleton instance
	static bufferpool* instance;
	
	//Pool internals
	std::vector<datapacket_t*> pool;	
	std::vector<bufferpool_slot_state_t> pool_status;	
	long long unsigned int pool_size; //This might be different from pool.size during initialization/resizing

	//Mutex and cond
	static pthread_mutex_t mutex;
	static pthread_cond_t cond;

	//Constructor and destructor
	bufferpool(long long unsigned int pool_items=DEFAULT_SIZE);
	~bufferpool();

	//get instance
	static bufferpool* get_instance(void);
};

#endif /* BUFFERPOOL_H_ */
