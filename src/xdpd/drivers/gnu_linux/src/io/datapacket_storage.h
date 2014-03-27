/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DATAPACKET_STORAGE_H_
#define DATAPACKET_STORAGE_H_

#include <vector>
#include <list>
#include <queue>
#include <cstddef>
#include <ctime>
#include <iostream>

#include <stdint.h>
#include <pthread.h>

#include <rofl/datapath/pipeline/common/datapacket.h>

/**
* @file datapacket_storage.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Temporal storage for packets. 
*
*/

namespace xdpd {
namespace gnu_linux {

typedef uint32_t storeid;

/**
* @brief Temporal storage for datapackets (PKT_IN events). 
*
* @ingroup fm_gnu_linux_io
*/
class datapacket_storage
{
public:
	/**
	 * constructor
	 * @param size
	 */
	datapacket_storage(uint16_t size, uint16_t expiration_time_sec);

	/**
	 * destructor
	 */
	~datapacket_storage();
	
	/**
	 * store a datapacket
	 * @param pkt
	 * @return id of the stored packet, -1 if the packet could not be stored
	 */
	storeid
	store_packet(datapacket_t *pkt);

	/**
	 * get the stored packet by id
	 * @param id
	 * @return
	 */
	datapacket_t*
	get_packet(storeid id);

	/**
	 * get the size of the storage
	 * @return
	 */
	uint16_t
	get_storage_size() const;
	
	/**
	 * returns true if the first element needs to be
	 * expired, and sets it's ID in the parameter.
	 */
	bool
	oldest_packet_needs_expiration(storeid *id);
	
#ifdef DEBUG
	void change_expiration_time(uint16_t sec);
#endif

	//Used only for debugging purposes
	void dump_state();
	void dump_slots();

	friend std::ostream&
	operator<< (std::ostream& os, datapacket_storage const& ds) {
		os << "<datapacket_storage: ";
			os << "max-size:" << (int)ds.max_size << " ";
			os << "expiration-time-sec:" << (int)ds.expiration_time_sec << " ";
			os << "store.size():" << (int)ds.store.size() << " ";
			os << "now:" << time(NULL) << " ";
		os << ">";
		return os;
	};

	//Define error constant
	static const storeid ERROR = 0xFFFFFFFF;

private:
	
	storeid next_id;
	uint16_t max_size;
	typedef struct {
		storeid id;
		datapacket_t* pkt;
		time_t input_timestamp;
	} store_mapping;

	std::list<store_mapping> store;
	pthread_mutex_t lock;
	uint16_t expiration_time_sec;

	// this class is noncopyable
	datapacket_storage(const datapacket_storage&);
	datapacket_storage& operator=(const datapacket_storage&);
};

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* DATAPACKET_STORAGE_H_ */
