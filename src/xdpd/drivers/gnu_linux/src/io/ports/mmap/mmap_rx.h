/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MMAP_RX_H
#define MMAP_RX_H 

#include <string>
#include <assert.h>

#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include <rofl/common/croflexception.h>
#include <rofl/common/utils/c_logger.h>
#include "../../../util/likely.h"
#include "../../../config.h"

/**
* @file mmap_rx.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief MMAP RX internals 
*
*/

namespace xdpd {
namespace gnu_linux {

class eConstructorMmapRx : public rofl::RoflException {};

/**
* @brief MMAP RX internals (v2)
*
* @ingroup driver_gnu_linux_io_ports
*/
class mmap_rx{

private:

	
	void* map;
	int block_size;
	int n_blocks;
	int frame_size;

	std::string devname; // device name e.g. "eth0"
	
	int sd; // socket descriptor
	struct sockaddr_ll ll_addr;
	struct tpacket_req req; // ring buffer
	//struct iovec *ring; // auxiliary pointers into the mmap'ed area

	//Circular buffer pointer
	unsigned int rpos; // current position within ring buffer


public:
	/**
	 *
	 */
	mmap_rx(std::string devname,
		int block_size,
		int n_blocks,
		int frame_size);

	~mmap_rx(void);


	/**
	 *
	 */
	inline struct tpacket2_hdr* read_packet(){

		struct tpacket2_hdr *hdr;
next:  
		hdr = (struct tpacket2_hdr*)((uint8_t*)map + rpos * req.tp_frame_size);

		/* treat any status besides kernel as readable */
		if (TP_STATUS_KERNEL == hdr->tp_status) {
			return NULL;
		}

		//Increment position
		rpos++;
		if (rpos == req.tp_frame_nr) {
			rpos = 0;
		}

		//Check if is valid 
		if( likely( ( hdr->tp_status&(TP_STATUS_COPY|TP_STATUS_CSUMNOTREADY) ) == 0 ) ){
#ifdef DEBUG
			//if( ( hdr->tp_status&(TP_STATUS_LOSING) ) > 0){
			//	ROFL_DEBUG_VERBOSE(DRIVER_NAME"[mmap_rx:%s] Congestion in RX of the port\n", devname.c_str());
			//}
		
#endif

			return hdr;
		}else{
			//TP_STATUS_COPY or TP_STATUS_CSUMNOTREADY (outgoing) => ignore
			static unsigned int dropped = 0;

			if( unlikely((dropped++%1000) == 0) ){
				ROFL_ERR(DRIVER_NAME"[mmap_rx:%s] ERROR: discarded %u frames. Reason(s): %s %s (%u), length: %u. If TP_STATUS_CSUMNOTREADY is the cause, consider disabling RX/TX checksum offloading via ethtool.\n", devname.c_str(), dropped, ((hdr->tp_status&TP_STATUS_COPY) == 0)? "":"TP_STATUS_COPY", ((hdr->tp_status&TP_STATUS_CSUMNOTREADY) == 0)? "":"TP_STATUS_CSUMNOTREADY", hdr->tp_status, hdr->tp_len );
			}

			//Skip
			hdr->tp_status = TP_STATUS_KERNEL;
			goto next;
		}
	}
	
	//Return buffer
	inline void return_packet(struct tpacket2_hdr* hdr){
		hdr->tp_status = TP_STATUS_KERNEL;
	}

	// Get read fds.
	inline int get_fd(void){
		return sd;
	};

	// Get 
	inline struct tpacket_req* get_tpacket_req(void){
		return &req;
	};
};

}// namespace xdpd::gnu_linux 
}// namespace xdpd


#endif /* MMAP_RX_H_ */
