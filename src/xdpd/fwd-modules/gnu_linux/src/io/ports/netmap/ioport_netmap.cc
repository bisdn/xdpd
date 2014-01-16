#include "ioport_netmap.h"
#include <iostream>
#include <rofl/common/utils/c_logger.h>
#include "../../bufferpool.h"
#include <fcntl.h>

#include <net/if.h>

using namespace xdpd::gnu_linux;

//Constructor and destructor
ioport_netmap::ioport_netmap(switch_port_t* of_ps, unsigned int num_queues):ioport(of_ps,num_queues){

	if (!strcmp(of_port_state->name, "lo"))
		throw "Don't register for loopback";
	else if (!strcmp(of_port_state->name, "eth0"))
		throw "Don't register for eth0";

	//ROFL_INFO("netmap: %s trying to open with netmap\n", of_port_state->name);
	fd = open("/dev/netmap", O_RDWR);
	if (fd == -1) {
		throw "Check kernel module";
	}

	struct nmreq req;
	bzero(&req,sizeof(nmreq));
	req.nr_version = NETMAP_API;

	strcpy(req.nr_name, of_port_state->name);

	if (ioctl(fd, NIOCREGIF, &req) == -1) {
		close(fd);
		throw "Unable to register";
	}
	//TODO fix
	num_of_queues=1;

	//ROFL_INFO("mapping %d Kbytes\n", req.nr_memsize>>10);
	mem = (struct netmap_d *) mmap(0, req.nr_memsize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	bzero(mem,sizeof(req.nr_memsize));
	if ( mem == MAP_FAILED ) {
		throw "MMAP Failed";
	}
	nifp = NETMAP_IF(mem, req.nr_offset);

	if(pipe(notify_pipe) == -1)
		ROFL_INFO("Pipe problem\n");

	for(unsigned int i=0;i<2;i++) {
		int flags = fcntl(notify_pipe[i], F_GETFL, 0);
		flags |= O_NONBLOCK;
		fcntl(notify_pipe[i], F_SETFL, flags);
	}
	deferred_drain = 0;

	ROFL_INFO("netmap: %s opened with netmap\n", of_port_state->name);
}

ioport_netmap::~ioport_netmap(){
	munmap(mem,memsize);
	close(notify_pipe[READ]);
	close(notify_pipe[WRITE]);
	close(fd);
}

//Read and write methods over port
void ioport_netmap::enqueue_packet(datapacket_t* pkt, unsigned int q_id){
	size_t ret;
	//Whatever
	const char c='a';

	//Put in the queue
	if(output_queues[q_id].blocking_write(pkt) != ROFL_SUCCESS ) {
		ROFL_INFO("Queue problem\n");
		bufferpool::release_buffer(pkt);
	}
	
	ROFL_DEBUG_VERBOSE("[mmap:%s] Packet(%p) enqueued, buffer size: %d\n",  of_port_state->name, pkt, output_queues[q_id].size());
	//TODO: make it happen only if thread is really sleeping...
	ret = ::write(notify_pipe[WRITE],&c,sizeof(c));
	(void)ret;
	return;
}

void ioport_netmap::flush_ring() {
	struct pollfd x[1];
	x[0].fd = fd;
	x[0].events = (POLLIN);

	poll(x,1,1000);

	struct netmap_ring *ring;
	ring = NETMAP_RXRING(nifp, 0);
	while(ring->avail > 0) {
		ring->cur = NETMAP_RING_NEXT(ring, ring->cur);
		ring->avail--;
	}
	if(ioctl(fd,NIOCTXSYNC|NIOCRXSYNC, NULL) == -1)
		ROFL_DEBUG_VERBOSE("Netmap sync problem\n");
	return;
}

void ioport_netmap::empty_pipe() {
	int ret;
	if(deferred_drain == 0)
		return;
	
	ret = ::read(notify_pipe[READ], draining_buffer, deferred_drain);

	if(ret == -1) {
		ROFL_INFO("pipe is empty");
	} else if((deferred_drain -ret) < 0)
		deferred_drain = 0;
	else
		deferred_drain -= ret;
}

datapacket_t* ioport_netmap::read(){

	datapacket_t* pkt;
	datapacketx86* pkt_x86;
	unsigned int cur;
	char *buf;

	//epoll ??
	//struct pollfd x[1];
	//x[0].fd = fd;
	//x[0].events = (POLLIN);

	struct netmap_ring *ring;
	//poll(x, 1, 1000);

	ring = NETMAP_RXRING(nifp, 0);
	//ROFL_INFO("Ring Avail: %"PRIu32"\n", ring->avail);
	
	if (ring->avail == 0) {
		return NULL;
	}

	//Allocate free buffer
	pkt = bufferpool::get_free_buffer(false);
	if(!pkt) {
		ROFL_INFO("Buffer Problem\n");
		return NULL;
	}
	pkt_x86 = ((datapacketx86*)pkt->platform_state);

	cur = ring->cur;
	struct netmap_slot *slot = &ring->slot[cur];

	buf = NETMAP_BUF(ring, slot->buf_idx);
	ROFL_INFO("Got pkt %p of size %d\n", buf, slot->len);

	pkt_x86->init((uint8_t*)buf, slot->len, of_port_state->attached_sw, of_port_state->of_port_num,0,true,true);
	//pkt_x86->init(buf, slot->len, of_port_state->attached_sw, of_port_state->of_port_num);

	ring->cur = NETMAP_RING_NEXT(ring, cur);
	ring->avail-- ;
	//ROFL_INFO("We have %zu packets in our ring\n", ring->avail);

	ROFL_DEBUG_VERBOSE("Filled buffer with id:%d. Sending to process.\n", pkt_x86->buffer_id);

	if(ioctl(fd,NIOCRXSYNC, NULL) == -1)
		ROFL_DEBUG_VERBOSE("Netmap sync problem\n");
	return pkt;
}

unsigned int ioport_netmap::write(unsigned int q_id, unsigned int num_of_buckets){
	unsigned int cur;
	datapacket_t* pkt;
	datapacketx86* pkt_x86;
	unsigned int i,count=num_of_buckets;

	// Do we have enough ring space?
	struct netmap_ring *ring;
	ring = NETMAP_TXRING(nifp, q_id);
	if ( ring->avail == 0 )
		return num_of_buckets;

	// Decrease the packet count
	if ( ring->avail < num_of_buckets)
		count = ring->avail;

	//ROFL_INFO("ID:%d Queue size %d\n",q_id,output_queues[q_id].size());
	
	//ROFL_INFO("Ring Avail: %"PRIu32" %zu\n", ring->avail, count);
	for(i=0;i<count;i++){
		
		// It shouldn't happen but let's make sure.
		if(output_queues[q_id].is_empty()) {
			if(deferred_drain > 0)
				empty_pipe();
			return num_of_buckets;
		}
		//Pick buffer
		pkt = output_queues[q_id].non_blocking_read();

		pkt_x86 = (datapacketx86*)pkt->platform_state;
		(void)pkt_x86;

		cur = ring->cur;
		struct netmap_slot *slot = &ring->slot[cur];
		char *buf;
		buf = NETMAP_BUF(ring, slot->buf_idx);
		pkt_copy(pkt_x86->get_buffer(),buf,pkt_x86->get_buffer_length());
		//slot->flags=0;
		//slot->flags |= NS_INDIRECT;
		//slot->ptr = (uint64_t)pkt_x86->get_buffer();

		//ROFL_INFO("Sending %p\n", pkt_x86->get_buffer());

		slot->len = pkt_x86->get_buffer_length();

		//ROFL_INFO("Sent pkt id:%p of size %d\n", pkt_x86, slot->len);
		ring->cur = NETMAP_RING_NEXT(ring, cur);

		//ROFL_DEBUG_VERBOSE("Getting buffer with id:%d. Putting it into the wire\n", pkt_x86->buffer_id);

		//Free buffer
		bufferpool::release_buffer(pkt);
	}
	ring->avail-=i;
	//ROFL_INFO("Sent %d out of %d\n", i,num_of_buckets);
	if(ioctl(fd,NIOCTXSYNC, NULL) == -1)
		ROFL_DEBUG_VERBOSE("Netmap sync problem\n");
	return num_of_buckets-i;
}

rofl_result_t ioport_netmap::disable(){
	struct ifreq ifr;
	int sd;
	if (( sd = socket(AF_PACKET, SOCK_RAW, 0)) <0 ) {
		return ROFL_FAILURE;
	}

	bzero(&ifr,sizeof(struct ifreq));
	strcpy(ifr.ifr_name, of_port_state->name);

	if (ioctl(sd, SIOCGIFFLAGS, &ifr) < 0) {
		close(sd);
		return ROFL_FAILURE;
	}

	if(IFF_UP & ifr.ifr_flags) {
		ifr.ifr_flags &= ~IFF_UP;
	}
	if(IFF_PROMISC & ifr.ifr_flags) {
		ifr.ifr_flags &= ~IFF_PROMISC;
	}

	if (ioctl(sd, SIOCSIFFLAGS, &ifr) <0) {
		close(sd);
		return ROFL_FAILURE;
	}

	of_port_state->up = true;
	close(sd);

	return ROFL_SUCCESS;
}

rofl_result_t ioport_netmap::enable(){
	struct ifreq ifr;
	int sd;
	if (( sd = socket(AF_PACKET, SOCK_RAW, 0)) <0 ) {
		return ROFL_FAILURE;
	}

	bzero(&ifr,sizeof(struct ifreq));
	strcpy(ifr.ifr_name, of_port_state->name);

	if (ioctl(sd, SIOCGIFFLAGS, &ifr) < 0) {
		close(sd);
		return ROFL_FAILURE;
	}
	if(IFF_UP | ifr.ifr_flags) {
		ifr.ifr_flags |= IFF_UP;
	}
	if(IFF_PROMISC | ifr.ifr_flags) {
		ifr.ifr_flags |= IFF_PROMISC;
	}

	if (ioctl(sd, SIOCSIFFLAGS, &ifr) <0) {
		close(sd);
		return ROFL_FAILURE;
	}

	of_port_state->up = true;
	close(sd);

	flush_ring();
	return ROFL_SUCCESS;
}
