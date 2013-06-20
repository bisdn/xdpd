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

#include <rofl/common/cerror.h>
#include <rofl/common/caddress.h>
#include <rofl/common/utils/c_logger.h>

/**
* @file mmap_rx.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief MMAP RX internals 
*
*/

using namespace rofl;

class eConstructorMmapRx : public rofl::cerror {};

class mmap_rx{

private:

	
	void* map;
	int block_size;
	int n_blocks;
	int frame_size;

	std::string devname; // device name e.g. "eth0"
	
	int sd; // socket descriptor
	caddress ll_addr; // link layer sockaddr
	struct tpacket_req req; // ring buffer
	//struct iovec *ring; // auxiliary pointers into the mmap'ed area

	//Circular buffer pointer
	unsigned int rpos; // current position within ring buffer


public:
	/**
	 *
	 */
	mmap_rx(std::string devname,
		int block_size = 96,
		int n_blocks = 2,
		int frame_size = 2048);

	~mmap_rx(void);


	/**
	 *
	 */
	inline struct tpacket2_hdr* read_packet(){
		
		struct tpacket2_hdr *hdr = (struct tpacket2_hdr*)((uint8_t*)map + rpos * req.tp_frame_size);

		/* treat any status besides kernel as readable */
		if (TP_STATUS_KERNEL == hdr->tp_status) {
			
			assert(TP_STATUS_KERNEL == hdr->tp_status);
			
			unsigned int rpos2=rpos+1;
			if(rpos2 == req.tp_frame_nr)
				rpos2 = 0;
			hdr = (struct tpacket2_hdr*)((uint8_t*)map + ((rpos2) * req.tp_frame_size));
			assert(TP_STATUS_KERNEL == hdr->tp_status);
			return NULL;
		}

		assert(TP_STATUS_USER == hdr->tp_status);

		//Increment and return
		rpos++;
		if (rpos == req.tp_frame_nr) {
			rpos = 0;
		}
		return hdr;
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

#endif /* MMAP_RX_H_ */
