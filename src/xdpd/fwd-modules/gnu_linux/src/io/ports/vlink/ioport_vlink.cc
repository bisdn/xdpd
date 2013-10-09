#include "ioport_vlink.h"
#include <iostream>
#include <rofl/common/utils/c_logger.h>
#include "../../bufferpool.h" 
#include <fcntl.h>

using namespace xdpd::gnu_linux;

//Constructor and destructor
ioport_vlink::ioport_vlink(switch_port_t* of_ps, unsigned int num_queues) : ioport(of_ps,num_queues){
	
	int flags,i;

	//Open pipe to simulate socket input fd 
	if(pipe(rx_notify_pipe) < 0)
		throw -1;
	if(pipe(tx_notify_pipe) < 0)
		throw -1;

	//Mark as non-blocking
	for(i=0;i<2;i++){
		flags = fcntl(rx_notify_pipe[i], F_GETFL, 0);	///get current file status flags
		flags |= O_NONBLOCK;				//turn off blocking flag
		fcntl(rx_notify_pipe[i], F_SETFL, flags);	//set up non-blocking read
	}

	for(i=0;i<2;i++){
		flags = fcntl(tx_notify_pipe[i], F_GETFL, 0);	///get current file status flags
		flags |= O_NONBLOCK;				//turn off blocking flag
		fcntl(tx_notify_pipe[i], F_SETFL, flags);	//set up non-blocking read
	}
}

ioport_vlink::~ioport_vlink(){
	close(rx_notify_pipe[READ]);
	close(rx_notify_pipe[WRITE]);
	close(tx_notify_pipe[READ]);
	close(tx_notify_pipe[WRITE]);
}

//Set other vlink edge
void ioport_vlink::set_connected_port(ioport_vlink* c_port){
	connected_port = c_port;
}

//Read and write methods over port
void ioport_vlink::enqueue_packet(datapacket_t* pkt, unsigned int q_id){
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
			ROFL_DEBUG("[vlink:%s] Packet(%p) trying to be enqueued in an invalid q_id: %u\n",  of_port_state->name, pkt, q_id);
			q_id = 0;
			bufferpool::release_buffer(pkt);
			assert(0);
		}
	
		//Store on queue and exit. This is NOT copying it to the vlink buffer
		if(output_queues[q_id].non_blocking_write(pkt) != ROFL_SUCCESS){
			ROFL_DEBUG("[vlink:%s] Packet(%p) dropped. Congestion in output queue: %d\n",  of_port_state->name, pkt, q_id);
			//Drop packet
			bufferpool::release_buffer(pkt);
			return;
		}

		ROFL_DEBUG_VERBOSE("[vlink:%s] Packet(%p) enqueued, buffer size: %d\n",  of_port_state->name, pkt, output_queues[q_id].size());
	
		//TODO: make it happen only if thread is really sleeping...
		ret = ::write(tx_notify_pipe[WRITE],&c,sizeof(c));
		(void)ret; // todo use the value
	} else {
		if(len < MIN_PKT_LEN){
			ROFL_ERR("[vlink:%s] ERROR: attempt to send invalid packet size for packet(%p) scheduled for queue %u. Packet size: %u\n", of_port_state->name, pkt, q_id, len);
			assert(0);
		}else{
			ROFL_DEBUG_VERBOSE("[vlink:%s] dropped packet(%p) scheduled for queue %u\n", of_port_state->name, pkt, q_id);
		}

		//Drop packet
		bufferpool::release_buffer(pkt);
	}


}

inline void ioport_vlink::empty_pipe(int* pipe){
	//Whatever
	char c;
	int ret;

	//Just take the byte from the pipe	
	ret = ::read(pipe[READ],&c,sizeof(c));
	(void)ret; // todo use the value
}

datapacket_t* ioport_vlink::read(){

	datapacket_t* pkt = input_queue.non_blocking_read();
		
	//Attempt to read one byte from the pipe
	if(pkt){
		datapacketx86* pkt_x86 = (datapacketx86*) pkt->platform_state;
		empty_pipe(rx_notify_pipe);
		//FIXME statistics
		pkt_x86->in_port = of_port_state->of_port_num;
	}

	return pkt;
}


unsigned int ioport_vlink::write(unsigned int q_id, unsigned int num_of_buckets){

	datapacket_t* pkt;
	//unsigned int cnt = 0;
	//int tx_bytes_local = 0;

	circular_queue<datapacket_t, IO_IFACE_RING_SLOTS>* queue = &output_queues[q_id];

	// read available packets from incoming buffer
	for ( ; 0 < num_of_buckets; --num_of_buckets ) {

		//Retrieve the buffer
		pkt = queue->non_blocking_read();
		
		if(!pkt)
			break;
		
		empty_pipe(tx_notify_pipe);
		
		//Store in the input queue in the 
		if(connected_port->tx_pkt(pkt) != ROFL_SUCCESS){
			//Congestion in the input queue of the vlink, drop
			bufferpool::release_buffer(pkt);
		}

	}

	//FIXME statistics
	
	return num_of_buckets;
}

rofl_result_t ioport_vlink::tx_pkt(datapacket_t* pkt){

	int ret;
	char c='a';

	if (unlikely(of_port_state->up == false))
		return ROFL_FAILURE;

	if(input_queue.non_blocking_write(pkt) == ROFL_FAILURE){
		return ROFL_FAILURE;
	}

	//Notify read event and return
	ret = ::write(rx_notify_pipe[WRITE],&c,sizeof(c));
	(void)ret;
	
	return ROFL_SUCCESS;
}

rofl_result_t ioport_vlink::disable(){
	of_port_state->up = false;
	return ROFL_SUCCESS;
}

rofl_result_t ioport_vlink::enable(){
	of_port_state->up = true;
	return ROFL_SUCCESS;
}
