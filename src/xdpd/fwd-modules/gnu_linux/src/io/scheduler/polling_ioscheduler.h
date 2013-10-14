/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef POLLING_IOSCHEDULER_H
#define POLLING_IOSCHEDULER_H 

#include <stdlib.h>
#include <pthread.h> 
#include <vector> 
#include <iostream> 
#include "ioscheduler.h" 
#include "../ports/ioport.h"

#include <rofl/datapath/pipeline/openflow/of_switch.h>

/**
* @file polling_ioscheduler.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief (experimental) I/O scheduler base class which defines the way
* way I/O threads go through the ports for TX and RX.
*/

namespace xdpd {
namespace gnu_linux {

class portgroup_state;

/**
* @brief (experimental) I/O scheduler base class which defines the way
* way I/O threads go through the ports for TX and RX.
*
* @ingroup fm_gnu_linux_io_schedulers
*
* @description Implements a simple I/O scheduler based on pure polling over
* the interfaces. This scheduler sacrifices 1 virtual/real CPU
* core for the I/O over the portgroup. Be advised that the usage
* of the CPU by this scheduler will be around 100%. Optimal #threads
* per portgroup using this scheduler is 1 per portgroup. 
* 
* It uses a weighted round-robin approach to implement
* scheduling policy.
*
* @warning this is an experimental scheduler. If you don't know what
* you are doing use epoll_ioscheduler instead
*/
class polling_ioscheduler: public ioscheduler{ 

public:
	//Main method inherited from ioscheduler
	static void* process_io(void* grp);

protected:
	/* POLLING stuff */
	static const unsigned int POLLING_TIMEOUT_MS=200;

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
	static void process_port_io(ioport* port);

	//Polling stuff
	static void update_running_ports(portgroup_state* pg, ioport*** running_ports, unsigned int* num_of_ports, unsigned int* current_hash);

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

#endif /* POLLING_IOSCHEDULER_H_ */
