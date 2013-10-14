#include "ioport_mockup.h"
#include <iostream>
#include <rofl/common/utils/c_logger.h>
#include "../../bufferpool.h" 
#include <fcntl.h>

using namespace xdpd::gnu_linux;

//Constructor and destructor
ioport_mockup::ioport_mockup(switch_port_t* of_ps, unsigned int num_queues):ioport(of_ps,num_queues){
	
	int ret,flags,i;

	//Open pipe to simulate socket input fd 
	ret = pipe(input);
	(void)ret; // todo use the value


	//Mark READ end as non-blocking for read
	flags = fcntl(input[READ], F_GETFL, 0);		//get current file status flags
	flags |= O_NONBLOCK;				//turn off blocking flag
	fcntl(input[READ], F_SETFL, flags);		//set up non-blocking read
		
	//Open pipe for output signaling on enqueue	
	ret = pipe(notify_pipe);
	(void)ret; // todo use the value

	//Set non-blocking read/write in the pipe
	for(i=0;i<2;i++){
		flags = fcntl(notify_pipe[i], F_GETFL, 0);	///get current file status flags
		flags |= O_NONBLOCK;				//turn off blocking flag
		fcntl(notify_pipe[i], F_SETFL, flags);		//set up non-blocking read
	}

}

ioport_mockup::~ioport_mockup(){
	close(input[READ]);
	close(input[WRITE]);
	close(notify_pipe[READ]);
	close(notify_pipe[WRITE]);
}

//Read and write methods over port
void ioport_mockup::enqueue_packet(datapacket_t* pkt, unsigned int q_id){

	size_t ret;
	//Whatever
	const char c='a';

	//Put in the queue
	output_queues[q_id].blocking_write(pkt);

	//TODO: make it happen only if thread is really sleeping...
	ret = ::write(notify_pipe[WRITE],&c,sizeof(c));
	(void)ret; // todo use the value

}

datapacket_t* ioport_mockup::read(){
	
	datapacket_t* pkt; 
	datapacketx86* pkt_x86;


	//First attempt drain local buffers from previous reads that failed to push 
	pkt = input_queue.non_blocking_read();	
	if(pkt)
		return pkt;		

	//Allocate free buffer	
	pkt = bufferpool::get_free_buffer();
	pkt_x86 = ((datapacketx86*)pkt->platform_state);

	//Init in user space
	pkt_x86->init(NULL, SIMULATED_PKT_SIZE, of_port_state->attached_sw, of_port_state->of_port_num);
	
	//copy something from stdin to buffer
	if(::read(input[READ],pkt_x86->get_buffer(),SIMULATED_PKT_SIZE) <0){
		bufferpool::release_buffer(pkt);
		return NULL;
	}

	ROFL_DEBUG_VERBOSE("Filled buffer with id:%d. Sending to process.\n", pkt_x86->buffer_id);
	
	return pkt;	
}


unsigned int ioport_mockup::write(unsigned int q_id, unsigned int num_of_buckets){

	size_t ret;
	uint8_t dummy[SIMULATED_PKT_SIZE];
	unsigned int i;
	datapacket_t* pkt;
	datapacketx86* pkt_x86;
	
	(void)pkt_x86;

	//Free the pipe
	ret = ::read(notify_pipe[READ],&dummy,SIMULATED_PKT_SIZE);
	(void)ret; // todo use the value

	//Go and do stuff
	for(i=0;i<num_of_buckets;i++){	
		//Pick buffer	
		pkt = output_queues[q_id].non_blocking_read();
		
		if(!pkt)
			break;
		
		//Just "put it into the wire" -> print it 
		pkt_x86 = (datapacketx86*)pkt->platform_state;
		
		ROFL_DEBUG_VERBOSE("Getting buffer with id:%d. Putting it into the wire\n", pkt_x86->buffer_id);
		
		//Free buffer
		bufferpool::release_buffer(pkt);
	}
	i++;
	return num_of_buckets-i;

}

rofl_result_t ioport_mockup::disable(){
	return ROFL_SUCCESS;
}

rofl_result_t ioport_mockup::enable(){
	return ROFL_SUCCESS;
}
