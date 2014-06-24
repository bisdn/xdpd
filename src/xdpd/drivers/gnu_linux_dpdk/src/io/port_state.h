/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PORT_STATE_H_
#define _PORT_STATE_H_

#include "../config.h"

struct dpdk_port_state{
	//Core attachement state
	bool scheduled;
	bool queues_set; 
	unsigned int core_id;
	unsigned int core_port_slot;

	//port id (dpdk)
	unsigned int port_id;
}__rte_cache_aligned;

typedef struct dpdk_port_state dpdk_port_state_t;  

#endif //_PORT_STATE_H_
