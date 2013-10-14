/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOMANAGER_H
#define IOMANAGER_H 1

#include <pthread.h>
#include <vector>
#include <rofl.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <semaphore.h>
#include "../config.h"
#include "scheduler/ioscheduler.h"
#include "ports/ioport.h"
#include "../util/safevector.h" 
#include "../util/compiler_assert.h" 

/**
* @file iomanager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief This static class is in charge of of launch/stopping of the I/O threads, 
* dealing with data packet tx/rx.
*
* TODO: in depth explanation
* 
*/

namespace xdpd {
namespace gnu_linux {

#define DEFAULT_MAX_THREADS_PER_PG 5
//WARNING: you don't want to change this, unless you really know what you are doing
#define DEFAULT_THREADS_PER_PG 1
COMPILER_ASSERT( INVALID_default_threads_per_pg , (DEFAULT_THREADS_PER_PG == 1) );

/**
* @brief Portgroup thread state
*
* @ingroup fm_gnu_linux_io
*/
class portgroup_state {

public:
	//Group id
	unsigned int id;	

	//Threading information
	unsigned int num_of_threads;
	pthread_t thread_state[DEFAULT_MAX_THREADS_PER_PG];

	//State synchronization condition
	sem_t sync_sem;
	
	// I/O port information
	safevector<ioport*>* ports; 		//All ports in the group
	safevector<ioport*>* running_ports;	//Ports of the group currently performing I/O operations
	uint32_t running_hash;			//Pseudo-hash over the running_ports state
	
};

/**
* @brief I/O manager, creates and destroys (launches and stops) I/O threads to work on the ports, or specifically a set of ports (portgroups). 
* This class is purely static.
*
* @ingroup fm_gnu_linux_io
*/
class iomanager{ 

public:
	/* Methods */
	//Group mgmt
	static rofl_result_t init( unsigned int _num_of_groups = IO_TOTAL_THREADS );
	static rofl_result_t destroy( void ){ return delete_all_groups(); };

	/*
	* Port management (external interface)
	*/	
	static rofl_result_t add_port(ioport* port);
	static rofl_result_t remove_port(ioport* port);

	/*
	* Port control
	*/
	static rofl_result_t bring_port_up(ioport* port);
	static rofl_result_t bring_port_down(ioport* port, bool mutex_locked=false);
	
	/*
	* Checkpoint for I/O threads to keep on working. Called by schedulers
	*/ 
	inline static bool keep_on_working(portgroup_state* pg){ return portgroups[pg->id]->running_ports->size() > 0;};

	/*
	* Signal that PG state has been syncrhonized within a particular I/O thread 
	*/
	inline static void signal_as_synchronized(portgroup_state* pg){ sem_post(&pg->sync_sem); };

	/* Utils */ 
	static portgroup_state* get_group(int grp_id);	
	static int get_group_id_by_port(ioport* port);
protected:

	//Constants
	static const unsigned int DEFAULT_THREADS_PER_PORTGROUP = DEFAULT_THREADS_PER_PG;
	static const unsigned int MAX_THREADS_PER_PORTGROUP = DEFAULT_MAX_THREADS_PER_PG;

	//Number of port_groups created
	static unsigned int num_of_groups;
	static unsigned int curr_group_sched_pointer;

	//Number of buffers currently required by the ports to operate
	static unsigned long long int num_of_port_buffers;

	//Portgroup state
	//static std::vector<portgroup_state> portgroups;	
	static safevector<portgroup_state*> portgroups;	
	
	//Handle mutual exclusion over the portgroup state
	static pthread_mutex_t mutex;

	/*
	* Group mgmt (internal API)
	*/
	static int create_group(unsigned int num_of_threads=DEFAULT_THREADS_PER_PG, bool mutex_locked=false);
	static rofl_result_t delete_group(unsigned int grp_id);
	static rofl_result_t delete_all_groups(void);
	
	/*
	* Port mgmt (internal API)
	*/
	static rofl_result_t add_port_to_group(unsigned int grp_id, ioport* port);
	static rofl_result_t remove_port_from_group(unsigned int grp_id, ioport* port, bool mutex_locked=false);

	/* Start/Stop portgroup threads */
	static void start_portgroup_threads(portgroup_state* pg);
	static void stop_portgroup_threads(portgroup_state* pg);
	
};

}// namespace xdpd::gnu_linux 
}// namespace xdpd


#endif /* IOMANAGER_H_ */
