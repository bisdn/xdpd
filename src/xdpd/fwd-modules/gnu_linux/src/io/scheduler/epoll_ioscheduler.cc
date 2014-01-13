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
	circular_queue<datapacket_t, PROCESSING_INPUT_QUEUE_SLOTS>* sw_queue;
	
	if(unlikely(!port) || unlikely(!port->of_port_state))
		return false;
	
	//Retrieve attached queue
	sw_queue = port->get_sw_processing_queue();
	
	//Perform up_to n_buckets_read
	ROFL_DEBUG_VERBOSE("Trying to read at port %s with %d\n", port->of_port_state->name, READ_BUCKETS_PP);
	
	for(i=0; i<READ_BUCKETS_PP; ++i){

		//First check wheather the switch is full
		//Avoid retrieving packets which the switch may not 
		//be able to process
		if( unlikely(sw_queue->is_full()) )
			return true;	
	
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
				//Timestamp S3_PRE
				TM_STAMP_STAGE(pkt, TM_S3_PRE);
	
				if( sw_queue->non_blocking_write(pkt) != ROFL_SUCCESS ){
					//XXX: check whether resources in the ioport (e.g. ioport_mmap) can be released only by that (maybe virtual function called by ioport)
					ROFL_DEBUG_VERBOSE("[%s] Packet(%p) DROPPED, buffer from sw:%s is FULL\n", port->of_port_state->name, pkt, port->of_port_state->attached_sw->name);
					bufferpool::release_buffer(pkt);
				
					TM_STAMP_STAGE(pkt, TM_S3_FAILURE);
				}else{
					TM_STAMP_STAGE(pkt, TM_S3_SUCCESS);
					ROFL_DEBUG_VERBOSE("[%s] Packet(%p) scheduled for process -> sw: %s\n", port->of_port_state->name, pkt, port->of_port_state->attached_sw->name);
					
				}
					
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

inline bool epoll_ioscheduler::process_port_tx(ioport* port){
	
	unsigned int q_id;
	unsigned int n_buckets;
	bool pkts_remaining = false; 

	if(unlikely(!port) || unlikely(!port->of_port_state))
		return pkts_remaining;

	//Process output (up to WRITE_BUCKETS[output_queue_state])
	for(q_id=0; q_id < IO_IFACE_NUM_QUEUES; ++q_id){
		
		//Increment number of buckets
		n_buckets = WRITE_BUCKETS_PP*WRITE_QOS_QUEUE_FACTOR[q_id];

		ROFL_DEBUG_VERBOSE("[%s] Trying to write at port queue: %d with n_buckets: %d.\n", port->of_port_state->name, q_id, n_buckets);
		
		//Perform up to n_buckets write	
		if(port->write(q_id, n_buckets) == 0)
			pkts_remaining=true;
	}
	
	return pkts_remaining;
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
			free(ev[(i*2)+READ].data.ptr);
			free(ev[(i*2)+WRITE].data.ptr);
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
	*ev = (epoll_event*)malloc( sizeof(struct epoll_event) * pg->running_ports->size()*2 );
	*events = (epoll_event*)malloc( sizeof(struct epoll_event) * pg->running_ports->size()*2 );

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
			epoll_ioscheduler::add_fd_epoll( &((*ev)[(i*2)+READ]), *epfd, port, fd);
		}
		/* Write */
		fd = (*pg->running_ports)[i]->get_write_fd();
		if( fd != -1 ){
			ROFL_DEBUG_VERBOSE("[epoll_ioscheduler] Adding TX event to epoll event list ioport: %s, fd: %d\n",port->of_port_state->name, fd);
			epoll_ioscheduler::add_fd_epoll(&((*ev)[(i*2)+WRITE]), *epfd, port, fd);
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


void* epoll_ioscheduler::process_io(void* grp){

	unsigned int i;
	int epfd, res;
	struct epoll_event *ev=NULL, *events = NULL;
	unsigned int current_hash=0, current_num_of_ports=0;
	portgroup_state* pg = (portgroup_state*)grp;
	//epoll_event_data_t* ev_port_data;
	ioport* port;
	bool more_packets;
	safevector<ioport*> ports;	//Ports of the group currently performing I/O operations
	
	//Init epoll fd set
	epfd = -1;
	init_or_update_fds(pg, ports, &epfd, &ev, &events, &current_num_of_ports, &current_hash );

	ROFL_DEBUG_VERBOSE("[epoll_ioscheduler] Initialization of epoll completed in thread:%d\n",pthread_self());
	
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
		
			do{
				more_packets = false;	

				for(i=0; i<current_num_of_ports; ++i){
					/* Read */
					port = ports[i];
	
					more_packets |= epoll_ioscheduler::process_port_rx(port);
					more_packets |= epoll_ioscheduler::process_port_tx(port);
				}
				
				//Check for updates in the running ports list
				//otherwise we will never stop looping hence not attending new ports 
				if( unlikely(pg->running_hash != current_hash) ){
					break;
				}

			//Make sure we check for keep_on_working flag, otherwise we will never
			//be able to stop the I/O in high load situations 
			}while(likely(more_packets) && likely(iomanager::keep_on_working(pg)) );
		}

		//Check for updates in the running ports 
		if( pg->running_hash != current_hash )
			init_or_update_fds(pg, ports, &epfd, &ev, &events, &current_num_of_ports, &current_hash );
	}

	//Release resources
	release_resources(epfd, ev, events, current_num_of_ports);

	ROFL_DEBUG("Finishing execution of I/O thread: #%u\n",pthread_self());

	//Return whatever
	pthread_exit(NULL);
}
