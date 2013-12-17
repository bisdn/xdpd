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
#include <rofl/datapath/pipeline/platform/cutil.h>
#include "../iomanager.h"
#include "../bufferpool.h"
#include "../../util/circular_queue.h"

//Profiling
#include "../../util/time_measurements.h"

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
		
		//Increment number of buckets
		n_buckets = WRITE_BUCKETS_PP*WRITE_QOS_QUEUE_FACTOR[q_id];

		ROFL_DEBUG_VERBOSE("[%s] Trying to write at port queue: %d with n_buckets: %d.\n", port->of_port_state->name, q_id, n_buckets);
		
		//Perform up to n_buckets write	
		tx_packets += n_buckets - port->write(q_id, n_buckets);
	}
	
	return tx_packets; 
}

/*
* EPOLL add fd
*/
inline void epoll_ioscheduler::add_fd_epoll(struct epoll_event* ev, int epfd, ioport* port, int fd){

	epoll_event_data_t* port_data = (epoll_event_data_t*)malloc(sizeof(epoll_event_data_t));

	if(!port_data){
		ROFL_ERR("[epoll_ioscheduler] Could not allocate port_data memory for port %p\n", port);
		assert(0);
		return;
	}

	//Set data	
	ev->events = EPOLLIN | EPOLLPRI /*| EPOLLERR | EPOLLET*/;
	port_data->fd = fd;
	port_data->port = port;
	ev->data.ptr = (void*)port_data; //Use pointer ONLY
	
	ROFL_DEBUG_VERBOSE("[epoll_ioscheduler] Trying to add event with flags: %u and port_data:%p\n", ev->events, port_data); 

	if( epoll_ctl(epfd, EPOLL_CTL_ADD, fd, ev) < 0){
		ROFL_ERR("[epoll_ioscheduler] Insertion of the event %p in the epoll failed\n", ev);
		assert(0);
	}
}


inline void epoll_ioscheduler::release_resources( int epfd, struct epoll_event* ev, struct epoll_event* events, unsigned int current_num_of_ports){
	
	unsigned int i;

	if(epfd != -1){
		close(epfd);	
		//Release port_data stuff
		for(i=0;i<current_num_of_ports;++i){
			free(ev[i].data.ptr);
		}
		free(ev);
		free(events);
	}
}

/*
* Initializes or updates the EPOLL file descriptor set (epoll_ctl)
*/
inline void epoll_ioscheduler::init_or_update_fds(portgroup_state* pg, safevector<ioport*>& ports, int* epfd, struct epoll_event** ev, struct epoll_event** events, unsigned int* current_num_of_ports, unsigned int* current_hash ){

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
               ROFL_ERR("[epoll_ioscheduler] malloc failed");
               pg->running_ports->read_unlock();
               return;
       }
	
	//Create epoll
	*epfd = epoll_create(pg->running_ports->size()*2);
	
	if(*epfd < 0){
		ROFL_ERR("[epoll_ioscheduler] malloc failed");
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
		fd = port->get_read_fd();
		if( fd != -1 ){
			ROFL_DEBUG_VERBOSE("[epoll_ioscheduler] Adding RX event to epoll event list ioport: %s, fd: %d\n",port->of_port_state->name, fd);
			epoll_ioscheduler::add_fd_epoll( &((*ev)[i]), *epfd, port, fd);
		}
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
		ROFL_WARN("[epoll_ioscheduler] Unable to get properties for thread on process id %u(%u): %s(%u)\n", syscall(SYS_gettid), pthread_self(), strerror(errno), errno);
		return;
	}

	//Set max
	sched_param.sched_priority = sched_get_priority_max(IO_KERN_SCHED_POL); 
	
	if(sched_setscheduler(0, IO_KERN_SCHED_POL, &sched_param) < 0){
		ROFL_WARN("[epoll_ioscheduler] Unable to set scheduling and/or priority for thread on process id %u(%u): %s(%u)\n", syscall(SYS_gettid), pthread_self(), strerror(errno), errno);
	}
		
	ROFL_DEBUG("[epoll_ioscheduler] Set scheduling policy (%u) and priority for thread on process id %u(%u)\n", IO_KERN_SCHED_POL, syscall(SYS_gettid), pthread_self());

#endif

}

void* epoll_ioscheduler::process_io_rx(void* grp){

	int i;
	int epfd, res;
	struct epoll_event *ev=NULL, *events = NULL;
	unsigned int current_hash=0, current_num_of_ports=0;
	portgroup_state* pg = (portgroup_state*)grp;
	ioport* port;
	safevector<ioport*> ports;	//Ports of the group currently performing I/O operations
	
	//Init epoll fd set
	epfd = -1;
	init_or_update_fds(pg, ports, &epfd, &ev, &events, &current_num_of_ports, &current_hash );

	assert(pg->type == PG_RX);

	ROFL_DEBUG("[epoll_ioscheduler] Launching I/O RX thread on process id: %u(%u) for group %u\n", syscall(SYS_gettid), pthread_self(), pg->id);
	
	ROFL_DEBUG_VERBOSE("[epoll_ioscheduler] Initialization of epoll completed in thread:%d\n",pthread_self());

	//Set scheduling and priority
	set_kernel_scheduling();

	/*
	* Infinite loop unless group is stopped. e.g. all ports detached
	*/
	while(iomanager::keep_on_working(pg)){

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
				/* Read */
				port = ((epoll_event_data_t*)events[i].data.ptr)->port;
				epoll_ioscheduler::process_port_rx(port);
			}
		}

		//Check for updates in the running ports 
		if( pg->running_hash != current_hash )
			init_or_update_fds(pg, ports, &epfd, &ev, &events, &current_num_of_ports, &current_hash );
	}

	//Release resources
	release_resources(epfd, ev, events, current_num_of_ports);

	ROFL_DEBUG("Finishing execution of the RX I/O thread: #%u\n",pthread_self());

	//Return whatever
	pthread_exit(NULL);
}

inline rofl_result_t epoll_ioscheduler::update_tx_port_list(portgroup_state* grp, unsigned int* current_num_of_ports, unsigned int* current_hash, ioport* port_list[]){

	unsigned int i;
	size_t size;

	//lock running vector, so that we can safely iterate over it
	grp->running_ports->read_lock();
	
	size = grp->running_ports->size();


	if(size > EPOLL_IOSCHEDULER_MAX_TX_PORTS_PER_PG){
		assert(0);
		//FIXME add trace
		grp->running_ports->read_unlock();
		return ROFL_FAILURE;
	}

	for(i=0;i<size;++i){
		port_list[i] = (*grp->running_ports)[i];
	}	

	//Assign current hash and size
	*current_hash = grp->running_hash;	
	*current_num_of_ports = size;	

	//Signal as synchronized
	iomanager::signal_as_synchronized(grp);
	
	//unlock running vector
	grp->running_ports->read_unlock();

	return ROFL_SUCCESS;
}

void* epoll_ioscheduler::process_io_tx(void* grp){

	unsigned int i, sem_ret;
	unsigned int current_hash=0, current_num_of_ports=0;
	portgroup_state* pg = (portgroup_state*)grp;
	ioport* port_list[EPOLL_IOSCHEDULER_MAX_TX_PORTS_PER_PG];
	sem_t* sem = &pg->tx_sem;
	struct timespec abs_timeout;
	int tx_packets;
	
	/*
	* Infinite loop unless group is stopped. e.g. all ports detached
	*/

	//Set scheduling and priority
	set_kernel_scheduling();

	assert(pg->type == PG_TX);
	ROFL_DEBUG("[epoll_ioscheduler] Launching I/O TX thread on process id: %u(%u) for group %u\n", syscall(SYS_gettid), pthread_self(), pg->id);
	
	if(update_tx_port_list(pg, &current_num_of_ports, &current_hash, port_list) != ROFL_SUCCESS){
		pthread_exit(NULL);
	}

	while(iomanager::keep_on_working(pg)){

		tx_packets=0;	
	
		//if(!more_packets){
			//cond_wait(update timeout)
			clock_gettime(CLOCK_REALTIME, &abs_timeout);
			//abs_timeout.tv_nsec += EPOLL_TIMEOUT_MS*100000000;
			abs_timeout.tv_sec += 1; //EPOLL_TIMEOUT_MS*100000000; //FIXME

			//Now wait
			sem_ret = sem_timedwait(sem, &abs_timeout);
	
			if(sem_ret < 0){
				//FIXME: trace
			}
		//}
		
		//No timeout go over the ports
		for(i=0; i<current_num_of_ports; ++i){
			tx_packets += epoll_ioscheduler::process_port_tx(port_list[i]);
		}

		int sem_value;

	
		sem_getvalue(sem, &sem_value);
		ROFL_DEBUG_VERBOSE("Value tx_packets:%d, sem value: %d \n", tx_packets, sem_value);

		//Drain credits for the (already) TX'd packets (tx_packets - 1(inital sem_wait))
		if(tx_packets){
			ROFL_DEBUG_VERBOSE("Draining :%u\n", tx_packets-1);
			for(tx_packets--;tx_packets;tx_packets--)
				sem_trywait(sem);	
		}

		//Check for updates in the running ports 
		if( unlikely(pg->running_hash != current_hash) ){
			if(unlikely(update_tx_port_list(pg, &current_num_of_ports, &current_hash, port_list) != ROFL_SUCCESS)){
				assert(0);
				pthread_exit(NULL);
			}
		}
	}

	ROFL_DEBUG("Finishing execution of the TX I/O thread: #%u\n",pthread_self());

	//Return whatever
	pthread_exit(NULL);
}
