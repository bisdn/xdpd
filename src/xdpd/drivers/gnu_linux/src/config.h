/* This Source Code Form is subjecamet to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDPD_GNU_LINUX_CONFIG_H 
#define XDPD_GNU_LINUX_CONFIG_H 

/**
* @file config.h
*
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Configuration file for the xDPd GNU/Linux driver. Configuration is optimized by a 
* system which has 4 hw cores (hw threads). Please adjust it to your needs (specially THREAD numbers).
* @warning Incorrect values on this file can impact heavily the performance of this driver... Be advised
* 
*/

//---------------------------------------------------------//


/*
* General parameters
*/
//Driver name
#define DRIVER_NAME "[gnu-linux]"

//None for the moment

/*
* I/O subsystem parameters
*
* The following parameters tweak the configuration of the GNU/Linux I/O subsystem. There are two major items
* one may want to tweak:
*
* - The amount of threads used for doing I/O: the IO_RX/TX_THREADS defines the amount of threads that may be used
*   for serving the ports. As a rule of thumb, one should configure: 
*
*                 IO_RX_THREADS+IO_TX_THREADS+1 == Number of cores or CPUs of the system
*                 IO_RX_THREADS+IO_TX_THREADS   <= IO_MAX_THREADS  
*
* - The amount of buffers: IO_BUFFERPOOL_CAPACITY defines the amount of pre-allocated buffers.
* - The number of (soft) queues per port: IO_IFACE_NUM_QUEUES
*
* It is recommended that IO_RX_THREADS >= IO_TX_THREADS
*
*/

//Num of RX(and OF processing) threads
#define IO_RX_THREADS 2

//Total number of TX threads
#define IO_TX_THREADS 2

//Maximum number of threads (pre-allocation)
//Warning: do not modify this value unless you know what you are doing!
#define IO_MAX_THREADS 64

//Number of output queues per interface
#define IO_IFACE_NUM_QUEUES 8

/*
* Buffer pool section
*/

//Bufferpool reservoir(PKT_INs); ideally at least X*max_num_lsis
#define IO_BUFFERPOOL_RESERVOIR 2048

//Number of buffers available for I/O. Dimension according to the 
//the maximum number of interfaces that can run at the same time
//Warning: changing the size of this variable can affect performance 
#define IO_BUFFERPOOL_CAPACITY 2048*16 //32K buffers

/*
* Port scheduling strategy
*/

//Use polling_ioscheduler
//Warning: not recommended!
//#define IO_POLLING_STRATEGY


/*
* Interface section
*/

//RX/TX ring size and output queue dimensions
//Align to a power of 2
#define IO_IFACE_RING_SLOTS 2048

//
// ioport_mmap specifics
//

//Max frame size (WARNING: do not go beyond 8192 bytes, and never underneath 2048 bytes)
//Align to a power of 2
#define IO_IFACE_MMAP_FRAME_SIZE 2048
//Do not touch these values unless you know what your are doing
#define IO_IFACE_MMAP_BLOCKS 2 
#define IO_IFACE_MMAP_BLOCK_SIZE 96

#define VETH_DISABLE_CHKSM_OFFLOAD 1

/*
* Kernel scheduling section
*/

//Kernel scheduling policy for I/O threads. Possible values SCHED_FIFO, SCHED_RR or SCHED_OTHER
//Warning: change it only if you know what you are doing 
#define IO_KERN_SCHED_POL SCHED_OTHER

//Uncomment this to prevent the driver to change priority and scheduling policy for I/O threads
//Warning: change it only if you know what you are doing 
//#define IO_KERN_DONOT_CHANGE_SCHED 


/*
* LSI parameters
*
* Parameters specified by logical switch instance
*/

//Per thread input queue to the switch
//Align to a power of 2
#define LSI_INPUT_QUEUE_SLOTS 1024 

//Per thread input queue to the switch
//Align to a power of 2
//WARNING: do not over-size it or congestion can be created
#define LSI_PKT_IN_QUEUE_SLOTS 64

//Buffer storage(PKT_IN) max buffers per LSI
#define LSI_PKT_IN_STORAGE_MAX_BUF 512

//Buffer storage(PKT_IN) expiration time (seconds)
#define LSI_PKT_IN_STORAGE_EXPIRATION_S 10


/* 
* Other
*/

//Only enable if you want to profile code. This MUST NOT
//be enabled in "production"
//#define ENABLE_TIME_MEASUREMENTS


//---------------------------------------------------------//

#endif //XDPD_GNU_LINUX_CONFIG_H
