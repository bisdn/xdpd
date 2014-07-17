/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DATAPACKETX86_H
#define DATAPACKETX86_H 

#include <bitset>
#include <iostream>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/platform/memory.h>

#include "packet_classifiers/pktclassifier.h"

//Profiling
#include "../util/time_measurements.h"

/**
* @file datapacketx86.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Data packet abstraction for an x86 (GNU/Linux)
*
*/

namespace xdpd {
namespace gnu_linux {


/* Auxiliary state for x86 datapacket*/
//buffering status
typedef enum{
	X86_DATAPACKET_BUFFER_IS_EMPTY,
	X86_DATAPACKET_BUFFERED_IN_NIC,
	X86_DATAPACKET_BUFFERED_IN_USER_SPACE
}x86buffering_status_t;

/**
* @brief Datapacket abstraction for an x86 (GNU/Linux)
*
* @ingroup driver_gnu_linux_io
*/
class datapacketx86{

public:
	
	//Constructor&destructor
	datapacketx86(datapacket_t*const pkt);
	~datapacketx86();

	//Incomming packet information
	of_switch_t* lsw;

	//Output queue
	uint32_t output_queue;

	//Temporary store for pkt_in information
	uint8_t pktin_table_id;
	of_packet_in_reason_t pktin_reason;	
	uint16_t pktin_send_len;

	//Time profiling
	TM_PKT_STATE;

public: // methods

	//Initialize the already constructed object
	inline rofl_result_t init(uint8_t* buf, size_t buflen, of_switch_t* sw, uint32_t in_port, uint32_t in_phy_port = 0, bool classify=true, bool copy_packet_to_internal_buffer = true);

	//Destroy object. This is NOT a destructor nor releases memory, but resets fields
	inline void destroy(void);

	/*
	* Return pointer to the buffer, regardless of where is right now (NIC or USER_SPACE). For memory on USER_SPACE returns pointer to the FIRST packet bytes.
	*/
	inline uint8_t* get_buffer(){ return (uint8_t*)clas_state.base; }
	inline size_t get_buffer_length(){ return clas_state.len; }
	inline x86buffering_status_t get_buffering_status(){ return buffering_status; }

	//Transfer buffer to user-space
	rofl_result_t transfer_to_user_space(void);

	//Header packet classification
	struct classifier_state clas_state;

	//Other	
	friend std::ostream& operator<<(std::ostream& os, datapacketx86& pack);
	inline void dump(void) {
		dump_pkt_classifier(&clas_state);
	}

	/*
	* Push&pop raw operations. To be used ONLY by classifiers
	*/
	rofl_result_t push(unsigned int offset, unsigned int num_of_bytes);
	rofl_result_t pop(unsigned int offset, unsigned int num_of_bytes);

	rofl_result_t push(uint8_t* push_point, unsigned int num_of_bytes);
	rofl_result_t pop(uint8_t* pop_point, unsigned int num_of_bytes);
	
private:
	//HOST buffer size
	static const unsigned int PRE_GUARD_BYTES  = 256;
	static const unsigned int FRAME_SIZE_BYTES = 9000;
	static const unsigned int POST_GUARD_BYTES = 64;

	/*
	 * real memory area
	 * => in user space, this contains the above buffer including head and tail space
	 * => in NIC space, it is set to 0
	 */
	struct iovec slot;

	//FIXME: NIC buffer info MISSING

	//User space buffer	
	uint8_t user_space_buffer[PRE_GUARD_BYTES+FRAME_SIZE_BYTES+POST_GUARD_BYTES];

	//Status of this buffer
	x86buffering_status_t buffering_status;

	/**
	 * utility function to set the correct buffer location
	 * @param location
	 */
	void init_internal_buffer_location_defaults(x86buffering_status_t location, uint8_t* buf, size_t buflen);
	//Add more stuff here...

public:

	friend std::ostream&
	operator<<(std::ostream& os, datapacketx86 const& pkt) {
		os << "<datapacketx86: ";
//			os << "buffer-id:" << (std::hex) << pkt.buffer_id << (std::dec) << " ";
//			os << "internal-buffer-id:" << (std::hex) << pkt.internal_buffer_id << (std::dec) << " ";
			os << "lsw:" << (int*)(pkt.lsw) << " ";
			os << "in-port:" << pkt.clas_state.port_in << " ";
			os << "in-phy-port:" << pkt.clas_state.phy_port_in << " ";
			os << "output-queue:" << pkt.output_queue << " ";
			os << "pktin-table-id:" << pkt.pktin_table_id << " ";
			os << "pktin-reason:" << pkt.pktin_reason << " ";
		os << ">";
		return os;
	};

};

/*
* Inline functions
*/ 

inline void datapacketx86::init_internal_buffer_location_defaults(x86buffering_status_t location, uint8_t* buf, size_t buflen){

	switch (location) {

		case X86_DATAPACKET_BUFFERED_IN_NIC:
			slot.iov_base = 0;
			slot.iov_len = 0;

			clas_state.base = buf;
			clas_state.len = buflen;
			buffering_status = X86_DATAPACKET_BUFFERED_IN_NIC;
			break;

		case X86_DATAPACKET_BUFFERED_IN_USER_SPACE:
			slot.iov_base = user_space_buffer;
			slot.iov_len = sizeof(user_space_buffer);
#ifndef NDEBUG
			// not really necessary, but makes debugging a little bit easier
			platform_memset(slot.iov_base, 0x00, slot.iov_len);
#endif
			clas_state.base = user_space_buffer + PRE_GUARD_BYTES;
			clas_state.len = buflen; // set to requested length
			buffering_status = X86_DATAPACKET_BUFFERED_IN_USER_SPACE;
			break;

		case X86_DATAPACKET_BUFFER_IS_EMPTY:
		default:
			// todo ?
			break;
	}
}

/*
 * Inline methods for acquiring/releasing a buffer from data packet storage
 */

//Init

rofl_result_t datapacketx86::init(
		uint8_t* buf, size_t buflen,
		of_switch_t* sw,
		uint32_t in_port,
		uint32_t in_phy_port,
		bool classify, 
		bool copy_packet_to_internal_buffer){

	// do this sanity check here, as someone may request later a transfer to user space,
	// so make sure we have enough space for doing this later
	if (buflen > FRAME_SIZE_BYTES){
		return ROFL_FAILURE;
	}

	if( copy_packet_to_internal_buffer ) {

		init_internal_buffer_location_defaults(X86_DATAPACKET_BUFFERED_IN_USER_SPACE, NULL, buflen);

		if(buf)
			platform_memcpy(clas_state.base, buf, buflen);
	}else{
		if(!buf)
			return ROFL_FAILURE;

		init_internal_buffer_location_defaults(X86_DATAPACKET_BUFFERED_IN_NIC, buf, buflen);
	}

	//Fill in
	this->lsw = sw;
	this->output_queue = 0;
	//Timestamp S1	
	TM_STAMP_STAGE_DPX86(this, TM_S1);
	
	//Classify the packet
	if(classify)
		classify_packet(&clas_state, get_buffer(), get_buffer_length(), in_port, 0);

	return ROFL_SUCCESS;
}



void datapacketx86::destroy(void){

	if (X86_DATAPACKET_BUFFERED_IN_USER_SPACE == get_buffering_status()){
#ifndef NDEBUG
		// not really necessary, but makes debugging a little bit easier
		platform_memset(slot.iov_base, 0x00, slot.iov_len);
#endif

		buffering_status = X86_DATAPACKET_BUFFER_IS_EMPTY;
	}
}
}// namespace xdpd::gnu_linux 
}// namespace xdpd



#endif /* DATAPACKETX86_H_ */
