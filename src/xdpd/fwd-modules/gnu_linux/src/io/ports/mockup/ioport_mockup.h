/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOPORT_MOCKUP_H
#define IOPORT_MOCKUP_H 

#include <unistd.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include "../ioport.h" 
#include "../../datapacketx86.h" 


namespace xdpd {
namespace gnu_linux {

/**
* @brief Simple mockup of a port, used for testing purposes only.
* 
* It opens two files, on for input and another for output.
*/
class ioport_mockup : public ioport{

public:
	//ioport_mockup
	ioport_mockup(switch_port_t* of_ps, unsigned int num_queues=MMAP_DEFAULT_NUM_OF_QUEUES);
	virtual ~ioport_mockup();
	 
	//Enque packet for transmission (blocking)
	virtual void enqueue_packet(datapacket_t* pkt, unsigned int q_id);

	//Non-blocking read and write
	virtual datapacket_t* read(void);
	virtual unsigned int write(unsigned int q_id, unsigned int num_of_buckets);

	//Get read&write fds. Return -1 if do not exist
	inline virtual int get_read_fd(void){return input[READ];};
	int get_fake_write_fd(void){return input[WRITE];};
	inline virtual int get_write_fd(void){return notify_pipe[READ];};

	//Get buffer status
	//virtual circular_queue_state_t get_input_queue_state(void); 
	//virtual circular_queue_state_t get_output_queue_state(unsigned int q_id=0);

	virtual rofl_result_t 
	disable();

	virtual rofl_result_t
	enable();

	static const size_t SIMULATED_PKT_SIZE=1500;

protected:
	//Queues
	static const unsigned int MMAP_DEFAULT_NUM_OF_QUEUES=8; 

	//fds
	int input[2];
	int notify_pipe[2];
	
	//Pipe extremes
	static const unsigned int READ=0;
	static const unsigned int WRITE=1;
	
};

}// namespace xdpd::gnu_linux 
}// namespace xdpd


#endif /* IOPORT_MOCKUP_H_ */
