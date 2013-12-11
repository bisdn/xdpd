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

#include "likely.h"

/*
 * Implementation of a ring elements
 */

namespace xdpd {
namespace gnu_linux {

//#define PTHREAD_IMP 1 

typedef enum{
		RB_BUFFER_AVAILABLE = 0,
		RB_BUFFER_FULL = 1,
		RB_BUFFER_CRITICALLY_FULL = 2,
		RB_BUFFER_INVALID = -1,
}circular_queue_state_t;

template<typename T>
class circular_queue{

public:
	//Constructor
	circular_queue(long long unsigned int capacity);
	~circular_queue(void);
	
	//Read
	inline T* non_blocking_read(void);
	inline T* blocking_read(unsigned int msseconds=0);

	//Write
	inline rofl_result_t non_blocking_write(T* elem);
	inline rofl_result_t blocking_write(T* elem, unsigned int msseconds=0);

	//
	inline unsigned int size(void)
	{
		return (writep >= readp) ? (writep-readp) : slots - (readp-writep);
	}

	inline bool is_empty(void);
	inline bool is_full(void);

	long long unsigned int slots;

private:
	//Buffer
	T** elements;

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
	T** circ_inc_pointer(T** pointer);

};

//Utility funcs
template<typename T>
inline bool circular_queue<T>::is_full(){
	// todo would perform slightly better using "int readpos,writepos"
	return ((size_t)(writep + 1 - elements) & (slots-1)) == (size_t)(readp - elements);
}

template<typename T>
inline bool circular_queue<T>::is_empty(){
	return writep == readp;
}

template<typename T>
inline T** circular_queue<T>::circ_inc_pointer(T** pointer){
	T** return_pointer;

	if ((elements + slots) == (pointer) + 1) {
		return_pointer = &elements[0];
	} else {
		return_pointer = pointer+1;
	}
	
	return return_pointer;
}

template<typename T>
circular_queue<T>::circular_queue(long long unsigned int capacity){

	if(!capacity)
		return;

	//Allocate
	elements = new T*[capacity];
	slots = capacity;

	//Set 0 structure
	memset(elements, 0, sizeof(T*)*capacity);

	//Set pointers
	writep = elements;
	readp = elements;

	//Init conditions
	pthread_cond_init(&write_cond, NULL);
	pthread_cond_init(&read_cond, NULL);

	//Mutexes
	pthread_mutex_init(&mutex_readers, NULL);
	pthread_mutex_init(&mutex_writers, NULL);
}

template<typename T>
circular_queue<T>::~circular_queue(){

	//Destroy
	pthread_cond_destroy(&write_cond);
	pthread_cond_destroy(&read_cond);
	pthread_mutex_destroy(&mutex_readers);
	pthread_mutex_destroy(&mutex_writers);
	
	delete[] elements;
}

//Read
template<typename T>
T* circular_queue<T>::non_blocking_read(void){


#ifndef PTHREAD_IMP
	
	T** pointer, **read_cpy;

	//Increment
	do{
		if (unlikely(is_empty())) {
			return NULL;
		}
		
		//Calulcate readpos+1
		read_cpy = readp;
		pointer = circ_inc_pointer(read_cpy); 

		//Try to set it atomically
	}while(__sync_bool_compare_and_swap(&readp, read_cpy, pointer) != true);

	return *pointer;
#else	
	T* elem;
	
	#ifdef RB_MULTI_READERS
		pthread_mutex_lock(&mutex_readers);
	#endif //RB_MULTI_READERS
		if (is_empty()) {
	#ifdef RB_MULTI_WRITERS
			pthread_mutex_unlock(&mutex_readers);
	#endif //RB_MULTI_WRITERS
			return NULL;
		}

		elem = *readp;
		readp = circ_inc_pointer(readp);

	#ifdef RB_MULTI_READERS
		pthread_mutex_unlock(&mutex_readers);
	#endif //RB_MULTI_READERS

		pthread_cond_broadcast(&write_cond);
		return elem;
#endif
}

template<typename T>
T* circular_queue<T>::blocking_read(unsigned int seconds){

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
template<typename T>
rofl_result_t circular_queue<T>::non_blocking_write(T* elem){

#ifndef PTHREAD_IMP

	T** pointer, **write_cpy;

	//Increment
	do{
		if (unlikely(is_full())) {
			return ROFL_FAILURE;
		}
		
		//Calulcate readpos+1
		write_cpy = writep;
		pointer = circ_inc_pointer(write_cpy); 

		//Try to set it atomically
	}while(__sync_bool_compare_and_swap(&writep, write_cpy, pointer) != true);

	*pointer = elem;

	return ROFL_SUCCESS;

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
		writep = circ_inc_pointer(writep);

	#ifdef RB_MULTI_WRITERS
		pthread_mutex_unlock(&mutex_writers);
	#endif

		pthread_cond_broadcast(&read_cond);

		return ROFL_SUCCESS;
#endif
}

template<typename T>
rofl_result_t circular_queue<T>::blocking_write(T* elem, unsigned int seconds){

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
