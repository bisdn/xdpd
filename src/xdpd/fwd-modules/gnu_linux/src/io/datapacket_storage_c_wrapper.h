/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DATAPACKET_STORAGE_C_WRAPPER_H_
#define DATAPACKET_STORAGE_C_WRAPPER_H_

#include <stdint.h>
#include <rofl.h>

/**
* @file datapacket_storage_c_wrapper.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
*
* @brief C wrapper for the datapacket_storage
*
*/
//C++ extern C

ROFL_BEGIN_DECLS

typedef uint32_t storeid;
typedef void* datapacket_store_handle;

datapacket_store_handle
create_datapacket_store(size_t max_size, uint16_t expiration);

void
destroy_datapacket_store(datapacket_store_handle handle);

storeid
datapacket_storage_store_packet(datapacket_store_handle handle, datapacket_t *pkt);

datapacket_t *
datapacket_storage_get_packet_wrapper(datapacket_store_handle handle,
		storeid id);

int
datapacket_storage_oldest_packet_needs_expiration_wrapper(
		datapacket_store_handle handle, storeid *id);

//C++ extern C
ROFL_END_DECLS

#endif // DATAPACKET_STORAGE_C_WRAPPER_H_
