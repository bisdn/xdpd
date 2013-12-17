/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_REGS_
#define _NETFPGA_REGS_

#include <stdbool.h>
#include <linux/sockios.h>
#include <rofl.h>
#include <rofl/common/utils/c_logger.h>

#include "../util/compiler_assert.h"
#include "netfpga.h"
#include "xparameters.h"

/**
* @file regs.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NetFPGA register manipulation routines 
*/

//Constants

#define NETFPGA_NUMBER_OF_MISC_STATS 29

#define NETFPGA_IOCTL_CMD_READ_STAT (SIOCDEVPRIVATE+0)
#define NETFPGA_IOCTL_CMD_WRITE_REG (SIOCDEVPRIVATE+1)
#define NETFPGA_IOCTL_CMD_READ_REG (SIOCDEVPRIVATE+2)

/********************************************************
 * Constants underneath this comment are gathered from:
 * https://github.com/eastzone/openflow/tree/netfpga-10g 
 ********************************************************/

#define NETFPGA_TYPE_VLAN	0x8100
#define NETFPGA_TYPE_VLAN_PCP	0x88aa
#define NETFPGA_TYPE_IP		0x0800
#define NETFPGA_TYPE_ARP	0x0806
#define NETFPGA_TYPE_MPLS	0x8847
#define NETFPGA_TYPE_MPLS_MC	0x8848
#define NETFPGA_NO_VLAN		0xffff
#define NETFPGA_PROTO_ICMP	0x01
#define NETFPGA_PROTO_TCP	0x06
#define NETFPGA_POTO_UDP	0x11
#define NETFPGA_PROTO_SCTP	0x84

// Ports to forward on
#define NETFPGA_OPENFLOW_FORWARD_BITMASK_POS	0
#define NETFPGA_OPENFLOW_FORWARD_BITMASK_WIDTH	16
#define NETFPGA_OPENFLOW_NF2_ACTION_FLAG_POS	16
#define NETFPGA_OPENFLOW_NF2_ACTION_FLAG_WIDTH	16
#define NETFPGA_OPENFLOW_SET_VLAN_VID_POS	32

// Vlan ID to be replaced
#define NETFPGA_OPENFLOW_SET_VLAN_VID_WIDTH	16
#define NETFPGA_OPENFLOW_SET_VLAN_PCP_POS 	48

// Vlan priority to be replaced
#define NETFPGA_OPENFLOW_SET_VLAN_PCP_WIDTH	8
#define NETFPGA_OPENFLOW_SET_DL_SRC_POS		56

// Source MAC address to be replaced
#define NETFPGA_OPENFLOW_SET_DL_SRC_WIDTH 	48
#define NETFPGA_OPENFLOW_SET_DL_DST_POS		104

// Destination MAC address to be replaced
#define NETFPGA_OPENFLOW_SET_DL_DST_WIDTH 	48
#define NETFPGA_OPENFLOW_SET_NW_SRC_POS		152

// Source network address to be replaced
#define NETFPGA_OPENFLOW_SET_NW_SRC_WIDTH 	32
#define NETFPGA_OPENFLOW_SET_NW_DST_POS		184

// Destination network address to be replaced
#define NETFPGA_OPENFLOW_SET_NW_DST_WIDTH 	32
#define NETFPGA_OPENFLOW_SET_NW_TOS_POS		216

// TOS to be replaced
#define NETFPGA_OPENFLOW_SET_NW_TOS_WIDTH 	6
#define NETFPGA_OPENFLOW_SET_NW_ECN_POS		222

// ECN to be replaced
#define NETFPGA_OPENFLOW_SET_NW_ECN_WIDTH	2
#define NETFPGA_OPENFLOW_SET_TP_SRC_POS		224

// Source transport port to be replaced
#define NETFPGA_OPENFLOW_SET_TP_SRC_WIDTH	16
#define NETFPGA_OPENFLOW_SET_TP_DST_POS		240

// Destination transport port to be replaced
#define NETFPGA_OPENFLOW_SET_TP_DST_WIDTH	16
#define NETFPGA_OFPAT_OUTPUT			0x0001
#define NETFPGA_OFPAT_SET_VLAN_VID		0x0002
#define NETFPGA_OFPAT_SET_VLAN_PCP		0x0004
#define NETFPGA_OFPAT_POP_VLAN			0x0008
#define NETFPGA_OFPAT_SET_DL_SRC		0x0010
#define NETFPGA_OFPAT_SET_DL_DST		0x0020
#define NETFPGA_OFPAT_SET_NW_SRC		0x0040
#define NETFPGA_OFPAT_SET_NW_DST		0x0080
#define NETFPGA_OFPAT_SET_NW_TOS		0x0100
#define NETFPGA_OFPAT_SET_TP_SRC		0x0200
#define NETFPGA_OFPAT_SET_TP_DST		0x0400
#define NETFPGA_OFPAT_SET_NW_ECN		0x0800
#define NETFPGA_OFPAT_PUSH_VLAN			0x1000
#define NETFPGA_OFPAT_SET_NW_TTL		0x2000
#define NETFPGA_OFPAT_DEC_NW_TTL		0x4000

#define NETFPGA_OPENFLOW_PKT_COUNTER_POS	0
#define NETFPGA_OPENFLOW_PKT_COUNTER_WIDTH	25
#define NETFPGA_OPENFLOW_LAST_SEEN_POS		25
#define NETFPGA_OPENFLOW_LAST_SEEN_WIDTH	7
#define NETFPGA_OPENFLOW_BYTE_COUNTER_POS	32
#define NETFPGA_OPENFLOW_BYTE_COUNTER_WIDTH	32
#define NETFPGA_WILDCARD_BASE	 		0x8000
#define NETFPGA_EXACT_BASE			0x0000

/*
* Modules
*/

// Module tags
#define NETFPGA_OF_BASE_ADDR	XPAR_OPENFLOW_DATAPATH_0_S_AXI_BASEADDR

// Registers

// Name: openflow_datapath(OF)
// Description: openflow_datapath
#define NETFPGA_OF_NUM_PKTS_DROPPED_0_REG	NETFPGA_OF_BASE_ADDR+0x00
#define NETFPGA_OF_NUM_PKTS_DROPPED_1_REG	NETFPGA_OF_BASE_ADDR+0x01
#define NETFPGA_OF_NUM_PKTS_DROPPED_2_REG	NETFPGA_OF_BASE_ADDR+0x02
#define NETFPGA_OF_NUM_PKTS_DROPPED_3_REG	NETFPGA_OF_BASE_ADDR+0x03
#define NETFPGA_OF_NUM_PKTS_DROPPED_4_REG	NETFPGA_OF_BASE_ADDR+0x04
#define NETFPGA_OF_EXACT_HITS_REG		NETFPGA_OF_BASE_ADDR+0x05
#define NETFPGA_OF_EXACT_MISSES_REG		NETFPGA_OF_BASE_ADDR+0x06
#define NETFPGA_OF_WILDCARD_HITS_REG		NETFPGA_OF_BASE_ADDR+0x07
#define NETFPGA_OF_WILDCARD_MISSES_REG		NETFPGA_OF_BASE_ADDR+0x08
#define NETFPGA_OF_DL_PARSE_CNT_0_REG		NETFPGA_OF_BASE_ADDR+0x09
#define NETFPGA_OF_DL_PARSE_CNT_1_REG		NETFPGA_OF_BASE_ADDR+0x0a
#define NETFPGA_OF_DL_PARSE_CNT_2_REG		NETFPGA_OF_BASE_ADDR+0x0b
#define NETFPGA_OF_DL_PARSE_CNT_3_REG		NETFPGA_OF_BASE_ADDR+0x0c
#define NETFPGA_OF_DL_PARSE_CNT_4_REG		NETFPGA_OF_BASE_ADDR+0x0d
#define NETFPGA_OF_MPLS_PARSE_CNT_0_REG		NETFPGA_OF_BASE_ADDR+0x0e
#define NETFPGA_OF_MPLS_PARSE_CNT_1_REG		NETFPGA_OF_BASE_ADDR+0x0f
#define NETFPGA_OF_MPLS_PARSE_CNT_2_REG		NETFPGA_OF_BASE_ADDR+0x10
#define NETFPGA_OF_MPLS_PARSE_CNT_3_REG		NETFPGA_OF_BASE_ADDR+0x11
#define NETFPGA_OF_MPLS_PARSE_CNT_4_REG		NETFPGA_OF_BASE_ADDR+0x12
#define NETFPGA_OF_ARP_PARSE_CNT_0_REG		NETFPGA_OF_BASE_ADDR+0x13
#define NETFPGA_OF_ARP_PARSE_CNT_1_REG		NETFPGA_OF_BASE_ADDR+0x14
#define NETFPGA_OF_ARP_PARSE_CNT_2_REG		NETFPGA_OF_BASE_ADDR+0x15
#define NETFPGA_OF_ARP_PARSE_CNT_3_REG		NETFPGA_OF_BASE_ADDR+0x16
#define NETFPGA_OF_ARP_PARSE_CNT_4_REG		NETFPGA_OF_BASE_ADDR+0x17
#define NETFPGA_OF_IP_TP_PARSE_CNT_0_REG	NETFPGA_OF_BASE_ADDR+0x18
#define NETFPGA_OF_IP_TP_PARSE_CNT_1_REG	NETFPGA_OF_BASE_ADDR+0x19
#define NETFPGA_OF_IP_TP_PARSE_CNT_2_REG	NETFPGA_OF_BASE_ADDR+0x1a
#define NETFPGA_OF_IP_TP_PARSE_CNT_3_REG	NETFPGA_OF_BASE_ADDR+0x1b
#define NETFPGA_OF_IP_TP_PARSE_CNT_4_REG	NETFPGA_OF_BASE_ADDR+0x1c
#define NETFPGA_OF_LAST_OUTSIDE_REG		NETFPGA_OF_BASE_ADDR+0x1c
#define NETFPGA_OF_ACC_RDY_REG			NETFPGA_OF_BASE_ADDR+0x1d
#define NETFPGA_OF_BASE_ADDR_REG	 	NETFPGA_OF_BASE_ADDR+0x1e
#define NETFPGA_OF_WRITE_ORDER_REG		NETFPGA_OF_BASE_ADDR+0x1f
#define NETFPGA_OF_MOD_WRITE_ORDER_REG		NETFPGA_OF_BASE_ADDR+0x20
#define NETFPGA_OF_READ_ORDER_REG		NETFPGA_OF_BASE_ADDR+0x21
#define NETFPGA_OF_ENTRY_BASE_REG		NETFPGA_OF_BASE_ADDR+0x22
#define NETFPGA_OF_LOOKUP_CMP_BASE_REG		NETFPGA_OF_BASE_ADDR+0x22
#define NETFPGA_OF_LOOKUP_CMP_MASK_BASE_REG	NETFPGA_OF_BASE_ADDR+0x2a
#define NETFPGA_OF_LOOKUP_ACTION_BASE_REG	NETFPGA_OF_BASE_ADDR+0x32
#define NETFPGA_OF_STATS_BASE_REG		NETFPGA_OF_BASE_ADDR+0x3c

/* End of constants */

//Data structures
typedef struct netfpga_register{
	uint32_t reg_id;
	uint32_t reg_val;
}netfpga_register_t;

COMPILER_ASSERT( Register_alignment, (sizeof(netfpga_register_t) == 8));
	
//C++ extern C
ROFL_BEGIN_DECLS

//Prototypes
/* Function declarations */

/**
* @brief Attempt to read a register 
*/
rofl_result_t netfpga_read_reg(netfpga_device_t *nfpga, uint32_t reg_id, uint32_t *value);

/**
* @brief Attempt to write in a register 
*/
rofl_result_t netfpga_write_reg(netfpga_device_t *nfpga, uint32_t reg_id, uint32_t value);

/**
* @brief Wait until NETFPGA_OF_ACC_RDY_REG is 1  
*/
void netfpga_wait_reg_ready(netfpga_device_t *nfpga);

//C++ extern C
ROFL_END_DECLS

#endif //NETFPGA_REGS
