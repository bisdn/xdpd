#include "epoll_ioscheduler.h"

#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <rofl/datapath/pipeline/platform/cutil.h>
#include "../iomanager.h"
#include "../bufferpool.h"
#include "../../util/circular_queue.h"

/*
* 
* Implements a simple WRR scheduling algorithm within port-group 
* To be further explored other I/O algorithms
*
*/

//Static members initialization
const float epoll_ioscheduler::WRITE_QOS_QUEUE_FACTOR[ioport::MAX_OUTPUT_QUEUES]={1,1.2,1.5,2,2.2,2.5,2.7,3.0};
#ifdef DEBUG
bool epoll_ioscheduler::by_pass_processing = false;
#endif

/*
* Call port based on scheduling algorithm 
*/
inline void epoll_ioscheduler::process_port_io(ioport* port){

	unsigned int i, q_id, n_buckets;
	datapacket_t* pkt;
	
	if(!port || !port->of_port_state)
		return;

	//Perform up_to n_buckets_read
	ROFL_DEBUG_VERBOSE("Trying to read at port %s with %d\n", port->of_port_state->name, n_buckets);
	
	for(i=0; i<READ_BUCKETS_PP; ++i){
		
		//Attempt to read (non-blocking)
		pkt = port->read();
		
		if(pkt){

#ifdef DEBUG
			if(by_pass_processing){
				//By-pass processing and schedule to write in the same port
				//Only used for testing
				port->enqueue_packet(pkt,0); //Push to queue 0
			}else{
#endif
				/*
				* Push packet to the logical switch queue. 
				* If not successful (congestion), drop it!
				*/
				if( port->get_sw_processing_queue()->non_blocking_write(pkt) != ROFL_SUCCESS ){
					//XXX: check whether resources in the ioport (e.g. ioport_mmap) can be released only by that (maybe virtual function called by ioport)
					ROFL_DEBUG_VERBOSE("[%s] Packet(%p) DROPPED, buffer from sw:%s is FULL\n", port->of_port_state->name, pkt, port->of_port_state->attached_sw->name);
					bufferpool::release_buffer(pkt);
				}else{
					ROFL_DEBUG_VERBOSE("[%s] Packet(%p) scheduled for process -> sw: %s\n", port->of_port_state->name, pkt, port->of_port_state->attached_sw->name);
				}
					
#ifdef DEBUG
			}
#endif
		}else{
			ROFL_DEBUG_VERBOSE("[%s] reading finished at: %d/%d\n", port->of_port_state->name, i, n_buckets);
			break;
		}
	}

	//Process output (up to WRITE_BUCKETS[output_queue_state])
	for(q_id=0; q_id < IO_IFACE_NUM_QUEUES; ++q_id){
		
		//Increment number of buckets
		n_buckets = WRITE_BUCKETS_PP*WRITE_QOS_QUEUE_FACTOR[q_id];

		ROFL_DEBUG_VERBOSE("[%s] Trying to write at port queue: %d with n_buckets: %d.\n", port->of_port_state->name, q_id, n_buckets);
		
		//Perform up to n_buckets write	
		port->write(q_id, n_buckets);
	}

}

/*
* EPOLL add fd
*/
inline void epoll_ioscheduler::add_fd_epoll(struct epoll_event* ev, int epfd, ioport* port, int fd){

	ev->events = EPOLLIN | EPOLLPRI /*| EPOLLET*/;
	ev->data.fd = fd;
	ev->data.ptr = (void*)port;

	if( epoll_ctl(epfd, EPOLL_CTL_ADD, fd, ev) < 0){
		ROFL_ERR("[epoll_ioscheduler] epoll failed");
		assert(0);
	}
}
/*
* Initializes or updates the EPOLL file descriptor set (epoll_ctl)
*/
inline void epoll_ioscheduler::init_or_update_fds(portgroup_state* pg, int* epfd, struct epoll_event** ev, struct epoll_event** events, unsigned int* current_num_of_ports, unsigned int* current_hash ){

	unsigned int i;
	int fd;

	//If there are no running_ports just skip
	if(!pg->running_ports->size())
		return;

	//Destroy previous epoll instance, if any
	if(*epfd != -1){
		close(*epfd);	
		free(*ev);
		free(*events);
	}
	
	//Create epoll
	*epfd = epoll_create(pg->running_ports->size()*2);

	//lock running vector, so that we can safely iterate over it
	pg->running_ports->read_lock();

	//Allocate memory
	*ev = (epoll_event*)malloc( sizeof(struct epoll_event) * pg->running_ports->size()*2 );
	*events = (epoll_event*)malloc( sizeof(struct epoll_event) * pg->running_ports->size()*2 );

	if(!*ev){
		//FIXME: what todo...
		ROFL_ERR("[epoll_ioscheduler] malloc failed");
		pg->running_ports->read_unlock();
		return;
	}

	//Assign current number_of_ports
	*current_num_of_ports = pg->running_ports->size();	

	for(i=0; i < *current_num_of_ports; i++){
		/* Read */
		fd = (*pg->running_ports)[i]->get_read_fd();
		if( fd != -1 )
			epoll_ioscheduler::add_fd_epoll( &((*ev)[(i*2)+READ]), *epfd, (*pg->running_ports)[i], fd);
		
		/* Write */
		fd = (*pg->running_ports)[i]->get_write_fd();
		if( fd != -1 )
			epoll_ioscheduler::add_fd_epoll(&((*ev)[(i*2)+WRITE]), *epfd, (*pg->running_ports)[i], fd);
	}

	//Assign current hash
	*current_hash = pg->running_hash;	

	//Signal as synchronized
	iomanager::signal_as_synchronized(pg);
	
	//unlock running vector
	pg->running_ports->read_unlock();
}


void* epoll_ioscheduler::process_io(void* grp){

	int epfd, res;
	struct epoll_event *ev=NULL, *events = NULL;
	unsigned int current_hash=0, current_num_of_ports=0;
	portgroup_state* pg = (portgroup_state*)grp;
 
	//Init epoll fd set
	epfd = -1;
	init_or_update_fds(pg, &epfd, &ev, &events, &current_num_of_ports, &current_hash );

	ROFL_DEBUG_VERBOSE("[epoll_ioscheduler] Initialization of epoll completed in thread:%d\n",pthread_self());
	
	/*
	* Infinite loop unless group is stopped. e.g. all ports detached
	*/
	while(iomanager::keep_on_working(pg)){

		//Wait for events or TIMEOUT_MS
		res = epoll_wait(epfd, events, current_num_of_ports, EPOLL_TIMEOUT_MS);
		
		if(res == -1){
			ROFL_DEBUG_VERBOSE("[epoll_ioscheduler] epoll returned -1. Continuing...\n");
			continue;
		}
		if(res == 0){
			//std::cerr<<"Timeout.."<<std::endl;
			//Check if this is really necessary
			//Timeout loop over ALL fds 
			//for(unsigned int i=0;i<current_num_of_ports*2;i+=2){
				//epoll_ioscheduler::process_port_io((ioport*)ev[i].data.ptr);
			//}	
		}else{	
			//std::cerr<<"Active fds.."<<std::endl;
			//Loop over active fd
			//FIXME: skip double port process_port_io (both input&output fd signal)
			for(int i = 0;i<res;i++){
				epoll_ioscheduler::process_port_io((ioport*)events[i].data.ptr);
			}	
		}
		//Check for updates in the running ports 
		if( pg->running_hash != current_hash )
			init_or_update_fds(pg, &epfd, &ev,&events, &current_num_of_ports, &current_hash );
	}

	ROFL_INFO("Finishing execution of I/O thread: #%u\n",pthread_self());

	//Free dynamic memory
	free(ev);
	free(events);

	//Return whatever
	pthread_exit(NULL);
}
