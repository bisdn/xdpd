/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_H_
#define _NETFPGA_H_

#include <inttypes.h>
#include <stdbool.h>
#include <pthread.h>
#include <rofl.h>
#include "stats.h"
#include "ports.h"
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>
#include <rofl/datapath/pipeline/physical_switch.h>
//#include "flow_entry.h"

/**
* @file netfga.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NetFPGA basic abstractions 
*/

//Constants
#define NETFPGA_RESERVED_FOR_CPU2NETFPGA		8
#define NETFPGA_OPENFLOW_EXACT_TABLE_SIZE		1024 
#define NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE	32
#define NETFPGA_OPENFLOW_WILDCARD_TABLE_SIZE	(NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE - NETFPGA_RESERVED_FOR_CPU2NETFPGA)

//Ports
#define NETFPGA_FIRST_PORT	1
#define NETFPGA_LAST_PORT	4
#define NETFPGA_PORT_BASE	1
#define NETFPGA_IN_PORT		0xfff8
#define NETFPGA_FLOOD_PORT	0xfffd
#define NETFPGA_ALL_PORTS	0xfffc

//Polynomials
#define NETFPGA_POLYNOMIAL1	0x04C11DB7
#define NETFPGA_POLYNOMIAL2	0x1EDC6F41 

struct netfpga_flow_entry;

typedef struct netfpga_device{

	//File descriptor for IOCTL
	int fd;

	//Counters (for easy rejection of flow_mods)
	unsigned int num_of_wildcarded_entries;
	unsigned int num_of_exact_entries;

	//Exact entry table
	struct netfpga_flow_entry* hw_exact_table[NETFPGA_OPENFLOW_EXACT_TABLE_SIZE];
	
	//Wildcarded
	struct netfpga_flow_entry* hw_wildcard_table[NETFPGA_OPENFLOW_WILDCARD_TABLE_SIZE];

	//Mutex (serialization over
	pthread_mutex_t mutex;

}netfpga_device_t;


//C++ extern C
ROFL_BEGIN_DECLS

//
// NetFPGA management
//

//Getter only used internally in the netfpga code
netfpga_device_t* netfpga_get(void);
	
/**
* @brief   Initializes the netfpga shared state, including appropiate state of registers and bootstrap.
*/
rofl_result_t netfpga_init(void);

rofl_result_t netfpga_destroy(void);

/**
* @brief Destroys state of the netfpga, and restores it to the original state (state before init) 
*/
rofl_result_t netfpga_destroy(void);

/**
* @brief Lock netfpga so that other threads cannot do operations of it. 
*/
void netfpga_lock(void);

/**
* @brief Unlock netfpga so that other threads can do operations of it. 
*/
void netfpga_unlock(void);



//
// Entry table management
//

/**
* @brief Sets the table default policy 
*/
rofl_result_t netfpga_set_table_behaviour(void);


//
// Flow mods
//

/**
* @brief Add flow entry to table. Assumes already mutual exclusion over table 
*/
rofl_result_t netfpga_add_flow_entry(of1x_flow_entry_t* entry);

/**
* @brief Deletes an specific entry defined by *entry. Assumes already mutual exclusion over table 
*/
rofl_result_t netfpga_delete_flow_entry(of1x_flow_entry_t* entry);


/**
* @brief Deletes all entries within a table. Assumes already mutual exclusion over table 
*/
rofl_result_t netfpga_delete_all_entries(void);


//Dumping stuff, only used for debug
rofl_result_t netfpga_dump_wildcard_hw_entries(void);



//C++ extern C
ROFL_END_DECLS

#endif //NETFPGA_H
