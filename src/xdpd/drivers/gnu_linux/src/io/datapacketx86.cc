/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "datapacketx86.h"

//Include here the classifier you want to use

using namespace xdpd::gnu_linux;

/*
 * x86 datapacket related methods
 */

//Change this if you want to use another classifier
typedef struct classify_state pktclassifier;

//Constructor
datapacketx86::datapacketx86(datapacket_t*const pkt) :
	lsw(0),
	pktin_table_id(0),
	pktin_reason(0),
	buffering_status(X86_DATAPACKET_BUFFER_IS_EMPTY){

}



datapacketx86::~datapacketx86(){

}



//Transfer copy to user-space
rofl_result_t datapacketx86::transfer_to_user_space(){

	switch (get_buffering_status()){

		case X86_DATAPACKET_BUFFERED_IN_NIC: {
			slot.iov_base 	= user_space_buffer;
			slot.iov_len	= sizeof(user_space_buffer);
#ifndef NDEBUG
			// not really necessary, but makes debugging a little bit easier
			platform_memset(slot.iov_base, 0x00, slot.iov_len);
#endif
			// safety check for clas_state.len <= FRAME_SIZE_BYTES was done in datapacketx86::init() already
			platform_memcpy((uint8_t*)slot.iov_base + PRE_GUARD_BYTES, clas_state.base, clas_state.len);

			clas_state.base = (uint8_t*)slot.iov_base + PRE_GUARD_BYTES;
			// clas_state.len stays as it is
			
			// set buffering flag
			buffering_status = X86_DATAPACKET_BUFFERED_IN_USER_SPACE;
			
			//Re-classify 
			//TODO: use offsets instead of fixed pointers for frames to avoid re-classification here
			clas_state.base = get_buffer();
			
			//Copy done
		} return ROFL_SUCCESS;

		case X86_DATAPACKET_BUFFERED_IN_USER_SPACE: {
		}return ROFL_SUCCESS;
		
		case X86_DATAPACKET_BUFFER_IS_EMPTY: // packet is un-initialized
		default: {
		} return ROFL_FAILURE; // do nothing
	}
}



/*
 * Push&pop operations
 */
rofl_result_t datapacketx86::push(unsigned int offset, unsigned int num_of_bytes){

	//If not already transfer to user space
	if(X86_DATAPACKET_BUFFERED_IN_NIC == buffering_status){
		transfer_to_user_space();
	}
	
	if (offset > clas_state.len){
		return ROFL_FAILURE;
	}

	size_t free_space_head = (uint8_t*)clas_state.base - (uint8_t*)slot.iov_base;
	size_t free_space_tail = slot.iov_len - (free_space_head + clas_state.len);

	/*
	 * this is the safe sanity check for both head and tail space
	 */
	if (num_of_bytes > (free_space_head + free_space_tail)){
		return ROFL_FAILURE;
	}

	/*
	 * implicit assumption: we only have pre-guard-bytes and move forward and backward at the head of the packet
	 */
	if (num_of_bytes > free_space_head){
		return ROFL_FAILURE;
	}

	// move header num_of_bytes backward
	platform_memmove((uint8_t*)clas_state.base - num_of_bytes, clas_state.base, offset);
#ifndef NDEBUG
	// initialize new pushed memory area with 0x00
	platform_memset((uint8_t*)clas_state.base - num_of_bytes + offset, 0x00, num_of_bytes);
#endif

	clas_state.base = (uint8_t*)clas_state.base - num_of_bytes;
	clas_state.len += num_of_bytes;


	return ROFL_SUCCESS;
}



rofl_result_t datapacketx86::pop(unsigned int offset, unsigned int num_of_bytes){

	//Check boundaries
	//FIXME

	//If not already transfer to user space
	if(X86_DATAPACKET_BUFFERED_IN_NIC == buffering_status){
		transfer_to_user_space();
	}

	// sanity check: start of area to be deleted must not be before start of buffer
	if (offset > clas_state.len){
		return ROFL_FAILURE;
	}

	// sanity check: end of area to be deleted must not be behind end of buffer
	if ((offset + num_of_bytes) > clas_state.len){
		return ROFL_FAILURE;
	}

	// move first bytes backward
	platform_memmove((uint8_t*)clas_state.base + num_of_bytes, clas_state.base, offset);

#ifndef NDEBUG
	// set now unused bytes to 0x00 for easier debugging
	platform_memset(clas_state.base, 0x00, num_of_bytes);
#endif

	clas_state.base = (uint8_t*)clas_state.base + num_of_bytes;
	clas_state.len -= num_of_bytes;

	// re-parse_ether() here? yes, we have to, but what about the costs?

	return ROFL_SUCCESS;
}



//Push&pop operations
rofl_result_t datapacketx86::push(uint8_t* push_point, unsigned int num_of_bytes){

	//If not already transfer to user space
	if(X86_DATAPACKET_BUFFERED_IN_NIC == buffering_status){
		transfer_to_user_space();
	}
	
	if (push_point < clas_state.base){
		return ROFL_FAILURE;
	}

	if (((uint8_t*)push_point + num_of_bytes) > ((uint8_t*)clas_state.base + clas_state.len)){
		return ROFL_FAILURE;
	}

	//size_t offset = ((uint8_t*)clas_state.base - push_point);
	size_t offset = (push_point - (uint8_t*)clas_state.base);

	return push(offset, num_of_bytes);
}



rofl_result_t datapacketx86::pop(uint8_t* pop_point, unsigned int num_of_bytes){

	if (pop_point < clas_state.base){
		return ROFL_FAILURE;
	}

	if (((uint8_t*)pop_point + num_of_bytes) > ((uint8_t*)clas_state.base + clas_state.len)){
		return ROFL_FAILURE;
	}

	//size_t offset = ((uint8_t*)clas_state.base - pop_point);
	size_t offset = ((uint8_t*)clas_state.base - pop_point);

	return pop(offset, num_of_bytes);
}


