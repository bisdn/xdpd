/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PROCESSINGMANAGER_H
#define PROCESSINGMANAGER_H 

#include <pthread.h>
#include <vector>
#include <map>
#include <rofl.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include "../util/safevector.h" 
#include "../io/ports/ioport.h"
#include "ls_internal_state.h"
#include "../config.h" 

/**
* @file processingmanager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief In charge of processing (Openflow pipeline)
* threads; e.g. launching and stopping them.
* 
*/

namespace xdpd {
namespace gnu_linux {

/**
* @brief Switch platform state (pipeline of1x_switch_t platform state)
*
* @ingroup fm_gnu_linux_processing
*/
class ls_processing_threads_state {

public:
	//Switch to be processed
	of_switch_t* sw;

	//Threading information
	unsigned int num_of_threads;
	pthread_t thread_state[PROCESSING_MAX_LSI_THREADS];

	//Input queues sched index
	unsigned int current_in_queue_index;
	
	//State of the threads
	bool keep_on_working;	
};

/**
* @brief Processing manager. Creates/destroys the processing threads associated with a logical switch instance.
*
* @ingroup fm_gnu_linux_processing
*/
class processingmanager{ 

public:
	//Default number of threads per logical switch
	static const unsigned int DEFAULT_THREADS_PER_LS = PROCESSING_THREADS_PER_LSI;
	static const unsigned int MAX_THREADS_PER_LS = PROCESSING_MAX_LSI_THREADS;
	
	/*
	* start_ls_workers start N processing threads to work on processing packets in the logical switch (ls). Default is DEFAULT_THREADS_PER_LS, and default processing routine is pprocessingmanager::process_packets_through_pipeline
	*/
	static rofl_result_t start_ls_workers(of_switch_t* ls, unsigned int num_of_threads=DEFAULT_THREADS_PER_LS, void* (*processing_function)(void*) = processingmanager::process_packets_through_pipeline);
 
	/*
	* This might take a while.
	*/	
	static rofl_result_t stop_ls_workers(of_switch_t* ls);	

	/**
	* Binds a port to a processing queue of the switch (which it is already attached to)
	*/
	static rofl_result_t bind_port_to_sw_processing_queue(ioport* port);	
	
	/**
	* Unbinds the port from the processing queue of the switch
	*/
	static rofl_result_t unbind_port_from_sw_processing_queue(ioport* port);
		
#if DEBUG 
	/* 
	* By passing the pipeline for testing 
	*/
	
	static bool by_pass_pipeline;
#endif

	//Timout for the processing threads (to be able to shutdown switch)	
	static unsigned int PROCESSING_THREADS_TIMEOUT_S_READ;

protected:

	//LS processing threads state db
	static std::map<of_switch_t*,ls_processing_threads_state*> ls_processing_groups;
	
	//Handle mutual exclusion over the ls_processing_groups 
	static pthread_mutex_t mutex;

	/* Default processing of packets routine */
	static void* process_packets_through_pipeline(void* processing_threads_state);
};


}// namespace xdpd::gnu_linux 
}// namespace xdpd

//Wrappers
ROFL_BEGIN_DECLS

typedef struct xdpd::gnu_linux::processingmanager processingmanager_t;
rofl_result_t start_ls_workers_wrapper(of_switch_t* ls);
rofl_result_t stop_ls_workers_wrapper(of_switch_t* ls);

ROFL_END_DECLS



#endif /* PROCESSINGMANAGER_H_ */
