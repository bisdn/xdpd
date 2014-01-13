#include "ioport_mmapv2.h"
#include "../../bufferpool.h"
#include "../../datapacketx86.h"
#include "../../../util/likely.h"
#include "../../iomanager.h"

#include <linux/ethtool.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/protocols/fetherframe.h>
#include <rofl/common/protocols/fvlanframe.h>

//Profiling
#include "../../../util/time_measurements.h"

using namespace rofl;
using namespace xdpd::gnu_linux;

//Constructor and destructor
ioport_mmapv2::ioport_mmapv2(
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
			deferred_drain(0)
{
	int rc;

	//Open pipe for output signaling on enqueue	
	rc = pipe(notify_pipe);
	(void)rc; // todo use the value

	//Set non-blocking read/write in the pipe
	for(unsigned int i=0;i<2;i++){
		int flags = fcntl(notify_pipe[i], F_GETFL, 0);	///get current file status flags
		flags |= O_NONBLOCK;				//turn off blocking flag
		fcntl(notify_pipe[i], F_SETFL, flags);		//set up non-blocking read
	}

}


ioport_mmapv2::~ioport_mmapv2()
{
	if(rx)
		delete rx;
	if(tx)
		delete tx;
	
	close(notify_pipe[READ]);
	close(notify_pipe[WRITE]);
}

//Read and write methods over port
void ioport_mmapv2::enqueue_packet(datapacket_t* pkt, unsigned int q_id){

	//Whatever
	const char c='a';
	int ret;
	unsigned int len;
	
	datapacketx86* pkt_x86 = (datapacketx86*) pkt->platform_state;
	len = pkt_x86->get_buffer_length();

	if ( likely(of_port_state->up) && 
		likely(of_port_state->forward_packets) &&
		likely(len >= MIN_PKT_LEN) ) {

		//Safe check for q_id
		if( unlikely(q_id >= get_num_of_queues()) ){
			ROFL_DEBUG("[mmap:%s] Packet(%p) trying to be enqueued in an invalid q_id: %u\n",  of_port_state->name, pkt, q_id);
			q_id = 0;
			bufferpool::release_buffer(pkt);
			assert(0);
		}
	
		//Store on queue and exit. This is NOT copying it to the mmap buffer
		if(output_queues[q_id].non_blocking_write(pkt) != ROFL_SUCCESS){
			TM_STAMP_STAGE(pkt, TM_SA6_FAILURE);
			
			ROFL_DEBUG("[mmap:%s] Packet(%p) dropped. Congestion in output queue: %d\n",  of_port_state->name, pkt, q_id);
			//Drop packet
			bufferpool::release_buffer(pkt);


			return;
		}
		TM_STAMP_STAGE(pkt, TM_SA6_SUCCESS);

		ROFL_DEBUG_VERBOSE("[mmap:%s] Packet(%p) enqueued, buffer size: %d\n",  of_port_state->name, pkt, output_queues[q_id].size());
	
		//TODO: make it happen only if thread is really sleeping...
		ret = ::write(notify_pipe[WRITE],&c,sizeof(c));
		(void)ret; // todo use the value
			
	} else {
		if(len < MIN_PKT_LEN){
			ROFL_ERR("[mmap:%s] ERROR: attempt to send invalid packet size for packet(%p) scheduled for queue %u. Packet size: %u\n", of_port_state->name, pkt, q_id, len);
			assert(0);
		}else{
			ROFL_DEBUG_VERBOSE("[mmap:%s] dropped packet(%p) scheduled for queue %u\n", of_port_state->name, pkt, q_id);
		}

		//Drop packet
		bufferpool::release_buffer(pkt);
	}

}

inline void ioport_mmapv2::empty_pipe(){
	int ret;

	if(unlikely(deferred_drain == 0))
		return;

	//Just take deferred_drain from the pipe 
	ret = ::read(notify_pipe[READ], draining_buffer, deferred_drain);

	if(unlikely(ret == -1)){
		//EAGAIN
	}else{
		if(unlikely( (deferred_drain - ret) < 0 ) ){
			deferred_drain = 0;
		}else
			deferred_drain -= ret;
	}
}

inline void ioport_mmapv2::fill_vlan_pkt(struct tpacket2_hdr *hdr, datapacketx86 *pkt_x86){

	//Initialize pktx86
	pkt_x86->init(NULL, hdr->tp_len + sizeof(struct fvlanframe::vlan_hdr_t), of_port_state->attached_sw, get_port_no(), 0, false); //Init but don't classify

	// write ethernet header
	memcpy(pkt_x86->get_buffer(), (uint8_t*)hdr + hdr->tp_mac, sizeof(struct fetherframe::eth_hdr_t));

	// set dl_type to vlan
	if( htobe16(ETH_P_8021Q) == ((struct fetherframe::eth_hdr_t*)((uint8_t*)hdr + hdr->tp_mac))->dl_type ) {
		((struct fetherframe::eth_hdr_t*)pkt_x86->get_buffer())->dl_type = htobe16(ETH_P_8021Q); // tdoo maybe this should be ETH_P_8021AD
	}else{
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

	//And classify
	pkt_x86->headers->classify();
}
	
// handle read
datapacket_t* ioport_mmapv2::read(){

	struct tpacket2_hdr *hdr;
	struct sockaddr_ll *sll;
	datapacket_t *pkt;
	datapacketx86 *pkt_x86;
	uint8_t* pkt_mac;

	//Check if we really have to read
	if(!of_port_state->up || of_port_state->drop_received || !rx)
		return NULL;

next:
	//Retrieve a packet	
 	hdr = rx->read_packet();

	//No packets available
	if (!hdr)
		return NULL;

	//Sanity check 
	if ( unlikely(hdr->tp_mac + hdr->tp_snaplen > rx->get_tpacket_req()->tp_frame_size) ) {
		ROFL_DEBUG_VERBOSE("[mmap:%s] sanity check during read mmap failed\n",of_port_state->name);
		//Increment error statistics
		switch_port_stats_inc_lockless(of_port_state,0,0,0,0,1,0);

		//Return packet to kernel in the RX ring		
		rx->return_packet(hdr);
		return NULL;
	}

	//Check if it is an ongoing frame from TX
	sll = (struct sockaddr_ll*)((uint8_t*)hdr + TPACKET_ALIGN(sizeof(struct tpacket_hdr)));
	if (PACKET_OUTGOING == sll->sll_pkttype) {
		/*ROFL_DEBUG_VERBOSE("cioport(%s)::handle_revent() outgoing "
					"frame rcvd in slot i:%d, ignoring\n", of_port_state->name, rx->rpos);*/

		//Return packet to kernel in the RX ring		
		rx->return_packet(hdr);
		goto next;
	}
	
	//Discard frames generated by the switch or the OS (feedback)
	pkt_mac = ((struct fetherframe::eth_hdr_t*)((uint8_t*)hdr + hdr->tp_mac))->dl_src;
	if (memcmp(pkt_mac, mac, ETHER_MAC_LEN) == 0 ){
		/*ROFL_DEBUG_VERBOSE("cioport(%s)::handle_revent() outgoing "
		"frame rcvd in slot i:%d, src-mac == own-mac, ignoring\n", of_port_state->name, rx->rpos);*/

		//Return packet to kernel in the RX ring		
		rx->return_packet(hdr);
		goto next;
	}

	//Retrieve buffer from pool: this is a non-blocking call
	pkt = bufferpool::get_free_buffer(false);

	//Handle no free buffer
	if(!pkt) {
		//Increment error statistics and drop
		switch_port_stats_inc_lockless(of_port_state,0,0,0,0,1,0);
		rx->return_packet(hdr);
		return NULL;
	}
			
	pkt_x86 = (datapacketx86*) pkt->platform_state;

	//Fill packet
	if(hdr->tp_vlan_tci != 0){
		//There is a VLAN
		fill_vlan_pkt(hdr, pkt_x86);	
	}else{
		// no vlan tag present
		pkt_x86->init((uint8_t*)hdr + hdr->tp_mac, hdr->tp_len, of_port_state->attached_sw, get_port_no());
	}

	//Timestamp S2	
	TM_STAMP_STAGE(pkt, TM_S2);

	//Return packet to kernel in the RX ring		
	rx->return_packet(hdr);

	//Increment statistics&return
	switch_port_stats_inc_lockless(of_port_state, 1, 0, hdr->tp_len, 0, 0, 0);	
	
	return pkt;


}

inline void ioport_mmapv2::fill_tx_slot(struct tpacket2_hdr *hdr, datapacketx86 *packet){

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

unsigned int ioport_mmapv2::write(unsigned int q_id, unsigned int num_of_buckets){

	struct tpacket2_hdr *hdr;
	datapacket_t* pkt;
	datapacketx86* pkt_x86;
	unsigned int cnt = 0;
	int tx_bytes_local = 0;

	circular_queue<datapacket_t, IO_IFACE_RING_SLOTS>* queue = &output_queues[q_id];

	if ( unlikely(tx == NULL) ) {
		return num_of_buckets;
	}

	// read available packets from incoming buffer
	for ( ; 0 < num_of_buckets; --num_of_buckets ) {

		
		//Check
		if(queue->size() == 0){
			ROFL_DEBUG_VERBOSE("[mmap:%s] no packet left in output_queue %u left, %u buckets left\n",
					of_port_state->name,
					q_id,
					num_of_buckets);

			//Drain notification pipe (pending tokens)
			if(unlikely(deferred_drain > 0)){
				empty_pipe();
			}
			break;
		}

		//Retrieve an empty slot in the TX ring
		hdr = tx->get_free_slot();

		//Skip, TX is full
		if(!hdr)
			break;
		
		//Retrieve the buffer
		pkt = queue->non_blocking_read();
		
		if(!pkt){
			ROFL_ERR("[mmap:%s] A packet has been discarded due to race condition on the output queue. Are you really running the I/O subsystem with a single thread? output_queue %u left, %u buckets left\n",
				of_port_state->name,
				q_id,
				num_of_buckets);
		
			assert(0);
			break;
		}
	
		TM_STAMP_STAGE(pkt, TM_SA7);
		
		pkt_x86 = (datapacketx86*) pkt->platform_state;

		if(unlikely(pkt_x86->get_buffer_length() > mps)){
			//This should NEVER happen
			ROFL_ERR("[mmap:%s] Packet length above the Max Packet Size (MPS). Packet length: %u, MPS %u.. discarding\n", of_port_state->name, pkt_x86->get_buffer_length(), mps);
			assert(0);
		
			//Return buffer to the pool
			bufferpool::release_buffer(pkt);
		
			//Increment errors
			switch_port_stats_inc_lockless(of_port_state, 0, 0, 0, 0, 0, 1);	
			port_queue_stats_inc_lockless(&of_port_state->queues[q_id], 0, 0, 1);
			deferred_drain++;
			continue;
		}else{	
			fill_tx_slot(hdr, pkt_x86);
		}
		
		TM_STAMP_STAGE(pkt, TM_SA8);
		
		//Return buffer to the pool
		bufferpool::release_buffer(pkt);


		// todo statistics
		tx_bytes_local += hdr->tp_len;
		cnt++;
		deferred_drain++;
	}
	
	//Empty reading pipe (batch)
	empty_pipe();


	//Increment stats and return
	if (cnt) {
		ROFL_DEBUG_VERBOSE("[mmap:%s] schedule %u packet(s) to be send\n", __FUNCTION__, cnt);

		// send packets in TX
		if(tx->send() != ROFL_SUCCESS){
			ROFL_ERR("[mmap:%s] ERROR while sending packets. This is due very likely to an invalid ETH_TYPE value. Now the port will be reset in order to continue operation\n", of_port_state->name);
			assert(0);
			switch_port_stats_inc_lockless(of_port_state, 0, 0, 0, 0, 0, cnt);	
			port_queue_stats_inc_lockless(&of_port_state->queues[q_id], 0, 0, cnt);	
			
			/*
			* We need to reset the port, meaning destroy and regenerate both TX rings
			* Disabling and enabling the port to accomplish so.
			*/
			if(tx){
				delete tx;
				tx = new mmap_tx(std::string(of_port_state->name), block_size, n_blocks, frame_size); 
			}	
			
			
			//Making sure fds are regenerated by manually incrementing pg hash	
			portgroup_state* pg = iomanager::get_group(iomanager::get_group_id_by_port((ioport*)this));	
			if(!pg){
				assert(0);
				ROFL_DEBUG("[mmap:%s] ERROR: Unable to update port-group hash. The port might be left unusable\n", of_port_state->name);
			}else
				ROFL_DEBUG("[mmap:%s] Port reset was successful\n", of_port_state->name);
			pg->running_hash++;
					
		}

		//Increment statistics
		switch_port_stats_inc_lockless(of_port_state, 0, cnt, 0, tx_bytes_local, 0, 0);	
		port_queue_stats_inc_lockless(&of_port_state->queues[q_id], cnt, tx_bytes_local, 0);	
	}

	// return not used buckets
	return num_of_buckets;
}

/*
*
* Enable and disable port routines
*
*/
rofl_result_t ioport_mmapv2::enable() {
	
	struct ifreq ifr;
	int sd, rc;
        struct ethtool_value eval;

	ROFL_DEBUG("[mmap:%s] Trying to enable port\n",of_port_state->name);
	
	if ((sd = socket(AF_PACKET, SOCK_RAW, 0)) < 0){
		return ROFL_FAILURE;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, of_port_state->name);

	if ((rc = ioctl(sd, SIOCGIFINDEX, &ifr)) < 0){
		return ROFL_FAILURE;
	}

	/*
	* Make sure we are disabling Generic and Large Receive Offload from the NIC.
	* This screws up the MMAP
	*/

	//First retrieve the current gro setup, so that we can gently
	//inform the user we are going to disable (and not set it back)
	eval.cmd = ETHTOOL_GGRO;
	ifr.ifr_data = (caddr_t)&eval;
	eval.data = 0;//Make valgrind happy

	if (ioctl(sd, SIOCETHTOOL, &ifr) < 0) {
		ROFL_WARN("[mmap:%s] Unable to detect if the Generic Receive Offload (GRO) feature on the NIC is enabled or not. Please make sure it is disabled using ethtool or similar...\n", of_port_state->name);
		
	}else{
		//Show nice messages in debug mode
		if(eval.data == 0){
			ROFL_DEBUG("[mmap:%s] GRO already disabled.\n", of_port_state->name);
		}else{
			//Do it
			eval.cmd = ETHTOOL_SGRO;
			eval.data = 0;
			ifr.ifr_data = (caddr_t)&eval;
			
			if (ioctl(sd, SIOCETHTOOL, &ifr) < 0) {
				ROFL_ERR("[mmap:%s] Could not disable Generic Receive Offload feature on the NIC. This can be potentially dangeros...be advised!\n",  of_port_state->name);
			}else{
				ROFL_DEBUG("[mmap:%s] GRO successfully disabled.\n", of_port_state->name);
			}

		}
	}

	//Now LRO
	eval.cmd = ETHTOOL_GFLAGS;
	ifr.ifr_data = (caddr_t)&eval;
	eval.data = 0;//Make valgrind happy

	if (ioctl(sd, SIOCETHTOOL, &ifr) < 0) {
		ROFL_WARN("[mmap:%s] Unable to detect if the Large Receive Offload (LRO) feature on the NIC is enabled or not. Please make sure it is disabled using ethtool or similar...\n", of_port_state->name);
	} else {
		if ((eval.data & ETH_FLAG_LRO) == 0) {
			//Show nice messages in debug mode
			ROFL_DEBUG("[mmap:%s] LRO already disabled.\n", of_port_state->name);
		} else {
			//Do it
			eval.cmd = ETHTOOL_SFLAGS;
			eval.data = (eval.data & ~ETH_FLAG_LRO);
			ifr.ifr_data = (caddr_t)&eval;

			if (ioctl(sd, SIOCETHTOOL, &ifr) < 0)
				ROFL_ERR("[mmap:%s] Could not disable Large Receive Offload (LRO) feature on the NIC. This can be potentially dangeros...be advised!\n",  of_port_state->name);
			else
				ROFL_DEBUG("[mmap:%s] LRO successfully disabled.\n", of_port_state->name);
		}
	}

	//Recover MTU
	memset((void*)&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, of_port_state->name, sizeof(ifr.ifr_name));
	
	if(ioctl(sd, SIOCGIFMTU, &ifr) < 0) {
		ROFL_ERR("[mmap:%s] Could not retreive MTU value from NIC. Default %u Max Packet Size(MPS) size will be used (%u total bytes). Packets exceeding this size will be DROPPED (Jumbo frames).\n",  of_port_state->name, (PORT_DEFAULT_PKT_SIZE-PORT_ETHER_LENGTH), PORT_DEFAULT_PKT_SIZE);
		mps = PORT_DEFAULT_PKT_SIZE;	
	}else{
		mps = ifr.ifr_mtu+PORT_ETHER_LENGTH;
		ROFL_DEBUG("[mmap:%s] Discovered Max Packet Size(MPS) of %u.\n",  of_port_state->name, mps);
	}

	//Recover flags
	if ((rc = ioctl(sd, SIOCGIFFLAGS, &ifr)) < 0){ 
		close(sd);
		return ROFL_FAILURE;
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
	
	//Check if is up or not
	if (IFF_UP & ifr.ifr_flags){
		
		//Already up.. Silently skip
		close(sd);

		//If tx/rx lines are not created create them
		if(!rx){	
			ROFL_DEBUG_VERBOSE("[mmap:%s] generating a new mmap_rx for RX\n",of_port_state->name);
			rx = new mmap_rx(std::string(of_port_state->name), 2 * block_size, n_blocks, frame_size);
		}
		if(!tx){
			ROFL_DEBUG_VERBOSE("[mmap:%s] generating a new mmap_tx for TX\n",of_port_state->name);
			tx = new mmap_tx(std::string(of_port_state->name), block_size, n_blocks, frame_size);
		}

		of_port_state->up = true;
		return ROFL_SUCCESS;
	}

	//Prevent race conditions with LINK/STATUS notification threads (bg)
	pthread_rwlock_wrlock(&rwlock);

	ifr.ifr_flags |= IFF_UP;
	if ((rc = ioctl(sd, SIOCSIFFLAGS, &ifr)) < 0){
		ROFL_DEBUG("[mmap:%s] Unable to bring interface down via ioctl\n",of_port_state->name);
		close(sd);
		pthread_rwlock_unlock(&rwlock);
		return ROFL_FAILURE;
	}
	
	//Release mutex		
	pthread_rwlock_unlock(&rwlock);

	//If tx/rx lines are not created create them
	if(!rx){	
		ROFL_DEBUG_VERBOSE("[mmap:%s] generating a new mmap_rx for RX\n",of_port_state->name);
		rx = new mmap_rx(std::string(of_port_state->name), 2 * block_size, n_blocks, frame_size);
	}
	if(!tx){
		ROFL_DEBUG_VERBOSE("[mmap:%s] generating a new mmap_tx for TX\n",of_port_state->name);
		tx = new mmap_tx(std::string(of_port_state->name), block_size, n_blocks, frame_size);
	}

	// todo recheck?
	// todo check link state IFF_RUNNING
	of_port_state->up = true;

	close(sd);
	return ROFL_SUCCESS;
}

rofl_result_t ioport_mmapv2::disable() {
	
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

	//Prevent race conditions with LINK/STATUS notification threads (bg)
	pthread_rwlock_wrlock(&rwlock);

	ifr.ifr_flags &= ~IFF_UP;

	if ((rc = ioctl(sd, SIOCSIFFLAGS, &ifr)) < 0) {
		ROFL_DEBUG("[mmap:%s] Unable to bring interface down via ioctl\n",of_port_state->name);
		close(sd);
		pthread_rwlock_unlock(&rwlock);
		return ROFL_FAILURE;
	}

	//Release mutex		
	pthread_rwlock_unlock(&rwlock);


	// todo recheck?
	// todo check link state IFF_RUNNING
	of_port_state->up = false;

	close(sd);

	return ROFL_SUCCESS;
}
