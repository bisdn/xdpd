/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EPOLL_IOSCHEDULER_H
#define EPOLL_IOSCHEDULER_H 

#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h> 
#include <vector> 
#include <iostream> 
#include "ioscheduler.h" 
#include "../ports/ioport.h"
#include "../../util/safevector.h"

/**
* @file epoll_ioscheduler.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief I/O scheduler abstract class which defines the way
* way I/O threads go through the ports for TX and RX.
*/

namespace xdpd {
namespace gnu_linux {

class portgroup_state;

//Hold fd AND ioport reference
typedef struct epoll_event_data{
	int fd;
	ioport* port;
}epoll_event_data_t;

/**
* @brief I/O scheduler base class which defines the way
* way I/O threads go through the ports for TX and RX.
*
* @ingroup fm_gnu_linux_io_schedulers
* 
* @description Implements a simple I/O scheduler based on async epoll
* events.
* 
* It uses a weighted round-robin approach to implement
* scheduling policy.
*/
class epoll_ioscheduler: public ioscheduler{ 

public:
	//Main method inherited from ioscheduler
	static void* process_io(void* grp);

protected:
	/* EPOLL stuff */
	static const unsigned int EPOLL_TIMEOUT_MS=200;

	//FDs constants
	static const unsigned int READ=0;
	static const unsigned int WRITE=1;

	/* WRR stuff */
	//READing buckets
	static const unsigned int READ_BUCKETS_PP=4;

	//WRITing buckets	
	static const unsigned int WRITE_BUCKETS_PP=4;
	static const float WRITE_QOS_QUEUE_FACTOR[ioport::MAX_OUTPUT_QUEUES];

	/* Methods */
	//WRR
	static bool process_port_rx(ioport* port);
	static bool process_port_tx(ioport* port);

	//EPOLL related	
	static void release_resources(int epfd, struct epoll_event* ev, struct epoll_event* events, unsigned int current_num_of_ports);
	static void add_fd_epoll(struct epoll_event* ev, int epfd, ioport* port, int fd);
	static void init_or_update_fds(portgroup_state* pg, safevector<ioport*>& ports, int* epfd, struct epoll_event** ev, struct epoll_event** events, unsigned int* current_num_of_ports, unsigned int* current_hash );


	/* Debugging stuff */
#ifdef DEBUG
public:
	//Method to by-pass processing systems.
	static void set_by_pass_processing(bool value){by_pass_processing=value;};	
private:
	static bool by_pass_processing;	
#endif


};

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* EPOLL_IOSCHEDULER_H_ */
