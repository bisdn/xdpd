/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRINGBUFFER_H
#define CRINGBUFFER_H 1

#include <string.h>
#include <pthread.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "ringbuffer_c_wrapper.h"

/*
 * Implementation of a ring buffer
 */

//#define RB_ASM_IMP 1 
#define RB_MULTI_READERS 1 
#define RB_MULTI_WRITERS 1 

typedef enum
{
	RB_BUFFER_AVAILABLE = 0,
	RB_BUFFER_FULL = 1,
	RB_BUFFER_CRITICALLY_FULL = 2,
	RB_BUFFER_INVALID = -1,
} ringbuffer_state_t;

class ringbuffer
{
public:
	//Constructor
	ringbuffer(void);
	~ringbuffer(void);
	
	//Read
	datapacket_t* non_blocking_read(void);
	datapacket_t* blocking_read(unsigned int seconds=0);

	//Write
	rofl_result_t non_blocking_write(datapacket_t* pkt);
	rofl_result_t blocking_write(datapacket_t* pkt, unsigned int seconds=0);

	//Get buffer state
	inline ringbuffer_state_t get_buffer_state(void)
	{
		return state;
	}

	inline unsigned int size(void)
	{
		return (writep >= readp) ? (writep-readp) : MAX_SLOTS - (readp-writep);
	}

	bool
	is_empty(void);

	//Some constants
	static const unsigned int exponent = 11; // todo this could be configureable
	static const unsigned int MAX_SLOTS = 1 << exponent;
	static const unsigned int SLOT_MASK = MAX_SLOTS - 1;

private:
	//MAX slots. Should be power of 2	
	static const float FULL_LIMIT = 0.75;
	static const float CRITICALLY_FULL_LIMIT = 0.95;

	//Buffer
	datapacket_t* buffer[MAX_SLOTS];
	/*volatile*/
	//unsigned int elements;

	//Buffer state 
	ringbuffer_state_t state;

	unsigned int full_limit;
	unsigned int critical_full_limit;

	/*volatile*/
	datapacket_t** writep;
	/*volatile*/
	datapacket_t** readp;

	//Condition for blocking calls
	pthread_cond_t write_cond;
	pthread_cond_t read_cond;

	//Only used when multi-readers enabled
	pthread_mutex_t mutex_readers;
	pthread_mutex_t mutex_writers;

	/* Private methods */
	bool is_full(void);
	void update_buffer_state();
	void circ_inc_pointer(datapacket_t*** pointer);

};

//Utility funcs
inline bool
ringbuffer::is_full()
{
	// todo would perform slightly better using "int readpos,writepos"
	return ((size_t)(writep + 1 - buffer) & SLOT_MASK) == (size_t)(readp - buffer);
}

inline bool ringbuffer::is_empty()
{
	return writep == readp;
}

inline void ringbuffer::circ_inc_pointer(datapacket_t*** pointer)
{
	if ((buffer + MAX_SLOTS) == (*pointer) + 1) {
		*pointer = &buffer[0];
	} else {
		(*pointer)++;
	}
}

inline void ringbuffer::update_buffer_state()
{
	// todo reverse logic
	if (size() >= critical_full_limit) state = RB_BUFFER_CRITICALLY_FULL;
	else if (size() >= full_limit) state = RB_BUFFER_FULL;
	else state = RB_BUFFER_AVAILABLE;
}

#endif /* CRINGBUFFER_H_ */
