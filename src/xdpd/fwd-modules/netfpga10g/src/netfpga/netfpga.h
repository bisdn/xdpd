/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_H_
#define _NETFPGA_H_

#include <inttypes.h>
#include <stdbool.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_pipeline.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_flow_entry.h>

/**
* @file netfga.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NetFPGA basic abstractions 
*/

//Constants
#define NETFPGA_RESERVED_FOR_CPU2NETFPGA	8
#define NETFPGA_OPENFLOW_EXACT_TABLE_SIZE		1024    
#define NETFPGA_OPENFLOW_WILDCARD_TABLE_SIZE	32


typedef struct netfpga_dev_info{

	//File descriptor for the netfpga
	int fd;

}netfpga_dev_info_t;

//
// NetFPGA management
//

/**
* @brief   Initializes the netfpga shared state, including appropiate state of registers and bootstrap.
*/
rofl_result_t netfpga_init(void);

/**
* @brief Destroys state of the netfpga, and restores it to the original state (state before init) 
*/
rofl_result_t netfpga_destroy(void);

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
* @brief Deletes an specific entry defined by *entry 
*/
rofl_result_t netfpga_delete_entry(of12_flow_entry_t* entry);


/**
* @brief Deletes all entries within a table 
*/
rofl_result_t netfpga_delete_all_entries(void);


#endif //NETFPGA_H
