/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_FLOW_ENTRY_H_
#define _NETFPGA_FLOW_ENTRY_H_

#include <inttypes.h>
#include <stdbool.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>

#include "netfpga.h"
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
typedef struct netfpga_align_mac_addr{
	uint8_t addr[6];
}netfpga_align_mac_addr_t;

//Matches
#pragma pack(1)	
typedef struct netfpga_flow_entry_matches {
	uint16_t transp_dst;
	uint16_t transp_src;
	uint8_t ip_proto;
	uint32_t ip_dst;
	uint32_t ip_src;
	uint16_t eth_type;
	netfpga_align_mac_addr_t eth_dst;
	netfpga_align_mac_addr_t eth_src;
	uint8_t src_port;
	uint8_t ip_tos;
	uint16_t vlan_id;
	uint8_t pad;
}netfpga_flow_entry_matches_t;

COMPILER_ASSERT( FLOW_ENTRY_MATCHES, ( sizeof(netfpga_flow_entry_matches_t) == 32) );

//Masks
typedef netfpga_flow_entry_matches_t netfpga_flow_entry_matches_mask_t;

#define NETFPGA_FLOW_ENTRY_MATCHES_WORD_LEN (sizeof(netfpga_flow_entry_matches_t)/4)

#define NETFPGA_FLOW_ENTRY_WILDCARD_WORD_LEN (sizeof(netfpga_flow_entry_matches_mask_t)/4)

//Actions

//Enum for the bitfield
#pragma pack(1)	
enum netfpga_action_type { //OF1.0 
	NETFPGA_AT_OUTPUT,           /* Output to switch port. */
	NETFPGA_AT_SET_VLAN_VID,     /* Set the 802.1q VLAN id. */
	NETFPGA_AT_SET_VLAN_PCP,     /* Set the 802.1q priority. */
	NETFPGA_AT_STRIP_VLAN,       /* Strip the 802.1q header. */
	NETFPGA_AT_SET_DL_SRC,       /* Ethernet source address. */
	NETFPGA_AT_SET_DL_DST,       /* Ethernet destination address. */
	NETFPGA_AT_SET_NW_SRC,       /* IP source address. */
	NETFPGA_AT_SET_NW_DST,       /* IP destination address. */
	NETFPGA_AT_SET_NW_TOS,       /* IP ToS (DSCP field, 6 bits). */
	NETFPGA_AT_SET_TP_SRC,       /* TCP/UDP source port. */
	NETFPGA_AT_SET_TP_DST,       /* TCP/UDP destination port. */
	NETFPGA_AT_ENQUEUE,          /* Output to queue.  */
	NETFPGA_AT_VENDOR = 0xffff
};
#pragma pack(1)	
typedef struct netfpga_flow_entry_actions {
	uint16_t forward_bitmask;
	uint16_t action_flags;
	uint16_t vlan_id;
	uint8_t vlan_pcp;
	netfpga_align_mac_addr_t eth_src;
	netfpga_align_mac_addr_t eth_dst;
	uint32_t ip_src;
	uint32_t ip_dst;
	uint8_t ip_tos;
	uint16_t transp_src;
	uint16_t transp_dst;
	uint8_t reserved[ 8/*18*/];
}netfpga_flow_entry_actions_t;

COMPILER_ASSERT( FLOW_ENTRY_ACTIONS, ( sizeof(netfpga_flow_entry_actions_t) == 40) );

#define NETFPGA_FLOW_ENTRY_ACTIONS_WORD_LEN (sizeof(netfpga_flow_entry_actions_t)/4)

#pragma pack(pop)

//Type of entry
typedef enum netfpga_flow_entry_type{
	NETFPGA_FE_FIXED = 0,
	NETFPGA_FE_WILDCARDED
}netfpga_flow_entry_type_t;

//Entry
typedef struct netfpga_flow_entry{
	//Position in the table
	unsigned int hw_pos;

	//Entry type	
	netfpga_flow_entry_type_t type;

	//Matches
	netfpga_flow_entry_matches_t* matches;

	//Wildcards
	netfpga_flow_entry_matches_mask_t* masks; 

	//Actions	
	netfpga_flow_entry_actions_t* actions;

	//Reference back to the sw pipeline entry
	of1x_flow_entry_t* ref_back;
}netfpga_flow_entry_t;

//Function prototypes

//C++ extern C
ROFL_BEGIN_DECLS

/**
* @brief Creates a (empty) flow entry (mappable to HW) 
*/
netfpga_flow_entry_t* netfpga_init_flow_entry(void);


/**
* @brief Destroys an entry previously created via netfpga_init_entry() 
*/
void netfpga_destroy_flow_entry(netfpga_flow_entry_t* entry);

/**
* @brief Generates a HW flow entry based on a ROFL-pipeline flow entry 
*/
netfpga_flow_entry_t* netfpga_generate_hw_flow_entry(netfpga_device_t* nfpga, of1x_flow_entry_t* of1x_entry);

//C++ extern C
ROFL_END_DECLS

#endif //NETFPGA_FLOW_ENTRY_H
