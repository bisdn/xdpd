/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDPD_GNU_LINUX_XDPD_CONFIG_H 
#define XDPD_GNU_LINUX_XDPD_CONFIG_H 

/**
* @file config.h
*
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Configuration file for the xDPd DPDK GNU/Linux driver. 
*
* TODO 
*/

//Driver name
#define DRIVER_NAME "[xdpd][dpdk]"

//This parameter disallows the driver to schedule phyiscal ports on logical
//cores belonging to a different CPU socket than the port(PCI).
//Warning: disabling this flag can affect performance; consider using a proper
//coremask instead
#define ABORT_ON_UNMATCHED_SCHED

/*
* BG stuff
*/

//Frequency(period) of port link status updating in milliseconds
#define BG_UPDATE_PORT_LINKS_MS 400

//Frequency(period) of port stats updating in milliseconds
#define BG_UPDATE_PORT_STATS_MS 500

//Frequency(period) of handling KNI commands in milliseconds
#define BG_HANDLE_KNI_COMMANDS_MS 1000

/*
* I/O stuff
*/
//Number of output queues per interface
#define IO_IFACE_NUM_QUEUES 1 //8
#define IO_IFACE_MAX_PKT_BURST 32
#define IO_MAX_PACKET_SIZE 1518

//Bufferpool reservoir(PKT_INs); ideally at least X*max_num_lsis
#define IO_BUFFERPOOL_RESERVOIR 2048

 
//TX queue (rte_ring) size
#define IO_TX_LCORE_QUEUE_SLOTS 2<<11 //2048

//Drain timing
#define IO_BURST_TX_DRAIN_US 100 /* TX drain every ~100us */

//Buffer storage(PKT_IN) max buffers
#define IO_PKT_IN_STORAGE_MAX_BUF 2<<9 //512
//Buffer storage(PKT_IN) expiration time (seconds)
#define IO_PKT_IN_STORAGE_EXPIRATION_S 10

/*
 * RX and TX Prefetch, Host, and Write-back threshold values should be
 * carefully set for optimal performance. Consult the network
 * controller's datasheet and supporting DPDK documentation for guidance
 * on how these parameters should be set.
 */
#define RX_PTHRESH 8 /**< Default values of RX prefetch threshold reg. */
#define RX_HTHRESH 8 /**< Default values of RX host threshold reg. */
#define RX_WTHRESH 0 /**< Default values of RX write-back threshold reg. */

/*
 * These default values are optimized for use with the Intel(R) 82599 10 GbE
 * Controller and the DPDK ixgbe PMD. Consider using other values for other
 * network controllers and/or network drivers.
 */
#define TX_PTHRESH 16 /**< Default values of TX prefetch threshold reg. */
#define TX_HTHRESH 4  /**< Default values of TX host threshold reg. */
#define TX_WTHRESH 0  /**< Default values of TX write-back threshold reg. */

/*
 * Configurable number of RX/TX ring descriptors
 */
#define RTE_RX_DESC_DEFAULT 512
#define RTE_TX_DESC_DEFAULT 512

/*
* Processing stuff
*/

//Maximum number of CPU sockets
#define MAX_CPU_SOCKETS 128

//DPDK defines

/*
 * The core mask defines the DPDK core mask.
 *
 * In xDPd the core mask must always include 0x1 as the management core which
 * is reserved and WON'T do I/O
 *
 * So at least another I/O core needs to be defined. Default is Ox2 is the only
 * core doing I/O
 *
 * The coremask can be overriden using driver extra parameters. However this is
 * not recommended for stable setups
 */
#define DEFAULT_RTE_CORE_MASK 0x0000ffff

//Other parameters
#define RTE_MEM_CHANNELS 4
#define MBUF_SIZE 16383

/* (9000 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM) */

/*
 * Default number of MBUFs per pool
 */
#define DEFAULT_NB_MBUF 65365

/**
* NF related parameters
*/

//Enable NF extensions (KNI and secondary process DPDK ports)
#define GNU_LINUX_DPDK_ENABLE_NF

//Maximum number of KNI interfaces, used during preallocation
#define GNU_LINUX_DPDK_MAX_KNI_IFACES 4

/**
* Uncomment the following line to enable the semaphore and implement a sleep/
* wake mechanism between xdpd and the NF process (DPDK secondary process)
*
* NOT RECOMMENDED
*/
//#define ENABLE_DPDK_SECONDARY_SEMAPHORE

//IVANO - FIXME: write a meaningfull value
//#define PKT_TO_NF_THRESHOLD 	200


#endif //XDPD_GNU_LINUX_XDPD_CONFIG_H
