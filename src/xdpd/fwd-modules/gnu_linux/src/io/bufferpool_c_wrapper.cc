/*
 * bufferpool_c_wrapper.cc
 *
 *      Author: msune
 */

#include "bufferpool_c_wrapper.h"
#include "bufferpool.h"

void bufferpool_release_buffer_wrapper(datapacket_t* pkt){
	bufferpool::release_buffer(pkt);
}	

datapacket_t* bufferpool_get_buffer_wrapper(){
	return bufferpool::get_free_buffer();
}

void bufferpool_init_wrapper(long long unsigned int capacity){
	bufferpool::init(capacity);
}

void bufferpool_destroy_wrapper(){
	bufferpool::destroy();
}
