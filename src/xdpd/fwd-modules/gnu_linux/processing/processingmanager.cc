#include "processingmanager.h"
#include <cstdlib>
#include <unistd.h>
#include <rofl/common/utils/c_logger.h>
#include "../io/bufferpool.h"
#include "../util/ringbuffer.h"
#include "../ls_internal_state.h"


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

	//Allocate
	group = new ls_processing_threads_state;	

	//Init values
	group->sw = ls;
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
			ROFL_ERR("Error creating processing pthread for groups %d\n",group->id);
		}
	}

	return ROFL_SUCCESS;
}

rofl_result_t processingmanager::stop_ls_workers(of_switch_t* ls){
	
	ls_processing_threads_state* group;
	std::map<of_switch_t*,ls_processing_threads_state*>::iterator it;
	
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
	datapacket_t* pkt;
	of_switch_t* sw;
	ringbuffer* sw_pkt_queue;
	ls_processing_threads_state* ps;
	
	//Init 	
	if(! ( ps = (ls_processing_threads_state*) state ) ) 
		return NULL;
	if(! ( sw = ps->sw ) ) 
		return NULL;
	if(! ( sw_pkt_queue = (ringbuffer*) ((struct logical_switch_internals*) sw->platform_state )->ringbuffer ) )
		return NULL;

	//Main processing loop
	while(ps->keep_on_working){

		//Get a packet to process
		pkt = sw_pkt_queue->blocking_read(PROCESSING_THREADS_TIMEOUT_S_READ);

		if(!pkt)
			continue;
#ifdef DEBUG
		if(by_pass_pipeline){
			//DEBUG; by-pass pipeline, print trace and sleep
			std::cout <<"!";
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
		pkt = sw_pkt_queue->non_blocking_read(); //FIXME: put timeout
		
		if(!pkt)
			break; 
		
		//Drop it (released). The switch is down!
		bufferpool::release_buffer(pkt);	
	}

	//Printing some information
	ROFL_INFO("Finishing execution of processing thread: #%u switch: %s\n",pthread_self(),sw->name);
	
	//Exit
	pthread_exit(NULL);	
}

//Wrappers
rofl_result_t start_ls_workers_wrapper(of_switch_t* ls){
	return processingmanager::start_ls_workers(ls);
}
rofl_result_t stop_ls_workers_wrapper(of_switch_t* ls){
	return processingmanager::stop_ls_workers(ls);
}


