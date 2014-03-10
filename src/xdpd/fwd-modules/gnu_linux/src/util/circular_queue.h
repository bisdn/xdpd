/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H 1

#include <string.h>
#include <pthread.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/common/utils/c_logger.h>
#include <stdio.h>
#include <unistd.h>

#include "likely.h"

/*
 * Implementation of a ring elements
 */

namespace xdpd {
namespace gnu_linux {

//#define PTHREAD_IMP 1 

//Exception
class eCircularQueueInvalidSize{};

template<typename T>
class circular_queue{

public:
	//Constructor
	circular_queue(long long unsigned int capacity);
	~circular_queue(void);
	
	//Read
	inline T* non_blocking_read(void);
	//inline T* blocking_read(unsigned int seconds=0);

	//Write
	inline rofl_result_t non_blocking_write(T* elem);
	//inline rofl_result_t blocking_write(T* elem, unsigned int seconds=0);

	//
	inline unsigned int size(void)
	{
		return (writep >= readp) ? (writep-readp) : slots - (readp-writep);
	}

	inline bool is_empty(void);
	inline bool is_empty(T*** write, T*** read);
	inline bool is_full(void);
	inline bool is_full(T*** write, T*** read);

	long long unsigned int slots;

	void dump(void)  __attribute__((used));
private:
	//Buffer
	T** elements;

	T** writep;
	T** _writep; //used only between writers (assembly 2 stage commit)
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
inline bool circular_queue<T>::is_full(T*** write, T*** read){
	return ((size_t)(*write + 1 - elements) & (slots-1)) == (size_t)(*read - elements);
}

template<typename T>
inline bool circular_queue<T>::is_empty(){
	return writep == readp;
}

template<typename T>
inline bool circular_queue<T>::is_empty(T*** write, T*** read){
	return *write == *read;
}

template<typename T>
inline T** circular_queue<T>::circ_inc_pointer(T** pointer){
	if ((elements + slots) == (pointer + 1) )
		return &elements[0];
	else
		return pointer+1;
}

template<typename T>
circular_queue<T>::circular_queue(long long unsigned int capacity){


	if( ( (capacity & (capacity - 1)) != 0 ) || capacity == 0){
		//Not power of 2!!
		ROFL_ERR("Unable to instantiate queue of size: %u. It is not power of 2! Revise your settings",capacity);
		throw eCircularQueueInvalidSize();
	}

	//Allocate
	elements = new T*[capacity];
	slots = capacity;

	//Set 0 structure
	memset(elements, 0, sizeof(T*)*capacity);

	//Set pointers
	writep = _writep = readp = elements;

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
	T** next, **read_cpy;
	T* to_return;

	do{
		//Recover current read pointer: MUST BE HERE
		read_cpy = readp;

		if (unlikely(is_empty(&writep, &read_cpy))) {
			return NULL;
		}
		
		to_return = *read_cpy;
		
		//Calculate readp+1
		next = circ_inc_pointer(read_cpy); 

	//Try to set it atomically
	}while(__sync_bool_compare_and_swap(&readp, read_cpy, next) != true);

	return to_return;
#else	
	T* elem;
	
	pthread_mutex_lock(&mutex_readers);
	
	if (is_empty()) {
		pthread_mutex_unlock(&mutex_readers);
		return NULL;
	}

	elem = *readp;
	//*readp = NULL;
	readp = circ_inc_pointer(readp);

	pthread_mutex_unlock(&mutex_readers);
	
	return elem;
#endif
}


//Write
template<typename T>
rofl_result_t circular_queue<T>::non_blocking_write(T* elem){

#ifndef PTHREAD_IMP

	T** next, **write_cpy;

	//Increment
	do{

		//Calculate writep+1: MUST BE HERE
		write_cpy = _writep;

		if(unlikely(is_full(&write_cpy, &readp))) {
			return ROFL_FAILURE;
		}
		
		next = circ_inc_pointer(write_cpy); 

	//Try to set writers only index atomically
	}while(__sync_bool_compare_and_swap(&_writep, write_cpy, next) != true);

	*write_cpy = elem;

	//Two stage commit, now let readers read it 
	while(__sync_bool_compare_and_swap(&writep, write_cpy, next) != true);

	return ROFL_SUCCESS;

#else

	pthread_mutex_lock(&mutex_writers);

	if (is_full()) {
		pthread_mutex_unlock(&mutex_writers);
		return ROFL_FAILURE;
	}

	*writep = elem;
	writep = circ_inc_pointer(writep);

	pthread_mutex_unlock(&mutex_writers);

	return ROFL_SUCCESS;
#endif
}


template<typename T>
void circular_queue<T>::dump(void){
	for(long long unsigned int i=0; i<slots;++i){
		if(readp == elements+i)
			ROFL_INFO(">");
		if(writep == elements+i)
			ROFL_INFO("=||");
		ROFL_INFO("[%llu:%p],", i, elements[i]);
		if(i%10 == 0)
			ROFL_INFO("\n");
	}
	ROFL_INFO("\n");
}

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* CIRCULAR_QUEUE_H_ */
