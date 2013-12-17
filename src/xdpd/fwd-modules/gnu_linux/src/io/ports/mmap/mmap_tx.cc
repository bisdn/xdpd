#include "mmap_tx.h"
#include <assert.h> 

using namespace xdpd::gnu_linux;

mmap_tx::mmap_tx(
		std::string __devname,
		int __block_size,
		int __n_blocks,
		int __frame_size) :
		map(NULL),
		block_size(__block_size),
		n_blocks(__n_blocks),
		frame_size(__frame_size),
		devname(__devname),
		sd(-1),
		ll_addr(ETH_P_ALL, devname, 0, 0, NULL, 0),
		tpos(0)
{
	ROFL_DEBUG_VERBOSE( "mmap_tx(%p)::mmap_tx() %s\n",
			this, "RX-RING");

	memset(&req, 0, sizeof(req));

	int rc = 0;

	if (-1 != sd)
	{
		// CHECK: unmap memory area first?
		close(sd);
	}

	// open socket
	if ((sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		throw eConstructorMmapTx();
	}

	/* prepare interface request struct */
	struct ifreq ifr; // for ioctls on socket
	memset((void*)&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname.c_str(), sizeof(ifr.ifr_name));

	/* get device ifindex from kernel */
	if ((rc = ioctl(sd, SIOCGIFINDEX, &ifr)) < 0)
	{
		throw eConstructorMmapTx();
	}
	ll_addr.ca_sladdr->sll_ifindex = ifr.ifr_ifindex;

	/* prepare packet request struct */
	struct packet_mreq mr;
	memset(&mr, 0, sizeof(mr));
	mr.mr_ifindex = ifr.ifr_ifindex;

	int tmp = 0;
	/* set packet loss option */
	if (setsockopt(sd, SOL_PACKET, PACKET_LOSS, (char *) &tmp, sizeof(tmp))
			< 0) {
		throw eConstructorMmapTx();
	}

	// todo check this
	//	// make socket non-blocking
	//	long flags;
	//	if ((flags = fcntl(sd, F_GETFL)) < 0)
	//	{
	//		throw eConstructorMmapTx();
	//	}
	//	flags |= O_NONBLOCK;
	//	if ((rc = fcntl(sd, F_SETFL, flags)) < 0)
	//	{
	//		throw eConstructorMmapTx();
	//	}
	//


	// setup the tx/rx-ring request ...try a more conservative setting first
	req.tp_block_size 	= block_size /* 96 */ * getpagesize(); // fixme should be this: long sz = sysconf(_SC_PAGESIZE);
	req.tp_block_nr 	= n_blocks; // 2
	req.tp_frame_size 	= frame_size; // 2048
	req.tp_frame_nr 	= req.tp_block_size * req.tp_block_nr / req.tp_frame_size;


	ROFL_DEBUG_VERBOSE( "mmap_tx(%p)::initialize() block-size:%u block-nr:%u frame-size:%u frame-nr:%u\n",
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
		ROFL_ERR( "mmap_tx(%p)::initialize() setsockopt() sys-call failed for PACKET_VERSION "
				"rc: %d errno: %d (%s)\n", this, rc, errno, strerror(errno));
		throw eConstructorMmapTx();
	}

	/* request the rx/rx-ring */
	if ((rc = setsockopt(sd, SOL_PACKET, PACKET_TX_RING,
			(void *) &req, sizeof(req))) < 0)
	{
		// todo implement a retry if the request is not accepted

		ROFL_DEBUG_VERBOSE( "mmap_tx(%p)::initialize() setsockopt() sys-call failed "
				"rc: %d errno: %d (%s)\n", this, rc, errno, strerror(errno));
		throw eConstructorMmapTx();
	}

#if 0
	//Setting this screws everything.. DO NOT uncomment

        /* change the buffer size */
	if (setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &req.tp_frame_size, sizeof(req.tp_frame_size))< 0)
	{
		ROFL_ERR( "mmap_tx(%p)::initialize() Could not change buffer_size\n");
		throw eConstructorMmapTx();
	}
#endif

	/* change the buffer size */
//	int optval = 131072;
//	int option = (PACKET_TX_RING == ring_type) ? SO_SNDBUF : SO_RCVBUF;
//	if ((rc = setsockopt(sd, SOL_SOCKET, option, (int*) &optval,
//			sizeof(optval))) < 0) {
//		throw eConstructorMmapTx();
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
		ROFL_ERR( "mmap_tx(%p)::initialize() mmap() sys-call failed "
				"rc: %d errno: %d (%s)\n", this, rc, errno, strerror(errno));
		throw eConstructorMmapTx();
	}

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

}


mmap_tx::~mmap_tx()
{
	//ROFL_DEBUG_VERBOSE( "mmap_tx(%p)::~mmap_tx() %s\n",
	//		this, "TX-RING");

	if (-1 != sd)
	{
		if (map != MAP_FAILED)
		{
			int rc = 0;

			if ((rc = munmap(map, req.tp_block_size * req.tp_block_nr)) < 0)
			{
				ROFL_ERR("mmap_tx(%p)::~mmap_tx() %s => errno: %d (%s) \n",
						this, "TX-RING", errno, strerror(errno));

			}
		}

		close(sd);
	}

}

