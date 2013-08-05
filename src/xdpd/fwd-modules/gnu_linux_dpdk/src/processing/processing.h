/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PROCESSING_H_
#define _PROCESSING_H_

#include <rte_common.h> 
#include <rte_eal.h> 
#include <rte_launch.h> 
#include <rte_mempool.h> 
#include <rte_mbuf.h> 


#include "../config.h"
#include "../io/dpdk_datapacket.h"

/**
* Core 
*/
void core_process_packets(core_task_list_t* task);

#endif //_PROCESSING_H_
