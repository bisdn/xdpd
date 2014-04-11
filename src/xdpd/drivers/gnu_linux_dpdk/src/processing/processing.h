/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PROCESSING_H_
#define _PROCESSING_H_

#include <rofl.h>
#include "../config.h"
#include <rte_config.h> 
#include <rte_common.h> 
#include <rte_eal.h> 
#include <rte_log.h>
#include <rte_launch.h> 
#include <rte_mempool.h> 
#include <rte_mbuf.h> 
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_launch.h>

#include "../io/dpdk_datapacket.h"

#define PROCESSING_MAX_PORTS_PER_CORE 32
#define PROCESSING_MAX_PORTS 128 

//Burst definition(queue)
struct mbuf_burst {
	unsigned len;
	struct rte_mbuf *burst[IO_IFACE_MAX_PKT_BURST];
};

//Port queues
typedef struct port_queues{
	//This are TX-queues of a port
	bool present; //signals that it is present AND is attached (usable by I/O subsytem)
	unsigned int core_id; //core id serving RX/TX on this port
	struct mbuf_burst tx_queues_burst[IO_IFACE_NUM_QUEUES];
}port_queues_t;

/**
* Core task list
*/
typedef struct core_tasks{
	bool available;
	bool active;
	unsigned int num_of_rx_ports;
	switch_port_t* port_list[PROCESSING_MAX_PORTS_PER_CORE]; //active ports MUST be on the very beginning of the array, contiguously.
	
	//This are the TX-queues for ALL ports in the system; index is port_id
	port_queues_t all_ports[PROCESSING_MAX_PORTS];
}core_tasks_t;


/**
* Processig core tasks 
*/
extern core_tasks_t processing_core_tasks[RTE_MAX_LCORE];


/**
* Total number of ports (scheduled, so usable by the I/O)
*/
extern unsigned int total_num_of_ports;

//C++ extern C
ROFL_BEGIN_DECLS

/**
* Initialize data structures for processing to work 
*/
rofl_result_t processing_init(void);

/**
* Destroy data structures for processing to work 
*/
rofl_result_t processing_destroy(void);

/**
* Schedule port to a core 
*/
rofl_result_t processing_schedule_port(switch_port_t* port);

/**
* Deschedule port to a core 
*/
rofl_result_t processing_deschedule_port(switch_port_t* port);


/**
* Packet processing routine for cores 
*/
int processing_core_process_packets(void*);

/**
* Dump core state
*/
void processing_dump_core_state(void);

//C++ extern C
ROFL_END_DECLS

#endif //_PROCESSING_H_
