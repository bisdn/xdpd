/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOPORTV2_MMAP_H
#define IOPORTV2_MMAP_H 

#include <string>

#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <rofl/common/cmacaddr.h>

#include "../ioport.h"
#include "mmap_rx.h"
#include "mmap_tx.h"
#include "../../datapacketx86.h"

namespace xdpd {
namespace gnu_linux {

#define PORT_ETHER_LENGTH 18
#define PORT_DEFAULT_PKT_SIZE 1518

/**
* @file ioport_mmapv2.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief GNU/Linux interface access via Memory Mapped
* region (MMAP) using PF_PACKET TX/RX rings 
*/


/**
* @brief GNU/Linux interface access via Memory Mapped
* region (MMAP) using PF_PACKET TX/RX rings (v2) 
*
* @ingroup fm_gnu_linux_io_ports
*/
class ioport_mmapv2 : public ioport{


public:
	//ioport_mmapv2
	ioport_mmapv2(
			/*int port_no,*/
			switch_port_t* of_ps,
			int block_size = IO_IFACE_MMAP_BLOCK_SIZE,
			int n_blocks = IO_IFACE_MMAP_BLOCKS,
			int frame_size = IO_IFACE_MMAP_FRAME_SIZE,
			unsigned int num_queues = IO_IFACE_NUM_QUEUES);

	virtual
	~ioport_mmapv2();

	//Enque packet for transmission(blocking)
	virtual void enqueue_packet(datapacket_t* pkt, unsigned int q_id);


	//Non-blocking read and write
	virtual datapacket_t* read(void);

	virtual unsigned int write(unsigned int q_id, unsigned int num_of_buckets);

	// Get read fds. Return -1 if do not exist
	inline virtual int
	get_read_fd(void){
		if(rx)
			return rx->get_fd();
		return -1;
	};

	// Get write fds. Return -1 if do not exist
	inline virtual int get_write_fd(void){
		return notify_pipe[READ];
	};

	unsigned int get_port_no() {
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

private:
	
	//Minimum frame size (ethernet header size)
	static const unsigned int MIN_PKT_LEN=14;
	
	//mmap internals
	mmap_rx* rx;
	mmap_tx* tx;

	//parameters for regenerating tx/rx
	int block_size;
	int n_blocks;
	int frame_size;
	int deferred_drain;

	//Pipe used to
	int notify_pipe[2];
	
	//Used to drain the pipe
	char draining_buffer[IO_IFACE_RING_SLOTS];
	

	//Pipe extremes
	static const unsigned int READ=0;
	static const unsigned int WRITE=1;

	void fill_vlan_pkt(struct tpacket2_hdr *hdr, datapacketx86 *pkt_x86);
	void fill_tx_slot(struct tpacket2_hdr *hdr, datapacketx86 *packet);
	void empty_pipe(void);
};

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* IOPORTV2_MMAP_H_ */
