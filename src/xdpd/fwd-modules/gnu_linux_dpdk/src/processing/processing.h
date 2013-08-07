/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PROCESSING_H_
#define _PROCESSING_H_

#include "../config.h"

#include <rofl.h>
#include <rte_common.h> 
#include <rte_eal.h> 
#include <rte_log.h>
#include <rte_launch.h> 
#include <rte_mempool.h> 
#include <rte_mbuf.h> 
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_launch.h>

#include "../io/dpdk_datapacket.h"

#define MAX_PORTS_PER_CORE 64

/**
* Core task list
*/
typedef struct core_tasks{
	bool available;
	bool active;
	unsigned int num_of_ports;
	switch_port_t* port_list[MAX_PORTS_PER_CORE]; //active ports MUST be on the very beginning of the array, contiguously.
}core_tasks_t;

/**
* Initialize data structures for processing to work 
*/
rofl_result_t processing_init(void);

/**
* Destroy data structures for processing to work 
*/
rofl_result_t processing_destroy(void);

/**
* Schedule port to a core 
*/
rofl_result_t processing_schedule_port(switch_port_t* port);

/**
* Deschedule port to a core 
*/
rofl_result_t processing_deschedule_port(switch_port_t* port);


/**
* Packet processing routine for cores 
*/
int processing_core_process_packets(void*);

/**
* Dump core state
*/
void processing_dump_core_state(void);


#endif //_PROCESSING_H_
