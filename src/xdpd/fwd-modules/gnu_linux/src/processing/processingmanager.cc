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
#include "../io/iomanager.h"
#include "../util/time_measurements.h"

using namespace xdpd::gnu_linux;

/* Static member initialization */
pthread_mutex_t processingmanager::mutex = PTHREAD_MUTEX_INITIALIZER; 

rofl_result_t processingmanager::create_rx_pgs(of_switch_t* sw){ 

	unsigned int i;
	int pg_index;
	switch_platform_state_t* state;

	if(unlikely(!sw) || unlikely(!sw->platform_state)){
		goto create_error;
	}
	
	state = (switch_platform_state_t*)sw->platform_state;

	//Do it in mutual exclusion
	pthread_mutex_lock(&mutex);
	
	for(i=0;i<IO_RX_THREADS_PER_LSI;++i){
		//Create group
		pg_index = iomanager::create_group(PG_RX);
		
		if(unlikely(pg_index == -1))
			goto create_error_unlock;
		
		state->pg_index[i] = pg_index;
	}
	
	pthread_mutex_unlock(&mutex);
		
	return ROFL_SUCCESS;

create_error_unlock:
	pthread_mutex_unlock(&mutex);
create_error:
	assert(0);
	return ROFL_FAILURE;
} 

rofl_result_t processingmanager::destroy_rx_pgs(of_switch_t* sw){ 

	unsigned int i;
	switch_platform_state_t* state;

	if(unlikely(!sw) || unlikely(!sw->platform_state)){
		goto destroy_error;
	}
	
	state = (switch_platform_state_t*)sw->platform_state;

	for(i=0;i<IO_RX_THREADS_PER_LSI;++i){
		//Create group
		if(iomanager::delete_group(state->pg_index[i]) != ROFL_SUCCESS)
			goto destroy_error;
	}
	
	return ROFL_SUCCESS;

destroy_error:
	assert(0);
	return ROFL_FAILURE;
} 

int processingmanager::get_rx_pg_index_rr(of_switch_t* sw, ioport* port){

	unsigned int i;
	switch_platform_state_t* state;

	if(unlikely(!sw) || unlikely(!sw->platform_state)){
		assert(0);
		return -1;	
	}
	
	state = (switch_platform_state_t*)sw->platform_state;

	//Do it in mutual exclusion
	pthread_mutex_lock(&mutex);
	i = (state->curr +1) % IO_RX_THREADS_PER_LSI; //RR
	state->curr = i;
	pthread_mutex_unlock(&mutex);

	return state->pg_index[i];
} 
