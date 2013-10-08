#include "polling_ioscheduler.h"

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

using namespace xdpd::gnu_linux;

//Static members initialization
const float polling_ioscheduler::WRITE_QOS_QUEUE_FACTOR[ioport::MAX_OUTPUT_QUEUES]={1,1.2,1.5,2,2.2,2.5,2.7,3.0};

#ifdef DEBUG
bool polling_ioscheduler::by_pass_processing = false;
#endif

/*
* Call port based on scheduling algorithm 
*/
inline void polling_ioscheduler::process_port_io(ioport* port){

	unsigned int i, q_id, n_buckets;
	datapacket_t* pkt;
	
	if(!port || !port->of_port_state)
		return;

	//Perform up_to n_buckets_read
	ROFL_DEBUG_VERBOSE("Trying to read at port %s with %d\n", port->of_port_state->name, READ_BUCKETS_PP);
	
	for(i=0; i< READ_BUCKETS_PP; ++i){
		
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
			ROFL_DEBUG_VERBOSE("[%s] reading finished at: %d/%d\n", port->of_port_state->name, i, READ_BUCKETS_PP);
			break;
		}
	}

	//Process output up to WRITE_BUCKETS_PP
	for(q_id=0; q_id < IO_IFACE_NUM_QUEUES; ++q_id){
		
		//Increment number of buckets
		n_buckets = WRITE_BUCKETS_PP*WRITE_QOS_QUEUE_FACTOR[q_id];

		ROFL_DEBUG_VERBOSE("[%s] Trying to write at port queue: %d with n_buckets: %d.\n", port->of_port_state->name, q_id, n_buckets);
		
		//Perform up to n_buckets write	
		port->write(q_id,n_buckets);
	}

}

/*
* Polling stuff 
*/
void polling_ioscheduler::update_running_ports(portgroup_state* pg, ioport*** running_ports, unsigned int* num_of_ports, unsigned int* current_hash){

	unsigned int i;

	//Free existing array of ioports
	if(*running_ports)
		free(*running_ports);

	//lock running vector, so that we can safely iterate over it
	pg->running_ports->read_lock();

	//Set the number of ports
	*num_of_ports = pg->running_ports->size();	

	//Allocate new memory
	*running_ports = (ioport**)malloc(sizeof(ioport*)*(*num_of_ports));

	//TODO: check error?
	//if(!running_ports)
	//	exit(EXIT_FAILURE);

	//Do the copy
	for(i=0; i<*num_of_ports; i++){
		(*running_ports)[i] = (*pg->running_ports)[i];		
	}
	
	//Assign current hash
	*current_hash = pg->running_hash;	

	//Signal as synchronized
	iomanager::signal_as_synchronized(pg);
	
	//unlock running vector
	pg->running_ports->read_unlock();
}

void* polling_ioscheduler::process_io(void* grp){

	unsigned int i, current_hash, num_of_ports;
	portgroup_state* pg = (portgroup_state*)grp;
	ioport** running_ports=NULL; //C-array of ioports
 
	//Update 
	update_running_ports(pg, &running_ports, &num_of_ports, &current_hash);	

	ROFL_DEBUG_VERBOSE("[polling_ioscheduler] Initialization of polling completed in thread:%d\n",pthread_self());
	
	/*
	* Infinite loop unless group is stopped. e.g. all ports detached
	*/
	while(iomanager::keep_on_working(pg)){
		
		//Loop over all running ports
		for(i = 0; i < num_of_ports ; i++){
			polling_ioscheduler::process_port_io(running_ports[i]);
		}	
		
		//Check for updates in the running ports 
		if( pg->running_hash != current_hash )
			update_running_ports(pg, &running_ports, &num_of_ports, &current_hash);	
	}

	ROFL_DEBUG("Finishing execution of I/O thread: #%u\n",pthread_self());

	//Return whatever
	pthread_exit(NULL);
}
