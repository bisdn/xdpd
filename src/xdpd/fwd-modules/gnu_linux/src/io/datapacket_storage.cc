#include "datapacket_storage.h"
#include <assert.h>
#include <rofl/common/thread_helper.h>
#include "../util/likely.h"

using namespace xdpd::gnu_linux;

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
	store_mapping map;
	map.pkt = pkt;
	
	pthread_mutex_lock(&lock);
		
	if (store.size() >= max_size) {
		pthread_mutex_unlock(&lock);
		return this->ERROR;
	}

	map.input_timestamp = time(NULL);

	if(unlikely(next_id+1 == 0xffffffff))
		map.id = next_id = 1; //Skip error and 0
	else
		map.id = ++next_id;
	store.push_back(map);

	pthread_mutex_unlock(&lock);

	return map.id;
}

datapacket_t*
datapacket_storage::get_packet(storeid id)
{
	std::list<store_mapping>::iterator iter;
	store_mapping map = { 0, NULL };

	pthread_mutex_lock(&lock);

	// search the packet
	for (iter = store.begin(); iter != store.end(); ++iter) {
		if ( id == (*iter).id ) {
			map = *iter;
			store.erase(iter);
			break;
		}
	}
	
	pthread_mutex_unlock(&lock);

	return map.pkt;
}

bool 
datapacket_storage::oldest_packet_needs_expiration(storeid *id)
{
	double buffer_life_time;
	if(store.empty())
		return false;
	
	buffer_life_time = difftime(time(NULL),store.front().input_timestamp);
	
	if(buffer_life_time>expiration_time_sec){
		*id = store.front().id;
		return true;
	}else{
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



void
datapacket_storage::dump_state()
{
	std::cerr << *this << std::endl;
}


void
datapacket_storage::dump_slots()
{
	for (std::list<store_mapping>::const_iterator
			it = store.begin(); it != store.end(); ++it) {
		store_mapping const& map = (*it);
		std::cerr << "  ";
		std::cerr << "id:" << (int)map.id << " ";
		std::cerr << "input-timestamp:" << (int)map.input_timestamp << " ";
		std::cerr << "pkt:" << (int*)map.pkt << " ";
		std::cerr << std::endl;
	}
}
