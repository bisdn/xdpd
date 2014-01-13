#include "ioport_netmap.h"
#include <iostream>
#include <rofl/common/utils/c_logger.h>
#include "../../bufferpool.h"
#include <fcntl.h>

#include <net/if.h>

using namespace xdpd::gnu_linux;

//Constructor and destructor
ioport_netmap::ioport_netmap(switch_port_t* of_ps, unsigned int num_queues):ioport(of_ps,num_queues){

	int fd;
	struct nmreq req;
	struct netmap_d *mem;
	if (!strcmp(of_port_state->name, "lo"))
		throw "Don't register for loopback";
	else if (!strcmp(of_port_state->name, "eth0"))
		throw "Don't register for eth0";

	ROFL_INFO("netmap: %s trying to open with netmap\n", of_port_state->name);
	fd = open("/dev/netmap", O_RDWR);
	if (fd == -1) {
		throw "Check kernel module";
	}

	bzero(&req,sizeof(nmreq));
	req.nr_version = NETMAP_API;

	strcpy(req.nr_name, of_port_state->name);
	req.nr_ringid = 0;

	if (ioctl(fd, NIOCREGIF, &req) == -1) {
		close(fd);
		throw "Unable to register";
	}

	ROFL_INFO("mapping %d Kbytes\n", req.nr_memsize>>10);
	mem = (struct netmap_d *) mmap(0, req.nr_memsize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	bzero(mem,sizeof(req.nr_memsize));
	if ( mem == MAP_FAILED ) {
		throw "MMAP Failed";
	}
	nifp = NETMAP_IF(mem, req.nr_offset);

	npipe[0].fd = fd;

	ROFL_INFO("netmap: %s opened with netmap\n", of_port_state->name);
}

ioport_netmap::~ioport_netmap(){
	close(npipe[READ].fd);
}

//Read and write methods over port
void ioport_netmap::enqueue_packet(datapacket_t* pkt, unsigned int q_id){
/*
	size_t ret;
	//Whatever
	const char c='a';
*/
	//Put in the queue
	output_queues[q_id].blocking_write(pkt);
/*
	//TODO: make it happen only if thread is really sleeping...
	ret = ::write(notify_pipe[WRITE],&c,sizeof(c));
	(void)ret; // todo use the value
*/
}

datapacket_t* ioport_netmap::read(){

	datapacket_t* pkt;
	datapacketx86* pkt_x86;
	unsigned int i=0;
	char *temp_buf;
	uint8_t *buf;

	struct netmap_ring *ring;

	struct pollfd x[1];
	x[0].fd = npipe[READ].fd;
	x[0].events = (POLLIN);

	//Allocate free buffer
	pkt = bufferpool::get_free_buffer();
	pkt_x86 = ((datapacketx86*)pkt->platform_state);

	poll(x, 1, 1000);
	ring = NETMAP_RXRING(nifp, i);
	if (ring->avail == 0)
		return NULL;
	ROFL_INFO("Ring Avail: %"PRIu32"\n", ring->avail);

	i = ring->cur;
	struct netmap_slot *slot = &ring->slot[i];

	temp_buf = NETMAP_BUF(ring, slot->buf_idx);
	memcpy(&buf,&temp_buf,sizeof(uint8_t *));
	ROFL_INFO("Got pkt %p of size %d\n", temp_buf, slot->len);

	pkt_x86->init(buf, slot->len, of_port_state->attached_sw, of_port_state->of_port_num,0,false,false);
	//pkt_x86->init(buf, slot->len, of_port_state->attached_sw, of_port_state->of_port_num);

	ring->cur = NETMAP_RING_NEXT(ring, i);
	ring->avail-- ;
	ROFL_INFO("We have %zu packets in our ring\n", ring->avail);

	ROFL_DEBUG_VERBOSE("Filled buffer with id:%d. Sending to process.\n", pkt_x86->buffer_id);

	return pkt;
}


unsigned int ioport_netmap::write(unsigned int q_id, unsigned int num_of_buckets){
	unsigned int i;
	datapacket_t* pkt;
	datapacketx86* pkt_x86;

	(void)pkt_x86;

/*
	//Free the pipe
	ret = ::read(notify_pipe[READ],&dummy,SIMULATED_PKT_SIZE);
	(void)ret; // todo use the value
	//Go and do stuff
	*/
	for(i=0;i<num_of_buckets;i++){	
		//Pick buffer	
		pkt = output_queues[q_id].non_blocking_read();
		
		if(!pkt)
			break;


		ROFL_DEBUG_VERBOSE("Getting buffer with id:%d. Putting it into the wire\n", pkt_x86->buffer_id);
		
		//Free buffer
		bufferpool::release_buffer(pkt);
	}
	i++;
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

	return ROFL_SUCCESS;
}
