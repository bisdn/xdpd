#include <stdio.h>
#include <assert.h>
#include <sstream>
#include <rofl/common/utils/c_logger.h>
#include "iomanager.h"
#include "bufferpool.h"

//Add it here if you want to use another scheduler...
#include "scheduler/epoll_ioscheduler.h"
#include "scheduler/polling_ioscheduler.h"

using namespace xdpd::gnu_linux;

/* Static members */
unsigned int iomanager::num_of_groups = 0;
unsigned int iomanager::num_of_rx_groups = 0;
unsigned int iomanager::num_of_tx_groups = 0;
unsigned int iomanager::next_rx_sched_grp_id = 0;
unsigned int iomanager::next_tx_sched_grp_id = 0;

pthread_mutex_t iomanager::mutex = PTHREAD_MUTEX_INITIALIZER; 
std::map<uint16_t, portgroup_state*> iomanager::portgroups; 

//Change this if you want to use another scheduler
#ifdef IO_POLLING_STRATEGY
	typedef polling_ioscheduler ioscheduler_provider;
#else
	typedef epoll_ioscheduler ioscheduler_provider;
#endif


/*
** PUBLIC APIs 
**
**/
rofl_result_t iomanager::init( unsigned int _rx_groups, unsigned int _tx_groups){
 
	unsigned int i;

	pthread_mutex_lock(&mutex);
		
	//Check if it has been already inited before
	if(num_of_groups != 0){
		goto INIT_ERROR;
	}

	if( unlikely(_rx_groups<0) || unlikely(_tx_groups<0) ){
		ROFL_ERR(DRIVER_NAME"[iomanager] Invalid RX(%u) or TX(%u) paramenters. Aborting...\n", _rx_groups, _tx_groups); 
		goto INIT_ERROR;
	}
		
	//Initialize the number of groups
	num_of_rx_groups = _rx_groups;
	num_of_tx_groups = _tx_groups;

	ROFL_DEBUG(DRIVER_NAME"[iomanager] Initializing iomanager with %u RX portgroups (%u total threads), and %u TX portgroups (%u total threads)\n", _rx_groups, _rx_groups*DEFAULT_THREADS_PER_PORTGROUP, _tx_groups, _tx_groups*DEFAULT_THREADS_PER_PORTGROUP);
	
	try{	
		for(i=0;i<num_of_rx_groups;++i){
			//Create the group
			if( create_group(PG_RX, DEFAULT_THREADS_PER_PG, true) < 0 ){
				goto INIT_ERROR;
			}
		}	

		for(i=0;i<num_of_tx_groups;++i){
			//Create the group
			if( create_group(PG_TX, DEFAULT_THREADS_PER_PG, true) < 0 ){
				goto INIT_ERROR;
			}
		}	
	}catch(...){
		ROFL_ERR(DRIVER_NAME"[iomanager] Unable to intialize port groups. Out of memory?\n");
		goto INIT_ERROR;
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

	//Determine RX group
	grp_id = rx_sched(); 
	
	pthread_mutex_unlock(&mutex);
	
	ROFL_DEBUG(DRIVER_NAME"[iomanager] Adding port %s to iomanager, at portgroup RX %u\n", port->of_port_state->name, grp_id); 
	
	if(add_port_to_group(grp_id, port) != ROFL_SUCCESS){
		ROFL_ERR(DRIVER_NAME"[iomanager] Adding port %s to iomanager (RX), at portgroup %u FAILED\n", port->of_port_state->name, grp_id); 
		assert(0);
		//FIXME remove TX
		return ROFL_FAILURE;	
	}
	
	pthread_mutex_lock(&mutex);

	//Determine TX group
	grp_id = tx_sched(); 
	
	pthread_mutex_unlock(&mutex);
	
	ROFL_DEBUG(DRIVER_NAME"[iomanager] Adding port %s to iomanager, at portgroup TX %u\n", port->of_port_state->name, grp_id); 
	 	
	if(add_port_to_group(grp_id, port) != ROFL_SUCCESS){
		ROFL_ERR(DRIVER_NAME"[iomanager] Adding port %s to iomanager (TX), at portgroup %u FAILED\n", port->of_port_state->name, grp_id); 
		assert(0);
		return ROFL_FAILURE;	
	}
	
	return ROFL_SUCCESS;	
}

rofl_result_t iomanager::remove_port(ioport* port){
	
	int grp_id;

	ROFL_DEBUG(DRIVER_NAME"[iomanager] Removing port %s from iomanager\n", port->of_port_state->name); 
	
	grp_id = get_group_id_by_port(port, PG_RX);
	
	if(grp_id < 0){
		assert(0);
		return ROFL_FAILURE;
	}

	if(remove_port_from_group(grp_id, port) != ROFL_SUCCESS){
		ROFL_ERR(DRIVER_NAME"[iomanager] Removal of port %s from iomanager (RX), at portgroup %u FAILED!\n", port->of_port_state->name, grp_id); 
		assert(0);
		return ROFL_FAILURE;
	}

	grp_id = get_group_id_by_port(port, PG_TX);
	
	if(grp_id < 0){
		assert(0);
		return ROFL_FAILURE;
	}

	if(remove_port_from_group(grp_id, port) != ROFL_SUCCESS){
		ROFL_ERR(DRIVER_NAME"[iomanager] Removal of port %s from iomanager (TX), at portgroup %u FAILED!\n", port->of_port_state->name, grp_id); 
		assert(0);
		return ROFL_FAILURE;
	}

	return ROFL_SUCCESS;
}

/*
* Stop a particular port (regardless of the group which is belonging) 
*/
rofl_result_t iomanager::bring_port_down(ioport* port, bool mutex_locked){

	unsigned int i,j;
	bool brought_rx_down = false, brought_tx_down = false;

	dump_state(mutex_locked);

	if(!mutex_locked)
		pthread_mutex_lock(&mutex);
	
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

				//Change flags
				if(pg->type == PG_RX)
					brought_rx_down = true;
				else
					brought_tx_down = true;
	
				if( brought_rx_down && brought_tx_down ){
					
					//Now that no more packets are feeded, bring it really down	
					port->down();
					
					if(!mutex_locked)
						pthread_mutex_unlock(&mutex);
					
					dump_state(mutex_locked);
				
					return ROFL_SUCCESS;
				}
			}
			
		}	
	}catch(...){
		//Do nothing; should never jump here
		ROFL_ERR(DRIVER_NAME"[iomanager] Exception thrown while trying to bring %s port down\n", port->of_port_state->name);
		assert(0);
		
	}
	
	//Was not found (not scheduled), simply set the state
	port->of_port_state->up = false;
	
	if(!mutex_locked)
		pthread_mutex_unlock(&mutex);
		
	return ROFL_FAILURE;
}
/*
* Start a particular port. The port must be already contained in one of the portgroups. If portgroup is not running, pg threads are going to be started.
*/
rofl_result_t iomanager::bring_port_up(ioport* port){

	unsigned int i,j;
	bool brought_rx_up = false, brought_tx_up = false;
	
	dump_state(false);

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
					return ROFL_SUCCESS;
				}
			
				//Bring up port
				//Note: this MUST be done *before* port is actually added to the group
				//Otherwise structures may not be created
				if(brought_rx_up == false && brought_tx_up == false)
					port->up();		
			
				//Check if portgroup I/O threads are running
				if( pg->running_ports->size() == 0 ){
					pg->running_ports->push_back(port);
					start_portgroup_threads(pg);
	
					//Wait for all I/O threads to be synchronized with the new state (make sure internal state can be modified). Note that no one else can change PG state meanwhile
					for(j=0;j<pg->num_of_threads;++j)
						sem_wait(&pg->sync_sem);
				}else{
					pg->running_ports->push_back(port);
					
					//Refresh hash
					pg->running_hash++;
	
					//Wait for all I/O threads to be synchronized with the new state (make sure internal state can be modified). Note that no one else can change PG state meanwhile
					for(j=0;j<pg->num_of_threads;++j)
						sem_wait(&pg->sync_sem);
				}
				
				//Change flags
				if(pg->type == PG_RX)
					brought_rx_up = true;
				else
					brought_tx_up = true;
	
				if( brought_rx_up && brought_tx_up ){
					pthread_mutex_unlock(&mutex);
					dump_state(false);
					return ROFL_SUCCESS;
				}
			}
		}	
	}catch(...){
		//Do nothing; should never jump here
		ROFL_ERR(DRIVER_NAME"[iomanager] Exception thrown while trying to bring %s port up\n", port->of_port_state->name);
		assert(0);
	}

	//Was not found (not scheduled), simply set the state
	port->of_port_state->up = true;
	
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
	void* (*func)(void*);

	if(pg->type == PG_RX)
		func = ioscheduler_provider::process_io<true>;
	else
		func = ioscheduler_provider::process_io<false>;

	pg->keep_on = true;
 
	//Create num_of_threads and invoke scheduler::process_io
	for(i=0;i<pg->num_of_threads;++i){
		if(pthread_create(&pg->thread_state[i], NULL, func, (void *)pg) < 0){
			//TODO: print a trace or something
			ROFL_WARN(DRIVER_NAME" WARNING: pthread_create failed for port-group %d\n", pg->id);
		}
	}
}

/*
* Stops threads launched by the iomanager.
*/
void iomanager::stop_portgroup_threads(portgroup_state* pg){

	unsigned int i;

	pg->keep_on = false;
	
	//Join all threads
	for(i=0;i<pg->num_of_threads;++i){
		pthread_join(pg->thread_state[i],NULL);
	}
}

/*
* Creates an empty portgroup structure
*/
int iomanager::create_group(pg_type_t type, unsigned int num_of_threads, bool mutex_locked){

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
	pg->type = type;
	sem_init(&pg->sync_sem,0,0); //Init to 0

	if(!mutex_locked){
		pthread_mutex_lock(&mutex);
	}

	//Add to portgroups and return position in the vector
	pg->id = num_of_groups;
	portgroups[pg->id] = pg;
	
	num_of_groups++;
	
	if(!mutex_locked){
		pthread_mutex_unlock(&mutex);
	}

	
	ROFL_DEBUG(DRIVER_NAME"[iomanager] Created %s portgroup with %u thread(s) and id: %u\n", (type==PG_TX)? "TX": "RX", num_of_threads, pg->id); 

	//Return group_id
	return pg->id;	
}

/* Deletes the portgroup. If there are existing ports, they are stopped and deleted */
rofl_result_t iomanager::delete_all_groups(){

	unsigned int i;
	portgroup_state* pg;

	try{	
		for(i=0;i<num_of_rx_groups;++i){
			pg = portgroups[i];
			if(iomanager::delete_group(pg->id) != ROFL_SUCCESS){
				assert(0);
			}
		}	

		for(i=0;i<num_of_tx_groups;++i){
			pg = portgroups[i+num_of_rx_groups];
			if(iomanager::delete_group(pg->id) != ROFL_SUCCESS){
				assert(0);
			}
		}	
	}catch(...){
		ROFL_ERR(DRIVER_NAME"[iomanager] Warning: Unable to destroy some port groups\n");
	}

	return ROFL_SUCCESS;
}

/* Deletes the portgroup. If there are existing ports, they are stopped and deleted */
rofl_result_t iomanager::delete_group(unsigned int grp_id){

	portgroup_state* pg;

	//Ensure serialization
	pthread_mutex_lock(&mutex);

	//Make sure this portgroup exists
	pg = portgroups[grp_id]; 
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
int iomanager::get_group_id_by_port(ioport* port, pg_type_t type){
	
	unsigned int i,j;
	
	for(i=0;i<portgroups.size();++i){

		if(!portgroups[i])
			continue;

		if(portgroups[i]->type != type)
			continue;

		safevector<ioport*>* ports = portgroups[i]->ports;
			
		for(j=0;j<ports->size();j++){
			if( (*ports)[j] == port)
				return portgroups[i]->id;
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
	pg = portgroups[grp_id]; 
	if(!pg){
		pthread_mutex_unlock(&mutex);
		assert(0);
		return ROFL_FAILURE;
	}

	//Check existance of port already in any portgroup
	for(i=0;i<portgroups.size();++i){
		if(!portgroups[i])
			continue;
		if(portgroups[i]->ports->contains(port) && portgroups[i]->type == pg->type){
			pthread_mutex_unlock(&mutex);
			assert(0);
			return ROFL_FAILURE;
		}
	}

	//Add to port
	pg->ports->push_back(port);	

	pthread_mutex_unlock(&mutex);

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
	pg = portgroups[grp_id];
	if(!pg){
		if(!mutex_locked){
			pthread_mutex_unlock(&mutex);
		}
		assert(0);
		return ROFL_FAILURE;
	}

	//Check if port is in the portgroup
	if(!pg->ports->contains(port)){
		if(!mutex_locked){
			pthread_mutex_unlock(&mutex);
		}
		assert(0);
		return ROFL_FAILURE;
	}
	
	//Bring it down
	bring_port_down(port, true);
	
	//Erase it
	pg->ports->erase(port);	
	
	if(!mutex_locked){
		pthread_mutex_unlock(&mutex);
	}

	return ROFL_SUCCESS;
}



void iomanager::dump_state(bool mutex_locked){

#ifdef DEBUG
	unsigned int i;
	std::stringstream s("");	
	
	if(!mutex_locked)
		pthread_mutex_lock(&mutex);

	//Go through the groups and find the portgroup
	for(i=0;i<portgroups.size();++i){
	
		portgroup_state* pg = portgroups[i];

		if(!pg)
			continue;
	
		s<<"\t\t\t["<<pg->id<<"):";
		if (pg->type == PG_RX)
			s << "rx { ";
		else
			s << "tx { ";

		for(unsigned int j=0;j<pg->ports->size();++j){
			ioport* port = (*pg->ports)[j];
			s << port->of_port_state->name;
			if(pg->running_ports->contains(port))
				s << "(up) ";
			else
				s << "(down) ";	
		}
		s << "}]\n";
	}
	ROFL_DEBUG(DRIVER_NAME"[iomanager] status:\n%s", s.str().c_str());
	
	if(!mutex_locked)
		pthread_mutex_unlock(&mutex);
#endif
}
