#include "epoll_ioscheduler.h"

#include <assert.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/switch_port.h>

/*
* 
* Implements a simple WRR scheduling algorithm within port-group 
* To be further explored other I/O algorithms
*
*/

using namespace xdpd::gnu_linux;

//Static members initialization
const float epoll_ioscheduler::WRITE_QOS_QUEUE_FACTOR[ioport::MAX_OUTPUT_QUEUES]={1,1.2,1.5,2,2.2,2.5,2.7,3.0};
#ifdef DEBUG
bool epoll_ioscheduler::by_pass_processing = false;
#endif

/*
* EPOLL add fd
*/
void epoll_ioscheduler::add_fd_epoll(struct epoll_event* ev, int epfd, ioport* port, int fd){

	epoll_event_data_t* port_data = (epoll_event_data_t*)malloc(sizeof(epoll_event_data_t));

	if(!port_data){
		ROFL_ERR(FWD_MOD_NAME"[epoll_ioscheduler] Could not allocate port_data memory for port %p\n", port);
		assert(0);
		return;
	}

	//Set data	
	ev->events = EPOLLIN | EPOLLPRI /*| EPOLLERR | EPOLLET*/;
	port_data->fd = fd;
	port_data->port = port;
	ev->data.ptr = (void*)port_data; //Use pointer ONLY
	
	ROFL_DEBUG_VERBOSE(FWD_MOD_NAME"[epoll_ioscheduler] Trying to add event with flags: %u and port_data:%p\n", ev->events, port_data); 

	if( epoll_ctl(epfd, EPOLL_CTL_ADD, fd, ev) < 0){
		ROFL_ERR(FWD_MOD_NAME"[epoll_ioscheduler] Insertion of the event %p in the epoll failed\n", ev);
		assert(0);
	}
}


void epoll_ioscheduler::release_resources( int epfd, struct epoll_event* ev, struct epoll_event* events, unsigned int current_num_of_ports){
	
	unsigned int i;

	if(epfd != -1){
		close(epfd);	
		//Release port_data stuff
		for(i=0;i<current_num_of_ports;++i){
			if(ev[i].data.ptr)
				free(ev[i].data.ptr);
		}
		free(ev);
		free(events);
	}
}

/*
* Initializes or updates the EPOLL file descriptor set (epoll_ctl)
*/
void epoll_ioscheduler::init_or_update_fds(portgroup_state* pg, safevector<ioport*>& ports, int* epfd, struct epoll_event** ev, struct epoll_event** events, unsigned int* current_num_of_ports, unsigned int* current_hash, bool rx ){

	unsigned int i;
	int fd;
	ioport* port;

	//If there are no running_ports just skip
	if(!pg->running_ports->size())
		return;
	
	//Destroy previous epoll instance, if any
	release_resources(*epfd, *ev, *events, *current_num_of_ports);

	//Allocate memory
	*ev = (epoll_event*)malloc( sizeof(struct epoll_event) * pg->running_ports->size());
	*events = (epoll_event*)malloc( sizeof(struct epoll_event) * pg->running_ports->size());

	if(!*ev || !*events){
	       //FIXME: what todo...
               ROFL_ERR(FWD_MOD_NAME"[epoll_ioscheduler] malloc failed");
               pg->running_ports->read_unlock();
               return;
       }
	
	//Create epoll
	*epfd = epoll_create(pg->running_ports->size()*2);
	
	if(*epfd < 0){
		ROFL_ERR(FWD_MOD_NAME"[epoll_ioscheduler] malloc failed");
		pg->running_ports->read_unlock();
		return; 
	}

	//lock running vector, so that we can safely iterate over it
	pg->running_ports->read_lock();

	//Assign current number_of_ports
	*current_num_of_ports = pg->running_ports->size();	

	for(i=0; i < *current_num_of_ports; i++){
		/* Read */
		port = (*pg->running_ports)[i];
		if(rx)
			fd = port->get_read_fd();
		else
			fd = port->get_write_fd();
			
		if( fd != -1 ){
			ROFL_DEBUG_VERBOSE(FWD_MOD_NAME"[epoll_ioscheduler] Adding %s event to epoll event list ioport: %s, fd: %d\n", (rx)? "RX":"TX", port->of_port_state->name, fd);
			epoll_ioscheduler::add_fd_epoll( &((*ev)[i]), *epfd, port, fd);
		}else
			(*ev)[i].data.ptr = NULL;
	}

	//Assign current hash
	*current_hash = pg->running_hash;	

	//Copy ports
	ports = *pg->running_ports;

	//Signal as synchronized
	iomanager::signal_as_synchronized(pg);
	
	//unlock running vector
	pg->running_ports->read_unlock();
}

void epoll_ioscheduler::set_kernel_scheduling(){

#ifndef IO_KERN_DONOT_CHANGE_SCHED
	struct sched_param sched_param;
	
	if (sched_getparam(0, &sched_param) < 0) {
		ROFL_WARN(FWD_MOD_NAME"[epoll_ioscheduler] Unable to get properties for thread on process id %u(%u): %s(%u)\n", syscall(SYS_gettid), pthread_self(), strerror(errno), errno);
		return;
	}

	//Set max
	sched_param.sched_priority = sched_get_priority_max(IO_KERN_SCHED_POL); 
	
	if(sched_setscheduler(0, IO_KERN_SCHED_POL, &sched_param) < 0){
		ROFL_WARN(FWD_MOD_NAME"[epoll_ioscheduler] Unable to set scheduling and/or priority for thread on process id %u(%u): %s(%u)\n", syscall(SYS_gettid), pthread_self(), strerror(errno), errno);
	}
		
	ROFL_DEBUG(FWD_MOD_NAME"[epoll_ioscheduler] Set scheduling policy (%u) and priority for thread on process id %u(%u)\n", IO_KERN_SCHED_POL, syscall(SYS_gettid), pthread_self());

#endif

}
