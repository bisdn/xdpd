#include "ioport_mmap.h"
#include "../../bufferpool.h"
#include "../../datapacketx86.h"

#include <rofl/common/utils/c_logger.h>
#include <rofl/common/protocols/fetherframe.h>
#include <rofl/common/protocols/fvlanframe.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

//Constructor and destructor
ioport_mmap::ioport_mmap(
		/*int port_no,*/
		switch_port_t* of_ps,
		int block_size,
		int n_blocks,
		int frame_size,
		unsigned int num_queues) :
			ioport(of_ps, num_queues),
			rx(NULL),
			tx(NULL),
			block_size(block_size),
			n_blocks(n_blocks),
			frame_size(frame_size),	
			hwaddr(of_ps->hwaddr, OFP_ETH_ALEN)
{
	int ret;

	//Open pipe for output signaling on enqueue	
	ret = pipe(notify_pipe);
	(void)ret; // todo use the value

	//Set non-blocking read/write in the pipe
	for(unsigned int i=0;i<2;i++){
		int flags = fcntl(notify_pipe[i], F_GETFL, 0);	///get current file status flags
		flags |= O_NONBLOCK;				//turn off blocking flag
		fcntl(notify_pipe[i], F_SETFL, flags);		//set up non-blocking read
	}
}


ioport_mmap::~ioport_mmap()
{
	if(rx)
		delete rx;
	if(tx)
		delete tx;
	
	close(notify_pipe[READ]);
	close(notify_pipe[WRITE]);
}

//Read and write methods over port
void
ioport_mmap::enqueue_packet(datapacket_t* pkt, unsigned int q_id)
{
	//Whatever
	const char c='a';
	int ret;
	

	if (of_port_state->up && of_port_state->forward_packets ) {

		//Safe check for q_id
		if(q_id >= get_num_of_queues()){
			ROFL_DEBUG("[mmap:%s] Packet(%p) trying to be enqueued in an invalid q_id: %u\n",  of_port_state->name, pkt, q_id);
			q_id = 0;
			bufferpool::release_buffer(pkt);
			assert(0);
		}
	
		//Store on queue and exit. This is NOT copying it to the mmap buffer
		if(output_queues[q_id].non_blocking_write(pkt) != ROFL_SUCCESS){
			ROFL_DEBUG("[mmap:%s] Packet(%p) dropped. Congestion in output queue: %d\n",  of_port_state->name, pkt, q_id);
			//Drop packet
			bufferpool::release_buffer(pkt);
			return;
		}

		ROFL_DEBUG_VERBOSE("[mmap:%s] Packet(%p) enqueued, buffer size: %d\n",  of_port_state->name, pkt, output_queues[q_id].size());
	
		//TODO: make it happen only if thread is really sleeping...
		ret = ::write(notify_pipe[WRITE],&c,sizeof(c));
		(void)ret; // todo use the value
	} else {
		ROFL_DEBUG_VERBOSE("[mmap:%s] dropped packet(%p) scheduled for queue %u\n", of_port_state->name, pkt, q_id);
		// port down -> drop packet
		bufferpool::release_buffer(pkt);
	}

}

// handle read
datapacket_t*
ioport_mmap::read()
{
	//Whatever
	char c;
	int ret;

	if(!of_port_state->up || of_port_state->drop_received || !rx)
		return NULL;

	//Just take the byte from the pipe	
	ret = ::read(notify_pipe[READ],&c,sizeof(c));
	(void)ret; // todo use the value
	
	if (input_queue.is_empty() && rx ) {
		read_loop(rx->sd, 1);
	}

	return input_queue.non_blocking_read();
}

int
ioport_mmap::read_loop(int fd /* todo do we really need the fd? */,
		int read_max)
{
	if (!rx || rx->sd != fd)
	{
		return 0;
	}

#if 0
	ROFL_DEBUG_VERBOSE("ioport_mmap(%s)::read_loop() total #slots:%d\n",
			of_port_state->name, rx->req.tp_frame_nr);
#endif
	int cnt = 0;
	int rx_bytes_local = 0;

	while (cnt < read_max) {

		// i = (i == rxline.req.tp_frame_nr - 1) ? 0 : (i+1);

		/*
		   Frame structure:

		   - Start. Frame must be aligned to TPACKET_ALIGNMENT=16
		   - struct tpacket_hdr
		   - pad to TPACKET_ALIGNMENT=16
		   - struct sockaddr_ll
		   - Gap, chosen so that packet data (Start+tp_net) aligns to
		     TPACKET_ALIGNMENT=16
		   - Start+tp_mac: [ Optional MAC header ]
		   - Start+tp_net: Packet data, aligned to TPACKET_ALIGNMENT=16.
		   - Pad to align to TPACKET_ALIGNMENT=16
		 */

		struct tpacket2_hdr *hdr = rx->read_packet();
		if (NULL == hdr) {
			break;
		}

		/* sanity check */
		if (hdr->tp_mac + hdr->tp_snaplen > rx->req.tp_frame_size) {
			ROFL_DEBUG_VERBOSE("[mmap:%s] sanity check during read mmap failed\n",of_port_state->name);
			
			//Increment error statistics
			switch_port_stats_inc(of_port_state,0,0,0,0,1,0);
			hdr->tp_status = TP_STATUS_KERNEL; // return packet to kernel
			return cnt;
		}

		// todo check if this is necessary
//		fetherframe ether(((uint8_t*)hdr + hdr->tp_mac), hdr->tp_len);
//		if (ether.get_dl_src() == hwaddr)
//		{
//			ROFL_DEBUG_VERBOSE("cioport(%s)::handle_revent() self-originating "
//				"frame rcvd in slot i:%d, ignoring", devname.c_str(), i);
//			continue; // ignore self-originating frames
//		}

		struct sockaddr_ll *sll = (struct sockaddr_ll*)((uint8_t*)hdr + TPACKET_ALIGN(sizeof(struct tpacket_hdr)));
		if (PACKET_OUTGOING == sll->sll_pkttype) {
			/*ROFL_DEBUG_VERBOSE("cioport(%s)::handle_revent() outgoing "
					"frame rcvd in slot i:%d, ignoring\n", of_port_state->name, rx->rpos);*/
			goto next; // ignore outgoing frames
		}

		{
			//Retrieve buffer from pool: this is a blocking call
			datapacket_t *pkt = bufferpool::get_free_buffer(false);

			// handle no free buffer
			if (NULL == pkt) {
				//Increment error statistics and drop
				switch_port_stats_inc(of_port_state,0,0,0,0,1,0);
				hdr->tp_status = TP_STATUS_KERNEL; // return packet to kernel
				return cnt;
			}

			datapacketx86 *pkt_x86 = (datapacketx86*) pkt->platform_state;

			cmacaddr eth_src = cmacaddr(((struct fetherframe::eth_hdr_t*)((uint8_t*)hdr + hdr->tp_mac))->dl_src, OFP_ETH_ALEN);

			if (hwaddr == eth_src) {
				/*ROFL_DEBUG_VERBOSE("cioport(%s)::handle_revent() outgoing "
						"frame rcvd in slot i:%d, src-mac == own-mac, ignoring\n", of_port_state->name, rx->rpos);*/
				//pkt_x86->destroy(); //This is not anymore necessary
				bufferpool::release_buffer(pkt);
				goto next; // ignore outgoing frames
			}


			if (0 != hdr->tp_vlan_tci) {
				// packet has vlan tag
				pkt_x86->init(NULL, hdr->tp_len + sizeof(struct fvlanframe::vlan_hdr_t), of_port_state->attached_sw, get_port_no(),0,false);

				// write ethernet header
				memcpy(pkt_x86->get_buffer(), (uint8_t*)hdr + hdr->tp_mac, sizeof(struct fetherframe::eth_hdr_t));

				// set dl_type to vlan
				if ( htobe16(ETH_P_8021Q) == ((struct fetherframe::eth_hdr_t*)((uint8_t*)hdr + hdr->tp_mac))->dl_type ) {
					((struct fetherframe::eth_hdr_t*)pkt_x86->get_buffer())->dl_type = htobe16(ETH_P_8021Q); // tdoo maybe this should be ETH_P_8021AD
				} else {
					((struct fetherframe::eth_hdr_t*)pkt_x86->get_buffer())->dl_type = htobe16(ETH_P_8021Q);
				}

				// write vlan
				struct fvlanframe::vlan_hdr_t* vlanptr =
						(struct fvlanframe::vlan_hdr_t*) (pkt_x86->get_buffer()
								+ sizeof(struct fetherframe::eth_hdr_t));
				vlanptr->byte0 =  (hdr->tp_vlan_tci >> 8);
				vlanptr->byte1 = hdr->tp_vlan_tci & 0x00ff;
				vlanptr->dl_type = ((struct fetherframe::eth_hdr_t*)((uint8_t*)hdr + hdr->tp_mac))->dl_type;

				// write payload
				memcpy(pkt_x86->get_buffer() + sizeof(struct fetherframe::eth_hdr_t) + sizeof(struct fvlanframe::vlan_hdr_t),
						(uint8_t*)hdr + hdr->tp_mac + sizeof(struct fetherframe::eth_hdr_t),
						hdr->tp_len - sizeof(struct fetherframe::eth_hdr_t));
                

		                pkt_x86->headers->classify();

			} else {
				// no vlan tag present
				pkt_x86->init((uint8_t*)hdr + hdr->tp_mac, hdr->tp_len, of_port_state->attached_sw, get_port_no());
			}

			ROFL_DEBUG("[mmap:%s] packet(%p) recieved\n", of_port_state->name ,pkt);

			// fill input_queue
			if(input_queue.non_blocking_write(pkt) != ROFL_SUCCESS){
				ROFL_DEBUG("[mmap:%s] Congestion in input intermediate buffer, dropping packet(%p)\n", of_port_state->name ,pkt);
				bufferpool::release_buffer(pkt);
				goto next;
			}
		}

		rx_bytes_local += hdr->tp_len;
		cnt++;

next:
		hdr->tp_status = TP_STATUS_KERNEL; // return packet to kernel
		rx->rpos++; // select next packet. todo: should be moved to pktline
		if (rx->rpos >=rx->req.tp_frame_nr) {
			rx->rpos = 0;
		}
	}

	// ROFL_DEBUG_VERBOSE("cnt=%d\n", cnt);

	//Increment statistics
	if(cnt)
		switch_port_stats_inc(of_port_state, cnt, 0, rx_bytes_local, 0, 0, 0);	
	
	return cnt;

}

unsigned int
ioport_mmap::write(unsigned int q_id, unsigned int num_of_buckets)
{
	datapacket_t* pkt;
	unsigned int cnt = 0;
	int tx_bytes_local = 0;

	if (0 == output_queues[q_id].size() || !tx) {
		return num_of_buckets;
	}

	ROFL_DEBUG_VERBOSE("[mmap:%s] (q_id=%d, num_of_buckets=%u): on %s with queue.size()=%u\n",
			of_port_state->name,
			q_id,
			num_of_buckets,
			output_queues[q_id].size());

	// read available packets from incoming buffer
	for ( ; 0 < num_of_buckets; --num_of_buckets ) {

		pkt = output_queues[q_id].non_blocking_read();

		if (NULL == pkt) {
			ROFL_DEBUG_VERBOSE("[mmap:%s] no packet in output_queue %u left, %u buckets left\n",
					of_port_state->name,
					q_id,
					num_of_buckets);
			break;
		}

		/*
		 Frame structure:

		 - Start. Frame must be aligned to TPACKET_ALIGNMENT=16
		 - struct tpacket_hdr
		 - pad to TPACKET_ALIGNMENT=16
		 - struct sockaddr_ll
		 - Gap, chosen so that packet data (Start+tp_net) aligns to
		 TPACKET_ALIGNMENT=16
		 - Start+tp_mac: [ Optional MAC header ]
		 - Start+tp_net: Packet data, aligned to TPACKET_ALIGNMENT=16.
		 - Pad to align to TPACKET_ALIGNMENT=16
		 */
		struct tpacket2_hdr *hdr = tx->get_free_slot();

		if (NULL != hdr) {
			datapacketx86* pkt_x86;
			// Recover x86 specific state
			pkt_x86 = (datapacketx86*) pkt->platform_state;

			// todo check the right size

			tx->copy_packet(hdr, pkt_x86);

			// todo statistics
			tx_bytes_local += hdr->tp_len;

		} else {
			ROFL_DEBUG_VERBOSE("no free slot in circular_queue\n");

			//Increment error statistics
			switch_port_stats_inc(of_port_state,0,0,0,0,0,1);
			port_queue_stats_inc(&of_port_state->queues[q_id], 0, 0, cnt);


			// Release and exit
			bufferpool::release_buffer(pkt);
			break;
		}

		ROFL_DEBUG("[mmap:%s] packet(%p) put in the MMAP region\n", of_port_state->name ,pkt);
		// pkt is processed
		bufferpool::release_buffer(pkt);
		cnt++;
	} // for

#if 0
	ROFL_DEBUG_VERBOSE("ioport_mmap::write(q_id=%d, num_of_buckets=%u) on %s all packets scheduled\n",
			q_id,
			num_of_buckets,
			of_port_state->name);
#endif

	if (cnt) {
		ROFL_DEBUG_VERBOSE("[mmap:%s] schedule %u packet(s) to be send\n", __FUNCTION__, cnt);
		// send packet-ins tx queue (this call is currently a blocking call!)
		if(tx->send()<0){
			ROFL_DEBUG("[mmap:%s] packet(%p) put in the MMAP region\n", of_port_state->name ,pkt);
			assert(0);
			switch_port_stats_inc(of_port_state, 0, 0, 0, 0, 0, cnt);	
			port_queue_stats_inc(&of_port_state->queues[q_id], 0, 0, cnt);	
		}

		//Increment statistics
		switch_port_stats_inc(of_port_state, 0, cnt, 0, tx_bytes_local, 0, 0);	
		port_queue_stats_inc(&of_port_state->queues[q_id], cnt, tx_bytes_local, 0);	
	}

	// return not used buckets
	return num_of_buckets;
}

/*
*
* Enable and disable port routines
*
*/
rofl_result_t
ioport_mmap::enable() {
	
	struct ifreq ifr;
	int sd, rc;

	ROFL_DEBUG_VERBOSE("[mmap:%s] Trying to enable port\n",of_port_state->name);
	
	if ((sd = socket(AF_PACKET, SOCK_RAW, 0)) < 0){
		return ROFL_FAILURE;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, of_port_state->name);

	if ((rc = ioctl(sd, SIOCGIFINDEX, &ifr)) < 0){
		return ROFL_FAILURE;
	}

	if ((rc = ioctl(sd, SIOCGIFFLAGS, &ifr)) < 0){ 
		close(sd);
		return ROFL_FAILURE;
	}

	if (IFF_UP & ifr.ifr_flags){
		
		//Already up.. Silently skip
		close(sd);

		//If tx/rx lines are not created create them
		if(!rx){	
			ROFL_DEBUG_VERBOSE("[mmap:%s] generating a new mmap_int for RX\n",of_port_state->name);
			rx = new mmap_int(PACKET_RX_RING, std::string(of_port_state->name), 2 * block_size, n_blocks, frame_size);
		}
		if(!tx){
			ROFL_DEBUG_VERBOSE("[mmap:%s] generating a new mmap_int for TX\n",of_port_state->name);
			tx = new mmap_int(PACKET_TX_RING, std::string(of_port_state->name), block_size, n_blocks, frame_size);
		}

		of_port_state->up = true;
		return ROFL_SUCCESS;
	}

	ifr.ifr_flags |= IFF_UP;
	if ((rc = ioctl(sd, SIOCSIFFLAGS, &ifr)) < 0){
		close(sd);
		return ROFL_FAILURE;
	}

	//If tx/rx lines are not created create them
	if(!rx){	
		ROFL_DEBUG_VERBOSE("[mmap:%s] generating a new mmap_int for RX\n",of_port_state->name);
		rx = new mmap_int(PACKET_RX_RING, std::string(of_port_state->name), 2 * block_size, n_blocks, frame_size);
	}
	if(!tx){
		ROFL_DEBUG_VERBOSE("[mmap:%s] generating a new mmap_int for TX\n",of_port_state->name);
		tx = new mmap_int(PACKET_TX_RING, std::string(of_port_state->name), block_size, n_blocks, frame_size);
	}

	

	// enable promiscous mode
	memset((void*)&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, of_port_state->name, sizeof(ifr.ifr_name));
	
	if ((rc = ioctl(sd, SIOCGIFFLAGS, &ifr)) < 0){
		close(sd);
		return ROFL_FAILURE;
	}

	ifr.ifr_flags |= IFF_PROMISC;
	if ((rc = ioctl(sd, SIOCSIFFLAGS, &ifr)) < 0){
		close(sd);
		return ROFL_FAILURE;
	}

	// todo recheck?
	// todo check link state IFF_RUNNING
	of_port_state->up = true;

	close(sd);
	return ROFL_SUCCESS;
}

rofl_result_t
ioport_mmap::disable() {
	struct ifreq ifr;
	int sd, rc;

	ROFL_DEBUG_VERBOSE("[mmap:%s] Trying to disable port\n",of_port_state->name);

	if ((sd = socket(AF_PACKET, SOCK_RAW, 0)) < 0) {
		return ROFL_FAILURE;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, of_port_state->name);

	if ((rc = ioctl(sd, SIOCGIFINDEX, &ifr)) < 0) {
		return ROFL_FAILURE;
	}

	if ((rc = ioctl(sd, SIOCGIFFLAGS, &ifr)) < 0) {
		close(sd);
		return ROFL_FAILURE;
	}

	//If rx/tx exist, delete them
	if(rx){
		ROFL_DEBUG_VERBOSE("[mmap:%s] destroying mmap_int for RX\n",of_port_state->name);
		delete rx;
		rx = NULL;
	}
	if(tx){
		ROFL_DEBUG_VERBOSE("[mmap:%s] destroying mmap_int for TX\n",of_port_state->name);
		delete tx;
		tx = NULL;
	}

	if ( !(IFF_UP & ifr.ifr_flags) ) {
		close(sd);
		//Already down.. Silently skip
		return ROFL_SUCCESS;
	}

	ifr.ifr_flags &= ~IFF_UP;

	if ((rc = ioctl(sd, SIOCSIFFLAGS, &ifr)) < 0) {
		close(sd);
		return ROFL_FAILURE;
	}

	// todo recheck?
	// todo check link state IFF_RUNNING
	of_port_state->up = false;

	close(sd);

	return ROFL_SUCCESS;
}
