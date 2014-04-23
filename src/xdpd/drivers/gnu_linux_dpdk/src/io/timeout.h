/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _TIMEOUT_H_
#define _TIMEOUT_H_

#include <rte_config.h>
#include <rte_cycles.h>

#include "port_manager.h"

namespace xdpd {
namespace gnu_linux_dpdk {


inline void
port_timeout(switch_port_t *port)
{
	uint64_t tmp;
	
	if(port->type != PORT_TYPE_PEX)
		return;

	pex_port_state *port_state = (pex_port_state_t*)port->platform_port_state;

	//check if the timeout is expired
	tmp = port_state->last_flush_time;
	while((rte_rdtsc() - tmp) > TIME_THRESHOLD)
	{
		if(__sync_bool_compare_and_swap(&(port_state->last_flush_time),tmp,rte_rdtsc()) == true)
		{
			sem_post(port_state->semaphore);
			break;
		}
		tmp = port_state->last_flush_time;
	}
}

}
}

#endif // _TIMEOUT_H_
