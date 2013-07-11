/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MMAP_TX_H
#define MMAP_TX_H 

#include <string>

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
* @file mmap_tx.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief MMAP TX internals 
*
*/

using namespace rofl;

class eConstructorMmapTx : public rofl::cerror {};

class mmap_tx{

private:

	void* map;
	int block_size;
	int n_blocks;
	int frame_size;
	
	std::string devname; // device name e.g. "eth0"
	
	int sd; // socket descriptor
	caddress ll_addr; // link layer sockaddr
	struct tpacket_req req; // ring buffer

	//Circular buffer pointer
	unsigned int tpos; // current position within ring buffer

public:
	/**
	 *
	 */
	mmap_tx(std::string devname,
		int block_size,
		int n_blocks,
		int frame_size);

	~mmap_tx(void);

	/**
	 *
	 */
	inline struct tpacket2_hdr* get_free_slot(){
		struct tpacket2_hdr *hdr = (struct tpacket2_hdr*)((uint8_t*)map + tpos * req.tp_frame_size);
	
		if (hdr->tp_status == TP_STATUS_AVAILABLE) {
		
			//Increment and return
			tpos++;
			if (tpos == req.tp_frame_nr) {
				tpos = 0;
			}
			
			return hdr;
		}else
			return NULL;
	};

	inline rofl_result_t send(void){
		ROFL_DEBUG_VERBOSE("%s() on socket descriptor %d\n", __FUNCTION__, sd);

		if ((::sendto(sd, NULL, 0, MSG_DONTWAIT, NULL, 0)) < 0) {

			ROFL_ERR("[%s:mmap_tx]: Error in port's sendto(), errno:%d, %s\n", devname.c_str(), errno, strerror(errno));
				return ROFL_FAILURE;	
		}
		return ROFL_SUCCESS;	
	};
};

#endif /* MMAP_TX_H_ */
