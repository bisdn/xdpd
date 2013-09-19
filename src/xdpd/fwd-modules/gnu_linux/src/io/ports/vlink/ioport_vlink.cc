#include "ioport_vlink.h"
#include <iostream>
#include <rofl/common/utils/c_logger.h>
#include "../../bufferpool.h" 
#include <fcntl.h>

//Constructor and destructor
ioport_vlink::ioport_vlink(switch_port_t* of_ps, unsigned int num_queues) : ioport(of_ps,num_queues){
	
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

ioport_vlink::~ioport_vlink(){
	close(input[READ]);
	close(input[WRITE]);
	close(notify_pipe[READ]);
	close(notify_pipe[WRITE]);
}

//Set other vlink edge
void ioport_vlink::set_connected_port(ioport_vlink* connected_port){

}

//Read and write methods over port
void ioport_vlink::enqueue_packet(datapacket_t* pkt, unsigned int q_id){

}

datapacket_t* ioport_vlink::read(){
	
	return NULL;
}


unsigned int ioport_vlink::write(unsigned int q_id, unsigned int num_of_buckets){

	return 0;
}

rofl_result_t ioport_vlink::disable(){
	return ROFL_SUCCESS;
}

rofl_result_t ioport_vlink::enable(){
	return ROFL_SUCCESS;
}
