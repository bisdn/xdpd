/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDPD_GNU_LINUX_CONFIG_H 
#define XDPD_GNU_LINUX_CONFIG_H 

/**
* @file config.h
*
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Configuration file for the xDPd GNU/Linux forwarding module. Configuration is optimized by a 
* system which has 4 hw cores (hw threads). Please adjust it to your needs (specially THREAD numbers).
* @warning Incorrect values on this file can impact heavily the performance of this forwarding module... Be advised
* 
*/

//---------------------------------------------------------//

/**
*
* TODO: Detailed explanation
* 
* As a rule of thumb, try to adjust number of threads per subsystem according to
* 
* Ncores_system >= Nio + Tlsi*Nlsi + 2
* 
* Where:
*   Ncores_system - Number of hw threads (hw cores) in the system
*   Nio - number of threads dedicated to the I/O subsystem
*   Tlsi - threads per LSI
*   Nlsi - number of LSI that . The system will STILL be capable of instantiating more, perhaps with some performance penalty
*   2 - This is a fixed number of threads, 1 for the bg and one for ctl attendance code (CMM)
*
*/

/*
* General parameters
*/

//None for the moment

/*
* I/O subsystem parameters
*/

//Num of maximum threads (hw threads) devoted to the I/O of interfaces (Nio)
#define IO_TOTAL_THREADS 2 

//Number of output queues per interface
#define IO_IFACE_NUM_QUEUES 8

//Bufferpool reservoir(PKT_INs); ideally at least X*max_num_lsis
#define IO_BUFFERPOOL_RESERVOIR 2048

//Max frame size (WARNING: do not go beyond 8192 bytes, and never underneath 2048 bytes)
//Align to a power of 2
#define IO_IFACE_MMAP_FRAME_SIZE 2048
//Do not touch these values unless you know what your are doing
#define IO_IFACE_MMAP_BLOCKS 2 
#define IO_IFACE_MMAP_BLOCK_SIZE 96

//RX/TX ring size and output queue dimensions
//Align to a power of 2
#define IO_IFACE_RING_SLOTS 2048

//Required buffers per interface
//(bufferpool will be dimensioned to be at least Nifaces*IO_IFACE_REQUIRED_BUFFERS)
#define IO_IFACE_REQUIRED_BUFFERS 2048

//Buffer storage(PKT_IN) max buffers per LSI
#define IO_PKT_IN_STORAGE_MAX_BUF 512
//Buffer storage(PKT_IN) expiration time (seconds)
#define IO_PKT_IN_STORAGE_EXPIRATION_S 180

/*
* Processing subsystem parameters
*/

//Num of processing threads per Logical Switch Instance (Tlsi)
#define PROCESSING_THREADS_PER_LSI 2

//Per thread input queue to the switch
//Align to a power of 2
#define PROCESSING_INPUT_QUEUE_SLOTS 1024 

//Per thread input queue to the switch
//Align to a power of 2
//WARNING: do not over-size it or congestion can be created
#define PROCESSING_PKT_IN_QUEUE_SLOTS 4

/* 
* Other
*/

//Only enable if you want to profile code. This MUST NOT
//be enabled in "production"
//#define ENABLE_TIME_MEASUREMENTS



//---------------------------------------------------------//

#endif //XDPD_GNU_LINUX_CONFIG_H
