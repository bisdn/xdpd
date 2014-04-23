/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
* @file runtime.c
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief Executes the PEX.
*/

#include <assert.h>
#include <rte_mbuf.h>

#include "main.h"

int do_pex(void *useless)
{
	mbuf_array_t pkts_received, pkts_to_send;
	
	//FIXME: the next two lines are just for the compiler
	if(useless != NULL)
		return -1;
	
//	ROFL_ERR("["MODULE_NAME"] %s is started!\n", pex_params.pex_name);
	

	while(1)
	{
		sem_wait(pex_params.semaphore);

		/*1) Receive incoming packets */
		
        pkts_received.n_mbufs = rte_ring_sc_dequeue_burst(pex_params.up_queue,(void **)&pkts_received.array[0],PKT_TO_PEX_THRESHOLD);

		if(likely(pkts_received.n_mbufs))
		{
			int i;
			for (i=0;i < pkts_received.n_mbufs;i++)
			{
				pkts_to_send.n_mbufs = 0;

				/*2) Operate on the packet */
			//	ROFL_DEBUG("["MODULE_NAME"] Processing a packet...\n");
			
				pkts_to_send.array[pkts_to_send.n_mbufs] = pkts_received.array[i];
		    	pkts_to_send.n_mbufs++;
		    	
		    //	ROFL_DEBUG("["MODULE_NAME"] The queue towards the vSwitch contains %d packets\n",pkts_to_send.n_mbufs);

				/*
				*	This check is required because the PEX could create (and then send) new packets, which may cause
				*	the reaching of the threshold before that all the packets just received are proecessed
				*/
			    if (unlikely((pkts_to_send.n_mbufs == PKT_TO_PEX_THRESHOLD)))
			    {
			    	/*3) Send the processed packet */
			    
			        int ret = rte_ring_sp_enqueue_burst(pex_params.down_queue,(void *const*)pkts_to_send.array,(unsigned)pkts_to_send.n_mbufs);
			        
			        if (unlikely(ret < pkts_to_send.n_mbufs)) 
			        {
			        	//  	ROFL_INFO("["MODULE_NAME"] Not enough room in the ring towards xDPD to enqueue; the packet will be dropped.\n");
						do {
							struct rte_mbuf *pkt_to_free = pkts_to_send.array[ret];
							rte_pktmbuf_free(pkt_to_free);
						} while (++ret < pkts_to_send.n_mbufs);
					}
			        
		    		pkts_to_send.n_mbufs = 0;
		    	}	
			}
			
			/*3) Send the processed packet */
			if(likely(pkts_to_send.n_mbufs > 0))
			{
				int ret = rte_ring_sp_enqueue_burst(pex_params.down_queue,(void *const*)pkts_to_send.array,(unsigned)pkts_to_send.n_mbufs);
			        
	        	if (unlikely(ret < pkts_to_send.n_mbufs)) 
		        {
		        	//  	ROFL_INFO("["MODULE_NAME"] Not enough room in the ring towards xDPD to enqueue; the packet will be dropped.\n");
					do {
						struct rte_mbuf *pkt_to_free = pkts_to_send.array[ret];
						rte_pktmbuf_free(pkt_to_free);
					} while (++ret < pkts_to_send.n_mbufs);
				}
			
			}

		}/* End of if(pkts_received.n_mbufs) */
		else
		{
	//		ROFL_ERR("["MODULE_NAME"] The PEX has been woken up without packets to be processed!\n");
			assert(0);
		}

	}/*End of while true*/
	return 0;
}

