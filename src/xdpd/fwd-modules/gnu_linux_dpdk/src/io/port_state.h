/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PORT_STATE_H_
#define _PORT_STATE_H_

#include "../config.h"

#define MAX_PKT_BURST 32

struct mbuf_table {
	unsigned len;
	struct rte_mbuf *m_table[IO_IFACE_MAX_PKT_BURST];
};

struct dpdk_port_state{

	//Core attachement state
	bool scheduled;
	unsigned int core_id;
	unsigned int core_port_slot;

	//port id (dpdk)
	unsigned int port_id;

	//TX queue(s)
	struct mbuf_table tx_mbufs[IO_IFACE_NUM_QUEUES];
} __rte_cache_aligned;

typedef struct dpdk_port_state dpdk_port_state_t;  

#endif //_PORT_STATE_H_
