/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOPORT_MMAP_H
#define IOPORT_MMAP_H 

#include <string>

#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <rofl/common/cmacaddr.h>

#include "mmap_int.h"
#include "../ioport.h"
#include "../../datapacketx86.h"

/**
* @file ioport_mmap.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief GNU/Linux interface access via Memory Mapped
* region (MMAP). THIS VERSION IS DEPRECATED! 
*
*/

namespace xdpd {
namespace gnu_linux {

//fwd decl
class packet_mmap;

/**
* @brief GNU/Linux interface access via Memory Mapped
* region (MMAP). THIS VERSION IS DEPRECATED! 
*
* @ingroup fm_gnu_linux_io_ports
*/
class ioport_mmap : public ioport{


public:
	//ioport_mmap
	ioport_mmap(
			/*int port_no,*/
			switch_port_t* of_ps,
			int block_size = 96,
			int n_blocks = 2,
			int frame_size = 2048,
			unsigned int num_queues = MMAP_DEFAULT_NUM_OF_QUEUES);

	virtual
	~ioport_mmap();

	//Enque packet for transmission(blocking)
	virtual void
	enqueue_packet(datapacket_t* pkt, unsigned int q_id);


	/**
	 * this function also blocks if the bufferpool is empty
	 *
	 * @param fd
	 * @param read_max
	 * @return
	 */
	int
	read_loop(int fd, int read_max);

	//Non-blocking read and write
	virtual datapacket_t*
	read(void);

	virtual unsigned int
	write(unsigned int q_id, unsigned int num_of_buckets);

	// Get read fds. Return -1 if do not exist
	inline virtual int
	get_read_fd(void){
		if(rx)
			return rx->sd;
		return -1;
	};

	// Get write fds. Return -1 if do not exist
	inline virtual int
	get_write_fd(void){
		return notify_pipe[READ];
	};

	unsigned int
	get_port_no() {
		/* FIXME: probably a check whether of_port_state is not null in the constructor will suffice*/
		if(of_port_state)
			return of_port_state->of_port_num;
		else
			return 0;
	}


	/**
	 * Sets the port administratively up. This MUST change the of_port_state appropiately
	 */
	virtual rofl_result_t enable(void);

	/**
	 * Sets the port administratively down. This MUST change the of_port_state appropiately
	 */
	virtual rofl_result_t disable(void);


protected:
	//Queues
	static const unsigned int MMAP_DEFAULT_NUM_OF_QUEUES = 8;

private:
	
	//mmap internals
	mmap_int* rx;
	mmap_int* tx;

	//parameters for regenerating tx/rx
	int block_size;
	int n_blocks;
	int frame_size;

	/* todo move to parent? */
	cmacaddr hwaddr;
	
	//Pipe used to
	int notify_pipe[2];
	
	//Pipe extremes
	static const unsigned int READ=0;
	static const unsigned int WRITE=1;
};

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* IOPORT_MMAP_H_ */
