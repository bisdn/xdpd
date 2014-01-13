/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DATAPACKETX86_H
#define DATAPACKETX86_H 

#include <bitset>
#include <inttypes.h>
#include <sys/types.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>

#include "packet_classifiers/packetclassifier.h"

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
* @ingroup fm_gnu_linux_io
*
* @ingroup fm_gnu_linux_io
*/
class datapacketx86{

public:
	
	//Constructor&destructor
	datapacketx86();
	~datapacketx86();

	//General data of the packet
	uint64_t buffer_id;		//Unique "non-reusable" buffer id
	uint64_t internal_buffer_id;	//IO subsystem buffer ID

	//Incomming packet information
	of_switch_t* lsw;
	uint32_t in_port;
	uint32_t in_phy_port;
	
	//Output queue
	uint32_t output_queue;

	//Checksum flags
	bool ipv4_recalc_checksum;
	bool tcp_recalc_checksum;
	bool udp_recalc_checksum;
	bool icmpv4_recalc_checksum;

	//Temporary store for pkt_in information
	uint8_t pktin_table_id;
	of_packet_in_reason_t pktin_reason;	

	//Opaque pointer
	void* extra;

	//Time profiling
	TM_PKT_STATE;

public: // methods

	//Initialize the already constructed object
	rofl_result_t init(uint8_t* buf, size_t buflen, of_switch_t* sw, uint32_t in_port, uint32_t in_phy_port = 0, bool classify=true, bool copy_packet_to_internal_buffer = true);

	//Destroy object. This is NOT a destructor nor releases memory, but resets fields
	void destroy(void);

	/*
	* Return pointer to the buffer, regardless of where is right now (NIC or USER_SPACE). For memory on USER_SPACE returns pointer to the FIRST packet bytes.
	*/
	inline uint8_t* get_buffer(){ return (uint8_t*)buffer.iov_base; }
	inline size_t get_buffer_length(){ return buffer.iov_len; }
	inline x86buffering_status_t get_buffering_status(){ return buffering_status; }

	//Transfer buffer to user-space
	rofl_result_t transfer_to_user_space(void);

	//Header packet classification	
	friend class packetclassifier;
	packetclassifier* headers;
	

	//Other	
	friend std::ostream& operator<<(std::ostream& os, datapacketx86& pack);
	inline void dump(void) {
		headers->dump();
	}

private:
	//HOST buffer size
	static const unsigned int PRE_GUARD_BYTES  = 256;
	static const unsigned int FRAME_SIZE_BYTES = 9000;
	static const unsigned int POST_GUARD_BYTES = 64;

	/*
	* Pointer to buffer, either on NIC or on USER_SPACE pointer. It ALWAYS points to the first byte of the packet.
	*/
	struct iovec buffer;

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
	void
	init_internal_buffer_location_defaults(x86buffering_status_t location, uint8_t* buf, size_t buflen);
	//Add more stuff here...

	
	/*
	* Push&pop raw operations. To be used ONLY by classifiers
	*/
	rofl_result_t push(unsigned int num_of_bytes, unsigned int offset = 0);
	rofl_result_t pop(unsigned int num_of_bytes, unsigned int offset = 0);

	rofl_result_t push(uint8_t* push_point, unsigned int num_of_bytes);
	rofl_result_t pop(uint8_t* pop_point, unsigned int num_of_bytes);

public:

	friend std::ostream&
	operator<<(std::ostream& os, datapacketx86 const& pkt) {
		os << "<datapacketx86: ";
			os << "buffer-id:" << (std::hex) << pkt.buffer_id << (std::dec) << " ";
			os << "internal-buffer-id:" << (std::hex) << pkt.internal_buffer_id << (std::dec) << " ";
			os << "lsw:" << (int*)(pkt.lsw) << " ";
			os << "in-port:" << pkt.in_port << " ";
			os << "in-phy-port:" << pkt.in_phy_port << " ";
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

inline void
datapacketx86::init_internal_buffer_location_defaults(
		x86buffering_status_t location, uint8_t* buf, size_t buflen)
{
	switch (location) {

		case X86_DATAPACKET_BUFFERED_IN_NIC:
			slot.iov_base = 0;
			slot.iov_len = 0;

			buffer.iov_base = buf;
			buffer.iov_len = buflen;
			buffering_status = X86_DATAPACKET_BUFFERED_IN_NIC;
			break;

		case X86_DATAPACKET_BUFFERED_IN_USER_SPACE:
			slot.iov_base = user_space_buffer;
			slot.iov_len = sizeof(user_space_buffer);
#ifndef NDEBUG
			// not really necessary, but makes debugging a little bit easier
			platform_memset(slot.iov_base, 0x00, slot.iov_len);
#endif
			buffer.iov_base = user_space_buffer + PRE_GUARD_BYTES;
			buffer.iov_len = buflen; // set to requested length
			buffering_status = X86_DATAPACKET_BUFFERED_IN_USER_SPACE;
			break;

		case X86_DATAPACKET_BUFFER_IS_EMPTY:
		default:
			// todo ?
			break;
	}
}

}// namespace xdpd::gnu_linux 
}// namespace xdpd



#endif /* DATAPACKETX86_H_ */
