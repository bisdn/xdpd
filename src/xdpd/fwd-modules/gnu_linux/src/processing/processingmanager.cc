#include "processingmanager.h"
#include <assert.h>
#include <cstdlib>
#include <unistd.h>
#include <rofl/common/utils/c_logger.h>
#include "../io/bufferpool.h"
#include "../util/circular_queue.h"
#include "../util/likely.h"
#include "ls_internal_state.h"

//Profiling
#include "../util/time_measurements.h"

using namespace xdpd::gnu_linux;

/* Static member initialization */
pthread_mutex_t processingmanager::mutex = PTHREAD_MUTEX_INITIALIZER; 
std::map<of_switch_t*,ls_processing_threads_state*> processingmanager::ls_processing_groups;

#if DEBUG
bool processingmanager::by_pass_pipeline = false;
#endif
unsigned int processingmanager::PROCESSING_THREADS_TIMEOUT_S_READ=3;

//Starts
rofl_result_t processingmanager::start_ls_workers(of_switch_t* ls, unsigned int num_of_threads, void* (*processing_function)(void*)){

	unsigned int i;
	ls_processing_threads_state* group;
	std::map<of_switch_t*,ls_processing_threads_state*>::const_iterator it;

	ROFL_DEBUG("Creating #%u processing threads for logical switch %s (%p)\n", num_of_threads, ls->name, ls);
	
	//Allocate
	group = new ls_processing_threads_state;	

	//Init values
	group->sw = ls;
	group->current_in_queue_index = 0;
	group->num_of_threads = num_of_threads;
	group->keep_on_working = true;
	
	//Do it in mutual exclusion
	pthread_mutex_lock(&mutex);
	
	//Check if the switch is already there
	if ( (it = ls_processing_groups.find(ls)) != ls_processing_groups.end() ){
		pthread_mutex_unlock(&mutex);
		delete group;
		return ROFL_FAILURE;
	}
	
	//Add it to the group
	ls_processing_groups[ls] = group;

	pthread_mutex_unlock(&mutex);
		
	//Create num_of_threads and invoke scheduler::process_io
	for(i=0;i<num_of_threads;++i){
		if(pthread_create(&group->thread_state[i], NULL, processing_function, (void *)group) < 0){
			//TODO: print a trace or something
			ROFL_ERR("Error creating processing pthread for switch %p\n",ls);
		}
	}

	return ROFL_SUCCESS;
}

rofl_result_t processingmanager::stop_ls_workers(of_switch_t* ls){
	
	ls_processing_threads_state* group;
	std::map<of_switch_t*,ls_processing_threads_state*>::iterator it;
	
	ROFL_DEBUG("Stopping processing threads of logical switch %s (%p)\n", ls->name, ls);
	
	//Find group and extract the group from map 
	pthread_mutex_lock(&mutex);
	
	it = ls_processing_groups.find(ls);
	
	if( it != ls_processing_groups.end() ){
		//Extract from the map
		group = it->second;
		ls_processing_groups.erase(it);		
	}else{
		//Does not exist
		pthread_mutex_unlock(&mutex);
		return ROFL_FAILURE;
	}
	pthread_mutex_unlock(&mutex);

	//Signal threads to stop
	group->keep_on_working = false;
	
	//Join all threads	
	for(unsigned int i=0;i<group->num_of_threads;i++){
		pthread_join(group->thread_state[i],NULL);
	}
	
	//Delete
	delete group;
	
	return ROFL_SUCCESS;
}

void* processingmanager::process_packets_through_pipeline(void* state){
	unsigned int q_id;
	datapacket_t* pkt;
	of_switch_t* sw;
	circular_queue<datapacket_t, PROCESSING_INPUT_QUEUE_SLOTS>* sw_input_queue;
	ls_processing_threads_state* ps;
	
	//Init 	
	if(! ( ps = (ls_processing_threads_state*) state ) ){ 
		assert(0);
		return NULL;
	}
	if(! ( sw = ps->sw ) ){
		assert(0);
		return NULL;
	}
	//Determine the thread's q_id
	for(q_id=0; q_id<PROCESSING_MAX_LSI_THREADS; q_id++){
		if( ps->thread_state[q_id] == pthread_self() )
			break;
		if( (q_id+1) == PROCESSING_MAX_LSI_THREADS )
			assert(0); //Shall never happen
	}
	//Recover input queue	
	if(! ( sw_input_queue = ((struct logical_switch_internals*) sw->platform_state )->input_queues[q_id] ) ){
		assert(0);
		return NULL;
	}

	//Main processing loop
	while( likely(ps->keep_on_working == true) ){

		//Get a packet to process
		pkt = sw_input_queue->blocking_read(PROCESSING_THREADS_TIMEOUT_S_READ);

	
		if(unlikely(pkt == NULL))
			continue;

		TM_STAMP_STAGE(pkt, TM_S4);
	
#ifdef DEBUG
		if(by_pass_pipeline){
			//DEBUG; by-pass pipeline, print trace and sleep
			ROFL_DEBUG_VERBOSE("!");
			usleep(rand()%300); //Random sleep up to 300ms
		}else{
#endif
			//Process it through the pipeline	
			of_process_packet_pipeline(sw,pkt);
#ifdef DEBUG
		}
#endif

	}

	
	//Shutdown of the Logical switch. Clean up the remaining unprocessed packets if any (drop them) 
	for(;;){
	
		//Get buffered packet
		pkt = sw_input_queue->non_blocking_read(); //FIXME: put timeout
		
		if(!pkt)
			break; 
		
		//Drop it (released). The switch is down!
		bufferpool::release_buffer(pkt);	
	}

	//Printing some information
	ROFL_DEBUG("Finishing execution of processing thread: #%u switch: %s\n",pthread_self(),sw->name);
	
	//Exit
	pthread_exit(NULL);	
}

//Binds a port to a processing queue of the switch (which it is already attached to)
rofl_result_t processingmanager::bind_port_to_sw_processing_queue(ioport* port){

	unsigned int index;
	of_switch_t* sw = port->of_port_state->attached_sw;
	std::map<of_switch_t*,ls_processing_threads_state*>::const_iterator it;
	struct logical_switch_internals* ls_int;

	if(!sw){
		assert(0);
		return ROFL_FAILURE;
	}

	//Recover the state of the processing threads
	if ( (it = ls_processing_groups.find(sw)) == ls_processing_groups.end() ){
		assert(0);
		return ROFL_FAILURE;
	}
	
	//Recover platform state of the switch
 	ls_int = (struct logical_switch_internals*)sw->platform_state;

	//Calculate queue_id (sort of round-robin)
	pthread_mutex_lock(&mutex);

	index = it->second->current_in_queue_index;
	it->second->current_in_queue_index =  (it->second->current_in_queue_index+1) % PROCESSING_THREADS_PER_LSI;

	pthread_mutex_unlock(&mutex);

	//Really bind the port  to the queue	
	port->set_sw_processing_queue( ls_int->input_queues[index] );
	
	ROFL_DEBUG("Binding port %s to sw(%p) at queue: %u\n", port->of_port_state->name, sw, index);

	return ROFL_SUCCESS;
}	

//Unbinds the port from the processing queue of the switch
rofl_result_t processingmanager::unbind_port_from_sw_processing_queue(ioport* port){
	port->set_sw_processing_queue(NULL);
	return ROFL_SUCCESS;
}

//Wrappers
rofl_result_t start_ls_workers_wrapper(of_switch_t* ls){
	return processingmanager::start_ls_workers(ls);
}
rofl_result_t stop_ls_workers_wrapper(of_switch_t* ls){
	return processingmanager::stop_ls_workers(ls);
}


