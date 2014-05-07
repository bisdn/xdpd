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

//XXX delete this
#define DRIVER_NAME "[xdpd][dpdk]"

/*
* BG stuff
*/

//Frequency(period) of port link status updating in milliseconds
#define BG_UPDATE_PORT_LINKS_MS 400

//Frequency(period) of port stats updating in milliseconds
#define BG_UPDATE_PORT_STATS_MS 2000

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
#define IO_PKT_IN_STORAGE_EXPIRATION_S 180

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
#define RTE_TEST_RX_DESC_DEFAULT 512
#define RTE_TEST_TX_DESC_DEFAULT 512

/*
* Processing stuff
*/
//Nothing yet

//DPDK defines
#define RTE_CORE_MASK 0x00000003 //2 cores(1 doing I/O)
#define RTE_MEM_CHANNELS 2
#define SOCKET0 0
#define MBUF_SIZE (/*2048*/8192 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)
#define NB_MBUF   8192

#endif //XDPD_GNU_LINUX_XDPD_CONFIG_H
