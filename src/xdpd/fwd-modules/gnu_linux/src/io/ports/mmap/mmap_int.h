/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CPKTLINE_H
#define CPKTLINE_H 

#include <string>
#include <unistd.h>
#include <assert.h>
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

#include <rofl/platform/unix/csyslog.h>
#include <rofl/common/cmemory.h>
#include <rofl/common/cerror.h>
#include <rofl/common/caddress.h>

#include "../../datapacketx86.h"

/**
* @file mmap_int.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief MMAP internals 
*
*/

namespace xdpd {
namespace gnu_linux {

using namespace rofl;

class ePktLineBase 		: public cerror {};
class ePktLineFailed 	: public ePktLineBase {};

/**
* @brief MMAP internals DEPRECATED
*
* @ingroup fm_gnu_linux_io_ports
*/
class mmap_int :
		public csyslog
{
public:
	/**
	 *
	 */
	mmap_int(
			int ring_type,
			std::string devname,
			int block_size = 96,
			int n_blocks = 2,
			int frame_size = 2048);

	/**
	 *
	 */
	virtual
	~mmap_int();

	struct tpacket2_hdr*
	get_free_slot();

	void
	copy_packet(struct tpacket2_hdr *hdr, datapacketx86* packet);

	int
	send();

	void* map;
	int block_size;
	int n_blocks;
	int frame_size;
	std::string devname; // device name e.g. "eth0"
	int ring_type;
	int sd; // socket descriptor
	caddress ll_addr; // link layer sockaddr
	struct tpacket_req req; // ring buffer
	struct iovec *ring; // auxiliary pointers into the mmap'ed area
	unsigned int rpos; // current position within ring buffer



	inline struct tpacket2_hdr*
	read_packet()
	{
		struct tpacket2_hdr *hdr = (struct tpacket2_hdr*)ring[rpos].iov_base;

		/* treat any status besides kernel as readable */
		if (TP_STATUS_KERNEL == hdr->tp_status) {
			return NULL;
		}

		return hdr;
	}

private:
	void
	initialize() throw (ePktLineFailed);

private:
	cmemory ringptrs; // memory for storing the (struct iovec*) pointers
};

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* CPKTLINE_H_ */
