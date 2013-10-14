#include <stdio.h>
#include <assert.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/platform/cutil.h>
#include "iomanager.h"
#include "bufferpool.h"
#include "../processing/processingmanager.h"

//Add it here if you want to use another scheduler...
#include "scheduler/epoll_ioscheduler.h"
#include "scheduler/polling_ioscheduler.h"

using namespace xdpd::gnu_linux;

/* Static members */
unsigned int iomanager::num_of_groups = 0;
unsigned int iomanager::curr_group_sched_pointer = 0;
unsigned long long int iomanager::num_of_port_buffers = 0;
pthread_mutex_t iomanager::mutex = PTHREAD_MUTEX_INITIALIZER; 
//std::vector<portgroup_state> iomanager::portgroups; //TODO: maybe add a pre-reserved memory here
safevector<portgroup_state*> iomanager::portgroups; //TODO: maybe add a pre-reserved memory here

//Change this if you want to use another scheduler
typedef epoll_ioscheduler ioscheduler_provider;
//typedef polling_ioscheduler ioscheduler_provider;



/*
** PUBLIC APIs 
**
**/
rofl_result_t iomanager::init( unsigned int _num_of_groups ){
	unsigned int i;

	pthread_mutex_lock(&mutex);
	
	if(! (_num_of_groups > 0) ){
		goto INIT_ERROR;
	}
		
	ROFL_DEBUG("Initializing iomanager with %u portgroups (%u total threads)\n", _num_of_groups, _num_of_groups*DEFAULT_THREADS_PER_PORTGROUP);
	
	//Check if it has been already inited before
	if(num_of_groups != 0){
		goto INIT_ERROR;
	}
	
	for(i=0;i<_num_of_groups;++i){
		//Create the group
		if( create_group(DEFAULT_THREADS_PER_PG, true) < 0 ){
			goto INIT_ERROR;
		}
	}	

	pthread_mutex_unlock(&mutex);
	return ROFL_SUCCESS;

INIT_ERROR:
	assert(0);
	pthread_mutex_unlock(&mutex);
	return ROFL_FAILURE;
}

/*
* Port management (external interface)
*/	
rofl_result_t iomanager::add_port(ioport* port){

	int grp_id;

	pthread_mutex_lock(&mutex);
	grp_id = curr_group_sched_pointer;
	curr_group_sched_pointer = (curr_group_sched_pointer+1) % num_of_groups;
	pthread_mutex_unlock(&mutex);
	
	ROFL_DEBUG("Adding port %s to iomanager, at portgroup %u\n", port->of_port_state->name, grp_id); 
	 	
	return add_port_to_group(grp_id, port);	
}

rofl_result_t iomanager::remove_port(ioport* port){
	
	int grp_id;

	ROFL_DEBUG("Removing port %s from iomanager\n", port->of_port_state->name); 
	
	grp_id = get_group_id_by_port(port);
	
	if(grp_id < 0){
		assert(0);
		return ROFL_FAILURE;
	}

	return remove_port_from_group(grp_id, port);	
}

/*
* Stop a particular port (regardless of the group which is belonging) 
*/
rofl_result_t iomanager::bring_port_down(ioport* port, bool mutex_locked){

	unsigned int i,j;

	if(!mutex_locked){
		pthread_mutex_lock(&mutex);
	}
	
	try{
		//Go through the groups and find the portgroup
		for(i=0;i<portgroups.size();i++){
		
			portgroup_state* pg = portgroups[i];

			//Delete from running group if is contained 
			if(pg->running_ports->contains(port)){	
			
				//Delete it from running list
				pg->running_ports->erase(port);
				
				//Refresh hash
				pg->running_hash++;
			
				//If there are no more ports, stop threads 
				if(pg->running_ports->size() == 0){
					stop_portgroup_threads(pg);
				}else{
					//Wait for all I/O threads to be synchronized with the new state (make sure internal state can be modified). Note that no one else can change PG state meanwhile
					for(j=0;j<pg->num_of_threads;++j){
						sem_wait(&pg->sync_sem);
					}
				}
				//Call ioport hook for down
				port->disable();
	
				if(!mutex_locked){
					pthread_mutex_unlock(&mutex);
				}
				return ROFL_SUCCESS; 
			}
			
		}	
	}catch(...){
		//Do nothing; should never jump here
		ROFL_ERR("Exception thrown while trying to bring %s port down\n", port->of_port_state->name);
		assert(0);
		
	}
	
	if(!mutex_locked){
		pthread_mutex_unlock(&mutex);
	}
		
	return ROFL_FAILURE;
}
/*
* Start a particular port. The port must be already contained in one of the portgroups. If portgroup is not running, pg threads are going to be started.
*/
rofl_result_t iomanager::bring_port_up(ioport* port){

	unsigned int i,j;

	//Go through the groups and find the portgroup	
	pthread_mutex_lock(&mutex);
	
	try{
		//Go through the groups and find the portgroup
		for(i=0;i<portgroups.size();++i){
		
			portgroup_state* pg = portgroups[i];

			//Check if is contained
			if(pg->ports->contains(port)){	
				
				if(pg->running_ports->contains(port)){
					//Port is already UP!
					pthread_mutex_unlock(&mutex);
					return ROFL_FAILURE;
				}
					
				//Call ioport hook for down
				port->enable();		
				
				//Check if portgroup I/O threads are running
				if( pg->running_ports->size() == 0 ){
					
					
					pg->running_ports->push_back(port);
					start_portgroup_threads(pg);
	
					//Wait for all I/O threads to be synchronized with the new state (make sure internal state can be modified). Note that no one else can change PG state meanwhile
					for(j=0;j<pg->num_of_threads;++j)
						sem_wait(&pg->sync_sem);
					
					pthread_mutex_unlock(&mutex);
					return ROFL_SUCCESS;

				}else{
					pg->running_ports->push_back(port);
					
					//Refresh hash
					pg->running_hash++;
	
					//Wait for all I/O threads to be synchronized with the new state (make sure internal state can be modified). Note that no one else can change PG state meanwhile
					for(j=0;j<pg->num_of_threads;++j)
						sem_wait(&pg->sync_sem);
			
					pthread_mutex_unlock(&mutex);
					return ROFL_SUCCESS;

				}
			}	
		}	
	}catch(...){
		//Do nothing; should never jump here
		ROFL_ERR("Exception thrown while trying to bring %s port up\n", port->of_port_state->name);
		assert(0);
	}
	
	pthread_mutex_unlock(&mutex);
	return ROFL_FAILURE;
}






/*
** PRIVATE APIs
** 
**/

/*
* Starts num_of_threads as per defined in the portgroup
*/
void iomanager::start_portgroup_threads(portgroup_state* pg){

	unsigned int i;

	//Create num_of_threads and invoke scheduler::process_io
	for(i=0;i<pg->num_of_threads;++i){
		if(pthread_create(&pg->thread_state[i], NULL, ioscheduler_provider::process_io, (void *)pg) < 0){
			//TODO: print a trace or something
			ROFL_WARN("WARNING: pthread_create failed for port-group %d\n", pg->id);
		}
	}
}

/*
* Stops threads launched by the iomanager.
*/
void iomanager::stop_portgroup_threads(portgroup_state* pg){

	unsigned int i;

	//Join all threads
	for(i=0;i<pg->num_of_threads;++i){
		pthread_join(pg->thread_state[i],NULL);
	}
}

/*
* Checks whether it exists a portgroup with grp_id. This method is NOT thread-safe 
*/
portgroup_state* iomanager::get_group(int grp_id){

	portgroup_state* pg;

	try{
		pg = portgroups[grp_id];
		return pg;
	}catch(...){
		return NULL; 
	}
}

/*
* Creates an empty portgroup structure
*/
int iomanager::create_group(unsigned int num_of_threads, bool mutex_locked){

	portgroup_state* pg;

	//Check if we are over the max. num of threads	
	if(num_of_threads > MAX_THREADS_PER_PORTGROUP)
		return -1;

	pg = new portgroup_state;
	
	//Init struct
	pg->num_of_threads = num_of_threads;
	pg->running_hash = 0;
	pg->ports = new safevector<ioport*>();	
	pg->running_ports = new safevector<ioport*>();	
	sem_init(&pg->sync_sem,0,0); //Init to 0

	if(!mutex_locked){
		pthread_mutex_lock(&mutex);
	}

	//Add to portgroups and return position in the vector
	pg->id = portgroups.size();
	portgroups.push_back(pg);
	
	num_of_groups++;
	
	if(!mutex_locked){
		pthread_mutex_unlock(&mutex);
	}


	//Return group_id
	return pg->id;	
}

/* Deletes the portgroup. If there are existing ports, they are stopped and deleted */
rofl_result_t iomanager::delete_all_groups(){

	while( iomanager::delete_group(0) != ROFL_FAILURE);
	return ROFL_SUCCESS;
}

/* Deletes the portgroup. If there are existing ports, they are stopped and deleted */
rofl_result_t iomanager::delete_group(unsigned int grp_id){

	portgroup_state* pg;

	//Ensure serialization
	pthread_mutex_lock(&mutex);

	//Make sure this portgroup exists
	pg = get_group(grp_id); 
	if(!pg){
		pthread_mutex_unlock(&mutex);
		return ROFL_FAILURE;
	}

	//First delete all the ports
	while(1){
		try{
			if( remove_port_from_group(grp_id,(*pg->ports)[0],true) != ROFL_SUCCESS){
				//TODO: trace error?
			}
		}catch(...){
			//No more ports
			break;
		}
	}

	//Delete it from the portgroups list 
	portgroups.erase(grp_id);

	//Free memory 
	delete pg->ports;
	delete pg->running_ports;
	delete pg;	
	
	num_of_groups--;
	
	pthread_mutex_unlock(&mutex);
	
	return ROFL_SUCCESS;
}

/*
* Returns the id of the portgroup in which the port is currently contained. iomanager:ROFL_FAILURE (-1) if not found. 
*/
int iomanager::get_group_id_by_port(ioport* port){
	
	unsigned int i,j;
	
	for(i=0;i<portgroups.size();++i){
		safevector<ioport*>* ports = portgroups[i]->ports;
			
		for(j=0;j<ports->size();j++){
			if( (*ports)[j] == port )	
				return i;
		}
	}
	
	return -1; 
}


/*
* Adds port to the portgroup. Addition of the port does NOT bring it up.
*/
rofl_result_t iomanager::add_port_to_group(unsigned int grp_id, ioport* port){

	unsigned int i;	
	portgroup_state* pg;

	//Ensure serialization
	pthread_mutex_lock(&mutex);

	//Make sure this portgroup exists
	pg = get_group(grp_id); 
	if(!pg){
		pthread_mutex_unlock(&mutex);
		return ROFL_FAILURE;
	}

	//Check existance of port already in any portgroup
	for(i=0;i<portgroups.size();++i){
		if(portgroups[i]->ports->contains(port)){
			pthread_mutex_unlock(&mutex);
			return ROFL_FAILURE;
		}
	}

	//Pre-allocate buffers for operating at line-rate
	num_of_port_buffers += port->get_required_buffers();
	bufferpool::increase_capacity(num_of_port_buffers);
	
	//Add to port
	pg->ports->push_back(port);	

	pthread_mutex_unlock(&mutex);
	
	//Bind port to switch queue
	if( processingmanager::bind_port_to_sw_processing_queue(port) != ROFL_SUCCESS){
		remove_port_from_group(grp_id, port);
		return ROFL_FAILURE;
	}
	
	return ROFL_SUCCESS; 
}

/*
* Remove port to the portgroup. Removal implicitely stops the port (brings it down).
*/
rofl_result_t iomanager::remove_port_from_group(unsigned int grp_id, ioport* port, bool mutex_locked){
	
	portgroup_state* pg;

	//Ensure serialization
	if(!mutex_locked){
		pthread_mutex_lock(&mutex);
	}

	//Make sure this portgroup exists
	pg = get_group(grp_id); 
	if(!pg){
		if(!mutex_locked){
			pthread_mutex_unlock(&mutex);
		}
		return ROFL_FAILURE;
	}

	//Check if port is in the portgroup
	if(!pg->ports->contains(port)){
		if(!mutex_locked){
			pthread_mutex_unlock(&mutex);
		}
		return ROFL_FAILURE;
	}
	
	//Bring it down
	bring_port_down(port, true);
	
	//Substract buffers that were required by this port
	num_of_port_buffers -= port->get_required_buffers();
		
	//Erase it
	pg->ports->erase(port);	
	
	if(!mutex_locked){
		pthread_mutex_unlock(&mutex);
	}
	
	//Unbind port to switch queue
	processingmanager::unbind_port_from_sw_processing_queue(port);

	return ROFL_SUCCESS;
}


