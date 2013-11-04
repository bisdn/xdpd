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
* @brief Configuration file for the xDPd DPDK GNU/Linux forwarding module. 
*
* TODO 
*/

//XXX delete this
#define FWD_MOD_NAME "DPDK"

/*
* I/O stuff
*/
//Number of output queues per interface
#define IO_IFACE_NUM_QUEUES 8
#define IO_IFACE_MAX_PKT_BURST 32

//Bufferpool reservoir(PKT_INs); ideally at least X*max_num_lsis
#define IO_BUFFERPOOL_RESERVOIR 2048

//Drain timing
#if 1
#define IO_BURST_TX_DRAIN 200000ULL /* around 100us at 2 Ghz */
#else
#define IO_BURST_TX_DRAIN_US 100 /* TX drain every ~100us */
#endif

//Buffer storage(PKT_IN) max buffers
#define IO_PKT_IN_STORAGE_MAX_BUF 2<<9 //512
//Buffer storage(PKT_IN) expiration time (seconds)
#define IO_PKT_IN_STORAGE_EXPIRATION_S 180


/*
* Processing stuff
*/
//Nothing yet

//DPDK defines
#define RTE_CORE_MASK 0x2//8 (1 emergency, 8 debug)
#define RTE_MEM_CHANNELS 1//8 (1 emergency, 8 debug)
#define RTE_LOG_LEVEL 1//8 (1 emergency, 8 debug)
#define RTE_MAX_LCORE 64
#define RTE_PKTMBUF_HEADROOM 128
#define SOCKET0 0
#define MBUF_SIZE (/*2048*/8192 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)
#define NB_MBUF   8192
#define RTE_ETHDEV_QUEUE_STAT_CNTRS IO_IFACE_NUM_QUEUES

#endif //XDPD_GNU_LINUX_XDPD_CONFIG_H
