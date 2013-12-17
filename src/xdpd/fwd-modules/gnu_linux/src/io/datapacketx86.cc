#include "datapacketx86.h"

//Include here the classifier you want to use
#include "packet_classifiers/rofl_pktclassifier.h"
#include "packet_classifiers/static_pktclassifier.h"

using namespace xdpd::gnu_linux;

/*
 * x86 datapacket related methods
 */

//Change this if you want to use another classifier
//typedef rofl_pktclassifier pktclassifier;
typedef static_pktclassifier pktclassifier;

//Constructor
datapacketx86::datapacketx86() :
	buffer_id(0),
	internal_buffer_id(0),
	lsw(0),
	in_port(0),
	in_phy_port(0),
	output_queue(0),
	ipv4_recalc_checksum(false),
	tcp_recalc_checksum(false),
	udp_recalc_checksum(false),
	icmpv4_recalc_checksum(false),
	pktin_table_id(0),
	pktin_reason(0),
	extra(NULL),
	headers(new pktclassifier(this)),
	buffering_status(X86_DATAPACKET_BUFFER_IS_EMPTY)
{

}



datapacketx86::~datapacketx86()
{
	delete headers;
}



/*
 * Inline methods for acquiring/releasing a buffer from data packet storage
 */

//Init

rofl_result_t
datapacketx86::init(
		uint8_t* buf, size_t buflen,
		of_switch_t* sw,
		uint32_t in_port,
		uint32_t in_phy_port,
		bool classify, 
		bool copy_packet_to_internal_buffer)
{
	// do this sanity check here, as someone may request later a transfer to user space,
	// so make sure we have enough space for doing this later
	if (buflen > FRAME_SIZE_BYTES){
		return ROFL_FAILURE;
	}
#if 0
	// if buffer is NULL we initialize to local buffer and do not copy anything
	if (NULL == buf) {
		init_internal_buffer_location_defaults(X86_DATAPACKET_BUFFERED_IN_USER_SPACE, NULL, buflen);
		return ROFL_SUCCESS;
	}
#endif

	if( copy_packet_to_internal_buffer) {

		init_internal_buffer_location_defaults(X86_DATAPACKET_BUFFERED_IN_USER_SPACE, NULL, buflen);

		if(buf)
			platform_memcpy(buffer.iov_base, buf, buflen);
	}else{
		if(!buf)
			return ROFL_FAILURE;

		init_internal_buffer_location_defaults(X86_DATAPACKET_BUFFERED_IN_NIC, buf, buflen);
	}

	//Fill in
	this->lsw = sw;
	this->in_port = in_port;
	this->in_phy_port = in_phy_port;
	//this->eth_type 		= 0;

	this->output_queue = 0;

	//Timestamp S1	
	TM_STAMP_STAGE_DPX86(this, TM_S1);
	
	//Classify the packet
	if(classify)
		headers->classify();

	return ROFL_SUCCESS;
}



void
datapacketx86::destroy(void)
{
	headers->classify_reset();

	if (X86_DATAPACKET_BUFFERED_IN_USER_SPACE == get_buffering_status()){
#ifndef NDEBUG
		// not really necessary, but makes debugging a little bit easier
		platform_memset(slot.iov_base, 0x00, slot.iov_len);
#endif

		slot.iov_base 	= 0;
		slot.iov_len 	= 0;
		buffer.iov_base = 0;
		buffer.iov_len 	= 0;

		buffering_status = X86_DATAPACKET_BUFFER_IS_EMPTY;
	}
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
			// safety check for buffer.iov_len <= FRAME_SIZE_BYTES was done in datapacketx86::init() already
			platform_memcpy((uint8_t*)slot.iov_base + PRE_GUARD_BYTES, buffer.iov_base, buffer.iov_len);

			buffer.iov_base = (uint8_t*)slot.iov_base + PRE_GUARD_BYTES;
			// buffer.iov_len stays as it is
			
			// set buffering flag
			buffering_status = X86_DATAPACKET_BUFFERED_IN_USER_SPACE;
			
			//Re-classify 
			//TODO: use offsets instead of fixed pointers for frames to avoid re-classification here
			headers->classify();
			
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
rofl_result_t
datapacketx86::push(
		unsigned int offset,
		unsigned int num_of_bytes)
{
	//If not already transfer to user space
	if(X86_DATAPACKET_BUFFERED_IN_NIC == buffering_status){
		transfer_to_user_space();
	}
	
	if (offset > buffer.iov_len){
		return ROFL_FAILURE;
	}

	size_t free_space_head = (uint8_t*)buffer.iov_base - (uint8_t*)slot.iov_base;
	size_t free_space_tail = slot.iov_len - (free_space_head + buffer.iov_len);

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
	platform_memmove((uint8_t*)buffer.iov_base - num_of_bytes, buffer.iov_base, offset);
#ifndef NDEBUG
	// initialize new pushed memory area with 0x00
	platform_memset((uint8_t*)buffer.iov_base - num_of_bytes + offset, 0x00, num_of_bytes);
#endif

	buffer.iov_base = (uint8_t*)buffer.iov_base - num_of_bytes;
	buffer.iov_len += num_of_bytes;


	return ROFL_SUCCESS;
}



rofl_result_t
datapacketx86::pop(
		unsigned int offset,
		unsigned int num_of_bytes)
{
	//Check boundaries
	//FIXME

	//If not already transfer to user space
	if(X86_DATAPACKET_BUFFERED_IN_NIC == buffering_status){
		transfer_to_user_space();
	}

	// sanity check: start of area to be deleted must not be before start of buffer
	if (offset > buffer.iov_len){
		return ROFL_FAILURE;
	}

	// sanity check: end of area to be deleted must not be behind end of buffer
	if ((offset + num_of_bytes) > buffer.iov_len){
		return ROFL_FAILURE;
	}

	// move first bytes backward
	platform_memmove((uint8_t*)buffer.iov_base + num_of_bytes, buffer.iov_base, offset);

#ifndef NDEBUG
	// set now unused bytes to 0x00 for easier debugging
	platform_memset(buffer.iov_base, 0x00, num_of_bytes);
#endif

	buffer.iov_base = (uint8_t*)buffer.iov_base + num_of_bytes;
	buffer.iov_len -= num_of_bytes;

	// re-parse_ether() here? yes, we have to, but what about the costs?

	return ROFL_SUCCESS;
}



//Push&pop operations
rofl_result_t
datapacketx86::push(
		uint8_t* push_point,
		unsigned int num_of_bytes)
{
	//If not already transfer to user space
	if(X86_DATAPACKET_BUFFERED_IN_NIC == buffering_status){
		transfer_to_user_space();
	}
	
	if (push_point < buffer.iov_base){
		return ROFL_FAILURE;
	}

	if (((uint8_t*)push_point + num_of_bytes) > ((uint8_t*)buffer.iov_base + buffer.iov_len)){
		return ROFL_FAILURE;
	}

	//size_t offset = ((uint8_t*)buffer.iov_base - push_point);
	size_t offset = (push_point - (uint8_t*)buffer.iov_base);

	return push(offset, num_of_bytes);
}



rofl_result_t
datapacketx86::pop(
		uint8_t* pop_point,
		unsigned int num_of_bytes)
{
	if (pop_point < buffer.iov_base){
		return ROFL_FAILURE;
	}

	if (((uint8_t*)pop_point + num_of_bytes) > ((uint8_t*)buffer.iov_base + buffer.iov_len)){
		return ROFL_FAILURE;
	}

	//size_t offset = ((uint8_t*)buffer.iov_base - pop_point);
	size_t offset = ((uint8_t*)buffer.iov_base - pop_point);

	return pop(offset, num_of_bytes);
}


