/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mmap_int.h"

#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <errno.h>

#include <rofl/common/utils/c_logger.h>

using namespace xdpd::gnu_linux;

mmap_int::mmap_int(
		int __type,
		std::string __devname,
		int __block_size,
		int __n_blocks,
		int __frame_size) :
		map(NULL),
		block_size(__block_size),
		n_blocks(__n_blocks),
		frame_size(__frame_size),
		devname(__devname),
		ring_type(__type),
		sd(-1),
		ll_addr(ETH_P_ALL, devname, 0, 0, NULL, 0),
		ring(NULL),
		rpos(0)
{
	ROFL_DEBUG_VERBOSE( "cpktline(%p)::cpktline() %s\n",
			this, (ring_type == PACKET_TX_RING) ? "TX-RING" : "RX-RING");

	memset(&req, 0, sizeof(req));

	initialize();
}


mmap_int::~mmap_int()
{
	//ROFL_DEBUG_VERBOSE( "cpktline(%p)::~cpktline() %s\n",
	//		this, (ring_type == PACKET_TX_RING) ? "TX-RING" : "RX-RING");

	if (-1 != sd)
	{
		if (map != MAP_FAILED)
		{
			int rc = 0;

			if ((rc = munmap(map, req.tp_block_size * req.tp_block_nr)) < 0)
			{
				ROFL_ERR("cpktline(%p)::~cpktline() %s => errno: %d (%s) \n",
						this, (ring_type == PACKET_TX_RING) ? "TX-RING" : "RX-RING", errno, strerror(errno));

				throw eInternalError();
			}
		}

		close(sd);
	}
}

void
mmap_int::initialize() throw (ePktLineFailed)
{
	int rc = 0;

	if (-1 != sd)
	{
		// CHECK: unmap memory area first?
		close(sd);
	}

	// open socket
	if ((sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		throw ePktLineFailed();
	}

	/* prepare interface request struct */
	struct ifreq ifr; // for ioctls on socket
	memset((void*)&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname.c_str(), sizeof(ifr.ifr_name));

	/* get device ifindex from kernel */
	if ((rc = ioctl(sd, SIOCGIFINDEX, &ifr)) < 0)
	{
		throw ePktLineFailed();
	}
	ll_addr.ca_sladdr->sll_ifindex = ifr.ifr_ifindex;

	/* prepare packet request struct */
	struct packet_mreq mr;
	memset(&mr, 0, sizeof(mr));
	mr.mr_ifindex = ifr.ifr_ifindex;

	if (PACKET_TX_RING == ring_type) {

		int tmp = 0;
		/* set packet loss option */
		if (setsockopt(sd, SOL_PACKET, PACKET_LOSS, (char *) &tmp, sizeof(tmp))
				< 0) {
			throw ePktLineFailed();
		}

	} else {

		/* enable promiscuous mode */
		mr.mr_type = PACKET_MR_PROMISC;
		if (setsockopt(sd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr))
				== -1) {
			throw ePktLineFailed();
		}

		/* todo check this:
		 * Enable auxillary data if supported and reserve room for
		 * reconstructing VLAN headers. */
#ifdef HAVE_PACKET_AUXDATA
		val = 1;
		if (setsockopt(sock_fd, SOL_PACKET, PACKET_AUXDATA, &val,
				sizeof(val)) == -1 && errno != ENOPROTOOPT) {
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
					"setsockopt: %s", pcap_strerror(errno));
			close(sock_fd);
			return PCAP_ERROR;
		}
		handle->offset += VLAN_TAG_LEN;
#endif /* HAVE_PACKET_AUXDATA */

	}

	// todo check this
	//	// make socket non-blocking
	//	long flags;
	//	if ((flags = fcntl(sd, F_GETFL)) < 0)
	//	{
	//		throw ePktLineFailed();
	//	}
	//	flags |= O_NONBLOCK;
	//	if ((rc = fcntl(sd, F_SETFL, flags)) < 0)
	//	{
	//		throw ePktLineFailed();
	//	}
	//


	// setup the tx/rx-ring request ...try a more conservative setting first
	req.tp_block_size 	= block_size /* 96 */ * getpagesize(); // fixme should be this: long sz = sysconf(_SC_PAGESIZE);
	req.tp_block_nr 	= n_blocks; // 2
	req.tp_frame_size 	= frame_size; // 2048
	req.tp_frame_nr 	= req.tp_block_size * req.tp_block_nr / req.tp_frame_size;


	ROFL_DEBUG_VERBOSE( "cpktline(%p)::initialize() block-size:%u block-nr:%u frame-size:%u frame-nr:%u\n",
			this,
			req.tp_block_size,
			req.tp_block_nr,
			req.tp_frame_size,
			req.tp_frame_nr);

	// todo probe for tpacket-v2 in kernel and go back to tpacket-v1?

	/* setup for the tx/rx-ring tpacket v2 */
	int val = TPACKET_V2; // to recv. vlan
	if ((rc = setsockopt(sd, SOL_PACKET, PACKET_VERSION,
			(void *) &val, sizeof(val))) < 0)
	{
		ROFL_ERR( "cpktline(%p)::initialize() setsockopt() sys-call failed for PACKET_VERSION "
				"rc: %d errno: %d (%s)\n", this, rc, errno, strerror(errno));
		throw ePktLineFailed();
	}

	/* request the rx/rx-ring */
	if ((rc = setsockopt(sd, SOL_PACKET, ring_type,
			(void *) &req, sizeof(req))) < 0)
	{
		// todo implement a retry if the request is not accepted

		ROFL_DEBUG_VERBOSE( "cpktline(%p)::initialize() setsockopt() sys-call failed "
				"rc: %d errno: %d (%s)\n", this, rc, errno, strerror(errno));
		throw ePktLineFailed();
	}

	/* change the buffer size */
//	int optval = 131072;
//	int option = (PACKET_TX_RING == ring_type) ? SO_SNDBUF : SO_RCVBUF;
//	if ((rc = setsockopt(sd, SOL_SOCKET, option, (int*) &optval,
//			sizeof(optval))) < 0) {
//		throw ePktLineFailed();
//	}

	// todo for rx
//	/* Reserve space for VLAN tag reconstruction */
//	val = VLAN_TAG_LEN;
//	if (setsockopt(handle->fd, SOL_PACKET, PACKET_RESERVE, &val,
//		       sizeof(val)) < 0) {
//		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
//		    "can't set up reserve on packet socket: %s",
//		    pcap_strerror(errno));
//		return -1;
//	}

	// this is mapped as contiguous memory area by the kernel
	if ((map = (char*)mmap(0, req.tp_block_size * req.tp_block_nr,
			PROT_READ | PROT_WRITE /* | PROT_EXEC*/, MAP_SHARED,
			/*file descriptor*/sd, /*offset*/0)) == MAP_FAILED)
	{
		ROFL_ERR( "cpktline(%p)::initialize() mmap() sys-call failed "
				"rc: %d errno: %d (%s)\n", this, rc, errno, strerror(errno));
		throw ePktLineFailed();
	}


	ring = (struct iovec*)ringptrs.resize(req.tp_frame_nr * sizeof(struct iovec));

	for (unsigned int i = 0; i < req.tp_frame_nr; ++i)
	{
		ring[i].iov_base = (void*)((uint8_t*)map + i * req.tp_frame_size);
		ring[i].iov_len  = req.tp_frame_size;
		//ROFL_DEBUG_VERBOSE( "cpktline(%p)::initialize() ring[%d].iov_base: %p   \n",
		//		this, i, ring[i].iov_base);
		//ROFL_DEBUG_VERBOSE( "cpktline(%p)::initialize() ring[%d].iov_len:  %lu\n",
		//		this, i, ring[i].iov_len);
	}



	if (PACKET_TX_RING == ring_type) {

		struct sockaddr_ll s_ll;

		memset(&s_ll, 0, sizeof(s_ll));

		s_ll.sll_family = AF_PACKET;
		s_ll.sll_protocol = htons(ETH_P_ALL);
		s_ll.sll_ifindex = ifr.ifr_ifindex;
		s_ll.sll_hatype = 0;
		s_ll.sll_halen = 0;
		s_ll.sll_pkttype = 0;

		if (-1 ==  bind(sd, (struct sockaddr *) &s_ll, sizeof(s_ll))) {
			assert(0);
		}
	} else {
		/* bind socket to device */
		if (-1 == bind(sd, ll_addr.ca_saddr, ll_addr.salen))
		{
			switch (errno) {
			case EADDRINUSE:
				throw ePktLineFailed();
			default:
				throw ePktLineFailed();
			}
		}
	}
}

struct tpacket2_hdr*
mmap_int::get_free_slot()
{
	struct tpacket2_hdr *hdr = NULL;

	if (((struct tpacket2_hdr*)ring[rpos].iov_base)->tp_status == TP_STATUS_AVAILABLE) {
		hdr = (struct tpacket2_hdr*)ring[rpos].iov_base;
		rpos++;
		if (rpos == req.tp_frame_nr) {
			rpos = 0;
		}
	}
	return hdr;
}

void
mmap_int::copy_packet(struct tpacket2_hdr *hdr, datapacketx86* packet)
{
	uint8_t *data = ((uint8_t *) hdr) + TPACKET2_HDRLEN - sizeof(struct sockaddr_ll);
	memcpy(data, packet->get_buffer(), packet->get_buffer_length());

#if 0
	ROFL_DEBUG_VERBOSE("%s(): datapacketx86 %p to tpacket_hdr %p\n"
			"	data = %p\n,"
			"	with content:\n", __FUNCTION__, packet, hdr, data);
	packet->dump();
#endif
	hdr->tp_len = packet->get_buffer_length();
	hdr->tp_snaplen = packet->get_buffer_length();
	hdr->tp_status = TP_STATUS_SEND_REQUEST;
}

int
mmap_int::send()
{
	int rc = 0;

	ROFL_DEBUG_VERBOSE("%s() on socket descriptor %d\n", __FUNCTION__, sd);

	if ((rc = ::sendto(sd, NULL, 0, MSG_DONTWAIT, NULL, 0)) < 0) {

		ROFL_ERR("Error in port %s: %d:%s\n", devname.c_str(), errno, strerror(errno));
	
		switch (errno) {
		case EMSGSIZE:

			break;
		}
	}

	return rc;
}
