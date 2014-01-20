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
#include "../iomanager.h"
#include "../bufferpool.h"
#include "../ports/ioport.h"
#include "../../util/safevector.h"
#include "../../util/circular_queue.h"

//Profiling
#include "../../util/time_measurements.h"

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

//Max number of ports per port_group
#define EPOLL_IOSCHEDULER_MAX_TX_PORTS_PER_PG 256

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
	template<bool is_rx>
	static void* process_io(void* grp);

protected:
	/* EPOLL stuff */
	static const unsigned int EPOLL_TIMEOUT_MS=200;

	/* WRR stuff */
	//READing buckets
	static const unsigned int READ_BUCKETS_PP=4;

	//WRITing buckets	
	static const unsigned int WRITE_BUCKETS_PP=4;
	static const float WRITE_QOS_QUEUE_FACTOR[ioport::MAX_OUTPUT_QUEUES];

	/* Methods */
	//WRR
	static inline bool process_port_rx(ioport* port);
	static inline int process_port_tx(ioport* port);

	//EPOLL related	
	static void release_resources(int epfd, struct epoll_event* ev, struct epoll_event* events, unsigned int current_num_of_ports);
	static void add_fd_epoll(struct epoll_event* ev, int epfd, ioport* port, int fd);
	static void init_or_update_fds(portgroup_state* pg, safevector<ioport*>& ports, int* epfd, struct epoll_event** ev, struct epoll_event** events, unsigned int* current_num_of_ports, unsigned int* current_hash, bool rx);

//	static rofl_result_t update_tx_port_list(portgroup_state* grp, unsigned int* current_num_of_ports, unsigned int* current_hash, ioport* port_list[]);

	static void set_kernel_scheduling(void);

	/* Debugging stuff */
#ifdef DEBUG
public:
	//Method to by-pass processing systems.
	static void set_by_pass_processing(bool value){by_pass_processing=value;};	
private:
	static bool by_pass_processing;	
#endif


};

//Inline functions and templates

/*
* Call port based on scheduling algorithm 
*/
inline bool epoll_ioscheduler::process_port_rx(ioport* port){

	unsigned int i;
	datapacket_t* pkt;
	of_switch_t* sw;
	
	if(unlikely(!port) || unlikely(!port->of_port_state) || unlikely(!port->of_port_state->attached_sw))
		return false;

	sw = port->of_port_state->attached_sw;
	
	//Perform up_to n_buckets_read
	ROFL_DEBUG_VERBOSE("Trying to read at port %s with %d\n", port->of_port_state->name, READ_BUCKETS_PP);
	
	for(i=0; i<READ_BUCKETS_PP; ++i){

		//Attempt to read (non-blocking)
		pkt = port->read();
		
		if(likely(pkt != NULL)){

#ifdef DEBUG
			if(by_pass_processing){
				//By-pass processing and schedule to write in the same port
				//Only used for testing
				port->enqueue_packet(pkt,0); //Push to queue 0
			}else{
#endif
				/*
				* Process packets
				*/
				//Process it through the pipeline
				TM_STAMP_STAGE(pkt, TM_S3);
				of_process_packet_pipeline(sw, pkt);
					
#ifdef DEBUG
			}
#endif
		}else{
			ROFL_DEBUG_VERBOSE("[%s] reading finished at: %d/%d\n", port->of_port_state->name, i, READ_BUCKETS_PP);
			break;
		}
	}
	
	return i==(READ_BUCKETS_PP);
}

inline int epoll_ioscheduler::process_port_tx(ioport* port){
	
	unsigned int q_id;
	unsigned int n_buckets;
	int tx_packets=0;

	if(unlikely(!port) || unlikely(!port->of_port_state))
		return 0;

	//Process output (up to WRITE_BUCKETS[output_queue_state])
	for(q_id=0; q_id < IO_IFACE_NUM_QUEUES; ++q_id){

		//Fas pre-check (avoid virtual function call overhead)
		if(port->output_queue_has_packets(q_id) == false)
			continue;
		
		//Increment number of buckets
		n_buckets = WRITE_BUCKETS_PP*WRITE_QOS_QUEUE_FACTOR[q_id];

		ROFL_DEBUG_VERBOSE("[%s] Trying to write at port queue: %d with n_buckets: %d.\n", port->of_port_state->name, q_id, n_buckets);
		
		//Perform up to n_buckets write	
		tx_packets += n_buckets - port->write(q_id, n_buckets);
	}
	
	return tx_packets; 
}

template<bool is_rx>
void* epoll_ioscheduler::process_io(void* grp){

	int i;
	int epfd, res;
	struct epoll_event *ev=NULL, *events = NULL;
	unsigned int current_hash=0, current_num_of_ports=0;
	portgroup_state* pg = (portgroup_state*)grp;
	ioport* port;
	safevector<ioport*> ports;	//Ports of the group currently performing I/O operations
	
	//Init epoll fd set
	epfd = -1;
	init_or_update_fds(pg, ports, &epfd, &ev, &events, &current_num_of_ports, &current_hash, is_rx);

	assert(pg->type == ((is_rx)? PG_RX:PG_TX));

	ROFL_DEBUG("[epoll_ioscheduler] Launching I/O RX thread on process id: %u(%u) for group %u\n", is_rx? "RX":"TX", syscall(SYS_gettid), pthread_self(), pg->id);
	ROFL_DEBUG_VERBOSE("[epoll_ioscheduler] Initialization of epoll completed in thread:%d\n",pthread_self());

	//Set scheduling and priority
	set_kernel_scheduling();

	/*
	* Infinite loop unless group is stopped. e.g. all ports detached
	*/
	while(likely(iomanager::keep_on_working(pg))){

		//Wait for events or TIMEOUT_MS
		res = epoll_wait(epfd, events, current_num_of_ports, EPOLL_TIMEOUT_MS);
		
		if(unlikely(res == -1)){
#ifdef DEBUG
			if (errno != EINTR){
				ROFL_DEBUG("[epoll_ioscheduler] epoll returned -1 (%s) Continuing...\n",strerror(errno));
				assert(0);
			}
#endif
			continue;
		}

		if(unlikely(res == 0)){
			//Timeout
		}else{	
			ROFL_DEBUG_VERBOSE("[epoll_ioscheduler] Got %d events\n", res); 
			for(i=0; i<res; ++i){
				
				port = ((epoll_event_data_t*)events[i].data.ptr)->port;
				
				if(is_rx)
					epoll_ioscheduler::process_port_rx(port);
				else
					epoll_ioscheduler::process_port_tx(port);
			}
		}

		//Check for updates in the running ports 
		if( unlikely(pg->running_hash != current_hash) )
			init_or_update_fds(pg, ports, &epfd, &ev, &events, &current_num_of_ports, &current_hash, is_rx);
	}

	//Release resources
	release_resources(epfd, ev, events, current_num_of_ports);

	ROFL_DEBUG("Finishing execution of the %s I/O thread: #%u\n", is_rx? "RX":"TX", pthread_self());

	//Return whatever
	pthread_exit(NULL);
}

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* EPOLL_IOSCHEDULER_H_ */
