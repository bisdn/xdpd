/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PORT_STATE_H_
#define _PORT_STATE_H_

#include "../config.h"

typedef struct dpdk_port_state{

	//Core attachement state
	bool scheduled;
	unsigned int core_id;
	unsigned int core_port_slot;

	//port id (dpdk)
	unsigned int port_id;

	//TX queue(s)
	unsigned int tx_len;
	struct rte_ring* tx_queues[IO_IFACE_NUM_QUEUES];
}dpdk_port_state_t;

#endif //_PORT_STATE_H_
