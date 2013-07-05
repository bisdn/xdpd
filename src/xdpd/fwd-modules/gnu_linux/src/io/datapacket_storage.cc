/*
 * datapacket_storage.cc
 *
 *  Created on: Jan 17, 2013
 *      Author: tobi
 */

#include "datapacket_storage.h"
#include <assert.h>
#include <rofl/common/thread_helper.h>

datapacket_storage::datapacket_storage(uint16_t size, uint16_t expiration) :
		next_id(1),
		max_size(size),
		expiration_time_sec(expiration)
{
	pthread_mutex_init(&lock, NULL);
}

datapacket_storage::~datapacket_storage()
{
	pthread_mutex_destroy(&lock);
}

storeid
datapacket_storage::store_packet(datapacket_t* pkt)
{	
	if (store.size() >= max_size) {
		return -1;
	}

	store_mapping map;
	map.pkt = pkt;
	map.input_timestamp = time(NULL);

	{
		rofl::Lock scoped_lock(&lock);
		map.id = ++next_id&0x7fffffff; //Prevent using 0xffffffff 
		store.push_back(map);
	}

	return map.id;
}

datapacket_t*
datapacket_storage::get_packet(storeid id)
{
	std::list<store_mapping>::iterator iter;
	store_mapping map = { 0, NULL };

	{
		rofl::Lock scoped_lock(&lock);

		// search the packet
		for (iter = store.begin(); iter != store.end(); ++iter) {
			if ( id == (*iter).id ) {
				map = *iter;
				store.erase(iter);
				break;
			}
		}
	}

	return map.pkt;
}

bool datapacket_storage::oldest_packet_needs_expiration(storeid *id)
{
	double buffer_life_time;
	if(store.empty())
		return false;
	
	buffer_life_time = difftime(time(NULL),store.front().input_timestamp);
	
	if(buffer_life_time>expiration_time_sec)
	{
		*id = store.front().id;
		return true;
	}
	else
	{
		return false;
	}
}

uint16_t
datapacket_storage::get_storage_size() const
{
	return store.size();
}

#ifdef DEBUG
void datapacket_storage::change_expiration_time(uint16_t sec)
{
	expiration_time_sec = sec;
}
#endif
