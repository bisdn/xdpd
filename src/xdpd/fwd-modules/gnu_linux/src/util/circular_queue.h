/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H 1

#include <string.h>
#include <pthread.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <stdio.h>

/*
 * Implementation of a ring elements
 */

namespace xdpd {
namespace gnu_linux {

//#define RB_ASM_IMP 1 
#define RB_MULTI_READERS 1 
#define RB_MULTI_WRITERS 1 

typedef enum{
		RB_BUFFER_AVAILABLE = 0,
		RB_BUFFER_FULL = 1,
		RB_BUFFER_CRITICALLY_FULL = 2,
		RB_BUFFER_INVALID = -1,
}circular_queue_state_t;

template<typename T, long long unsigned int SLOTS>
class circular_queue{

public:
	//Constructor
	circular_queue(void);
	~circular_queue(void);
	
	//Read
	T* non_blocking_read(void);
	T* blocking_read(unsigned int seconds=0);

	//Write
	rofl_result_t non_blocking_write(T* elem);
	rofl_result_t blocking_write(T* elem, unsigned int seconds=0);

	//Get elements state
	inline circular_queue_state_t get_queue_state(void)
	{
		update_elements_state();
		return state;
	}

	inline unsigned int size(void)
	{
		return (writep >= readp) ? (writep-readp) : SLOTS - (readp-writep);
	}

	bool
	is_empty(void);
	
	bool is_full(void);

	static const long long unsigned int MAX_SLOTS = SLOTS;
	static const long long unsigned int SLOT_MASK = MAX_SLOTS - 1;

private:
	//MAX slots. Should be power of 2	
	static const float FULL_LIMIT = 0.75*SLOTS;
	static const float CRITICALLY_FULL_LIMIT = 0.95*SLOTS;

	//Buffer
	T* elements[SLOTS];

	/*volatile*/
	//unsigned int elements;

	//Buffer state 
	circular_queue_state_t state;

	T** writep;
	T** readp;

	//Condition for blocking calls
	pthread_cond_t write_cond;
	pthread_cond_t read_cond;

	//Only used when multi-readers enabled
	pthread_mutex_t mutex_readers;
	pthread_mutex_t mutex_writers;

	/* Private methods */
	void update_elements_state();
	void circ_inc_pointer(T*** pointer);

};

//Utility funcs

template<typename T, long long unsigned int SLOTS>
inline bool circular_queue<T, SLOTS>::is_full(){
	// todo would perform slightly better using "int readpos,writepos"
	return ((size_t)(writep + 1 - elements) & SLOT_MASK) == (size_t)(readp - elements);
}

template<typename T, long long unsigned int SLOTS>
inline bool circular_queue<T, SLOTS>::is_empty(){
	return writep == readp;
}

template<typename T, long long unsigned int SLOTS>
inline void circular_queue<T, SLOTS>::circ_inc_pointer(T*** pointer){
	if ((elements + SLOTS) == (*pointer) + 1) {
		*pointer = &elements[0];
	} else {
		(*pointer)++;
	}
}

template<typename T, long long unsigned int SLOTS>
inline void circular_queue<T, SLOTS>::update_elements_state(){
	// todo reverse logic
	if (size() >= CRITICALLY_FULL_LIMIT) state = RB_BUFFER_CRITICALLY_FULL;
	else if (size() >= FULL_LIMIT) state = RB_BUFFER_FULL;
	else state = RB_BUFFER_AVAILABLE;
}

template<typename T, long long unsigned int SLOTS>
circular_queue<T, SLOTS>::circular_queue(){

	//Set 0 structure
	memset(&elements, 0, sizeof(elements));

	//Set pointers
	writep = elements;
	readp = elements;

	//Setting set
	state = RB_BUFFER_AVAILABLE;

	//Init conditions
	pthread_cond_init(&write_cond, NULL);
	pthread_cond_init(&read_cond, NULL);

	//Mutexes
	pthread_mutex_init(&mutex_readers, NULL);
	pthread_mutex_init(&mutex_writers, NULL);
}

template<typename T, long long unsigned int SLOTS>
circular_queue<T, SLOTS>::~circular_queue(){

	//Destroy
	pthread_cond_destroy(&write_cond);
	pthread_cond_destroy(&read_cond);
	pthread_mutex_destroy(&mutex_readers);
	pthread_mutex_destroy(&mutex_writers);
}

//Read
template<typename T, long long unsigned int SLOTS>
T* circular_queue<T, SLOTS>::non_blocking_read(void){

	T* elem;

#ifdef RB_ASM_IMP

	//TODO: put a pthread_mutex free imp

#else	
#ifdef RB_MULTI_READERS
	pthread_mutex_lock(&mutex_readers);
#endif
	if (is_empty()) {
#ifdef RB_MULTI_WRITERS
		pthread_mutex_unlock(&mutex_readers);
#endif
		return NULL;
	}

	elem = *readp;
	circ_inc_pointer(&readp); // = (readp + 1) % SLOTS;

#ifdef RB_MULTI_READERS
	pthread_mutex_unlock(&mutex_readers);
#endif

	pthread_cond_broadcast(&write_cond);
	return elem;
#endif
}

template<typename T, long long unsigned int SLOTS>
T* circular_queue<T, SLOTS>::blocking_read(unsigned int seconds){

	T* elem;
	struct timespec timeout;

	//Try it straight away
	elem = non_blocking_read();
	
	while(!elem) {

		//Acquire lock for pthread_cond_wait
		pthread_mutex_lock(&mutex_readers);

		//Sleep until signal or timeout (in case defined)
		if(seconds){
			timeout.tv_sec = time(NULL) + seconds;
			timeout.tv_nsec = 0;
			pthread_cond_timedwait(&read_cond, &mutex_readers,&timeout);
		}else	
			pthread_cond_wait(&read_cond, &mutex_readers);

		//Release it
		pthread_mutex_unlock(&mutex_readers);


		//Retry
		elem = non_blocking_read();
		
		if(seconds)
			//if timeout, then only once needs to be tried and exit
			break;
	
	}
	return elem;
}

//Write
template<typename T, long long unsigned int SLOTS>
rofl_result_t circular_queue<T, SLOTS>::non_blocking_write(T* elem){

#ifdef RB_ASM_IMP

	//TODO: put a pthread_mutex free imp

#else

#ifdef RB_MULTI_WRITERS
	pthread_mutex_lock(&mutex_writers);
#endif

	if (is_full()) {
#ifdef RB_MULTI_WRITERS
		pthread_mutex_unlock(&mutex_writers);
#endif
		return ROFL_FAILURE;
	}

	*writep = elem;
	circ_inc_pointer(&writep); // = (readp + 1) % SLOTS;

#ifdef RB_MULTI_WRITERS
	pthread_mutex_unlock(&mutex_writers);
#endif

	pthread_cond_broadcast(&read_cond);

	return ROFL_SUCCESS;
#endif
}

template<typename T, long long unsigned int SLOTS>
rofl_result_t circular_queue<T, SLOTS>::blocking_write(T* elem, unsigned int seconds){

	rofl_result_t result;
	struct timespec timeout;

	//Try it straight away
	result = non_blocking_write(elem);

	while(result == ROFL_FAILURE) {
	
		//Acquire lock for pthread_cond_wait
		pthread_mutex_lock(&mutex_writers);

		//Sleep until signal or timeout (in case defined)
		if(seconds){
			timeout.tv_sec = time(NULL) + seconds;
			pthread_cond_timedwait(&write_cond, &mutex_writers, &timeout);
		}else
	 		pthread_cond_wait(&write_cond, &mutex_writers);

		//Release it
		pthread_mutex_unlock(&mutex_writers);

		//Retry
		result = non_blocking_write(elem);
		
		if(seconds)
			//if timeout, then only once needs to be tried and exit
			break;
	}

	return result;
}

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* CIRCULAR_QUEUE_H_ */
