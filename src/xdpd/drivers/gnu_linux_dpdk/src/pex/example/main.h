/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
* @file main.h
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief main file of a DPDK PEX.
*/

#ifndef _MAIN_H_
#define _MAIN_H_

#include <semaphore.h>
#include <fcntl.h>
#include "../../config.h"

#define NAME_LENGTH		20

#define MODULE_NAME "PEX_EXAMPLE"

#define PKT_TO_PEX_THRESHOLD 200

typedef struct mbuf_array
{
	struct rte_mbuf *array[PKT_TO_PEX_THRESHOLD];
	int n_mbufs;
}mbuf_array_t;

struct pex_params_t 
{
	char pex_name[NAME_LENGTH];
	
	/* Rings */
	struct rte_ring *to_pex_queue;
	struct rte_ring *to_xdpd_queue;

	/* Semaphore */
	sem_t *semaphore;

	/* mbuf pools */
	//FIXME: currently it is not used - to be linked to the pool 
	//used by xDPD in order to create new packets
	struct rte_mempool *pool;
 
} __rte_cache_aligned;

struct pex_params_t pex_params;

int do_pex(void*);


#ifdef RTE_EXEC_ENV_BAREMETAL
#define MAIN _main
#else
#define MAIN main
#endif


#endif /* _MAIN_H_ */
