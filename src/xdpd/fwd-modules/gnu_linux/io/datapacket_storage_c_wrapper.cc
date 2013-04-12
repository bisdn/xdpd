/*
 * datapacket_storage_c_wrapper.c
 *
 *  Created on: Feb 1, 2013
 *      Author: victor
 */

#include "datapacket_storage.h"
#include "datapacket_storage_c_wrapper.h"

datapacket_store_handle
create_datapacket_store(size_t max_size, uint16_t expiration)
{
	return new datapacket_storage(max_size, expiration);
}

void
destroy_datapacket_store(datapacket_store_handle handle)
{
	delete (datapacket_storage*)handle;
}

storeid
datapacket_storage_store_packet(datapacket_store_handle handle, datapacket_t *pkt)
{
	return ((datapacket_storage*)handle)->store_packet(pkt);
}

datapacket_t *
datapacket_storage_get_packet_wrapper(datapacket_store_handle handle, storeid id)
{
	return ((datapacket_storage*)handle)->get_packet(id);
}

int
datapacket_storage_oldest_packet_needs_expiration_wrapper(
		datapacket_store_handle handle, storeid *id)
{
	return ((datapacket_storage*)handle)->oldest_packet_needs_expiration(id);
}
