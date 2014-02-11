/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDPD_NETFGPA_10G_CONFIG_H 
#define XDPD_NETFGPA_10G_CONFIG_H 

/**
* @file config.h
*
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Configuration file for the xDPd NetfGPA 10G GNU/Linux forwarding module. 
* 
*/

/*
* General parameters
*/

#define FWD_MOD_NAME "netfpga10g"

//I/O
//Bufferpool reservoir(PKT_INs); ideally at least X*max_num_lsis
#define IO_BUFFERPOOL_RESERVOIR 2048

//Number of buffers available for I/O. Dimension according to the 
//the maximum number of interfaces that can run at the same time
//Warning: changing the size of this variable can have ARNING:
#define IO_BUFFERPOOL_CAPACITY 2048*16 //32K buffers


//Buffer storage(PKT_IN) max buffers
#define IO_PKT_IN_STORAGE_MAX_BUF 512
//Buffer storage(PKT_IN) expiration time (seconds)
#define IO_PKT_IN_STORAGE_EXPIRATION_S 180
//added
#define IO_IFACE_NUM_QUEUES 8
#define IO_IFACE_REQUIRED_BUFFERS 2048

#endif //XDPD_NETFGPA_10G_CONFIG_H
