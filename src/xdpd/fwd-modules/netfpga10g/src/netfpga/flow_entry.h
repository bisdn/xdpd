/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_FLOW_ENTRY_H_
#define _NETFPGA_FLOW_ENTRY_H_

#include <inttypes.h>
#include <stdbool.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_pipeline.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_flow_entry.h>

#include "../util/compiler_assert.h"

/**
* @file flow_entry.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NetFPGA flow_entry basic abstraction
*/


/*
* Data structures
*/

#pragma pack(push)
#pragma pack(1)	

/** Mapped to HW structures **/

//Matches
typedef struct netfpga_flow_entry_matches {
	uint16_t transp_dst;
	uint16_t transp_src;
	uint8_t ip_proto;
	uint32_t ip_dst;
	uint32_t ip_src;
	uint16_t eth_type;
	uint8_t eth_dst[6];
	uint8_t eth_src[6];
	uint8_t src_port;
	uint8_t ip_tos;
	uint16_t vlan_id;
	uint8_t pad;
}netfpga_flow_entry_matches_t;

COMPILER_ASSERT( FLOW_ENTRY_MATCHES, ( sizeof(netfpga_flow_entry_matches_t) == 32) );

//Masks
typedef netfpga_flow_entry_matches_t netfpga_flow_entry_matches_mask_t;

//Actions
typedef struct netfpga_flow_entry_actions {
	uint16_t forward_bitmask;
	uint16_t nf2_action_flag;
	uint16_t vlan_id;
	uint8_t vlan_pcp;
	uint8_t eth_src[6];
	uint8_t eth_dst[6];
	uint32_t ip_src;
	uint32_t ip_dst;
	uint8_t ip_tos;
	uint16_t transp_src;
	uint16_t transp_dst;
	uint8_t reserved[ 8/*18*/];
}netfpga_flow_entry_actions_t;

COMPILER_ASSERT( FLOW_ENTRY_ACTIONS, ( sizeof(netfpga_flow_entry_actions_t) == 40) );

#pragma pack(pop)

//Type of entry
typedef enum netfpga_flow_entry_type{
	NETFPGA_FE_FIXED,
	NETFPGA_FE_WILDCARDED
}netfpga_flow_entry_type_t;

//Entry
typedef struct netfpga_flow_entry{
	//Position in the table
	unsigned int hw_pos;

	//Entry type	
	netfpga_flow_entry_type_t type;

	//Matches
	netfpga_flow_entry_matches_mask_t* mask; 

	//Wildcards
	netfpga_flow_entry_matches_t* matches;

	//Actions	
	netfpga_flow_entry_actions_t* actions;

	//Reference back to the sw pipeline entry
	of12_flow_entry_t* ref_back;
}netfpga_flow_entry_t;

//Function prototypes

/**
* @brief Creates a (empty) flow entry (mappable to HW) 
*/
netfpga_flow_entry_t* netfpga_init_entry(void);


/**netfpga_flow_entry_t
* @brief Destroys an entry previously created via netfpga_init_entry() 
*/
void netfpga_destroy_entry(netfpga_flow_entry_t* entry);


#endif //NETFPGA_FLOW_ENTRY_H
