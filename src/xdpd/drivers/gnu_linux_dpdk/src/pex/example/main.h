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
#include <rofl/common/utils/c_logger.h>
#include "../../config.h"

#define NAME_LENGTH		20

#define MODULE_NAME "PEX"

typedef struct mbuf_array
{
	struct rte_mbuf *array[PKT_TO_PEX_THRESHOLD];
	uint32_t n_mbufs;
}mbuf_array_t;

struct pex_params_t 
{
	char pex_name[NAME_LENGTH];
	
	/* Rings */
	struct rte_ring *up_queue;
	struct rte_ring *down_queue;

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
