/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PORT_STATE_H_
#define _PORT_STATE_H_

#include "../config.h"

typedef struct dpdk_port_state{
	//RX queue
	unsigned int rx_len;
	struct rte_mbuf* rx_queue;
	
	//TX queue(s)
	unsigned int tx_len;
	struct rte_mbuf* tx_queues[IO_IFACE_NUM_QUEUES];
}dpdk_port_state_t;

#endif //_PORT_STATE_H_
