#include <assert.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_async_events_hooks.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_table.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>
#include <rofl/common/utils/c_logger.h>

#include "../config.h"
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_spinlock.h>
#include <rte_ring.h>


#include "../io/pktin_dispatcher.h"
#include "../io/bufferpool.h"

using namespace xdpd::gnu_linux;

//MBUF pool
extern struct rte_mempool* pool_direct;

/*
* Hooks for configuration of the switch
*/
rofl_result_t platform_post_init_of1x_switch(of1x_switch_t* sw){

	ROFL_INFO(" calling %s()\n",__FUNCTION__);

	return ROFL_SUCCESS;
}

rofl_result_t platform_pre_destroy_of1x_switch(of1x_switch_t* sw){

	ROFL_INFO(" calling %s()\n",__FUNCTION__);
	

	return ROFL_SUCCESS;
}


//Async notifications

/*
* Packet in
*/


void platform_of1x_packet_in(const of1x_switch_t* sw, uint8_t table_id, datapacket_t* pkt, of_packet_in_reason_t reason)
{
	datapacket_t* pkt_replica=NULL;
	struct rte_mbuf* mbuf=NULL;
	datapacketx86* pktx86;
	datapacketx86* pktx86_replica;
	
	ROFL_INFO(" calling %s()\n",__FUNCTION__);

	//Protect
	if(unlikely(!pkt) || unlikely(!sw))
		return;

	//datapacket_t* pkt_replica;
	pkt_replica = bufferpool::get_free_buffer(false);
	
	if(unlikely(!pkt_replica)){
		//TODO: add trace
		goto PKT_IN_ERROR;
	}

	pktx86 = (datapacketx86*)pkt->platform_state;
	pktx86_replica = (datapacketx86*)pkt_replica->platform_state;

	//Retrieve an mbuf, copy contents, and initialize pktx86_replica
	mbuf = rte_pktmbuf_alloc(pool_direct);
	
	if(unlikely(!mbuf)){
		//TODO: add trace
		goto PKT_IN_ERROR;
	}

	//Copy PKT stuff
	memcpy(&pkt_replica->matches, &pkt->matches, sizeof(pkt->matches));
	memcpy(&pkt_replica->write_actions, &pkt->write_actions ,sizeof(pkt->write_actions));

	//mark as replica
	pkt_replica->is_replica = true;
	pkt_replica->sw = pkt->sw;

	//Initialize replica buffer (without classification)
	pktx86->init((uint8_t*)mbuf->buf_addr, mbuf->buf_len, (of_switch_t*)sw, pktx86->in_port, 0, false, false);

	//Replicate the packet(copy contents)	
	memcpy(pktx86_replica->get_buffer(), pktx86->get_buffer(), pktx86->get_buffer_length());
	pktx86_replica->ipv4_recalc_checksum 	= pktx86->ipv4_recalc_checksum;
	pktx86_replica->icmpv4_recalc_checksum 	= pktx86->icmpv4_recalc_checksum;
	pktx86_replica->tcp_recalc_checksum 	= pktx86->tcp_recalc_checksum;
	pktx86_replica->udp_recalc_checksum 	= pktx86->udp_recalc_checksum;

	//Classify
	//TODO: this could be improved by copying the classification state
	pktx86_replica->headers->classify();	

	//Store in PKT_IN ring
	if( unlikely( enqueue_pktin(pkt_replica) != ROFL_SUCCESS ) ){
		//PKT_IN ring full
		//TODO: add trace
		goto PKT_IN_ERROR;
		
	}

	return; //DO NOT REMOVE

PKT_IN_ERROR:
	//assert(0); //Sometimes useful while debugging. In general should be disabled
	
	//Release packet
	if(pkt_replica){
		bufferpool::release_buffer(pkt_replica);

		if(mbuf){
			rte_pktmbuf_free(mbuf);
		}
	}
	return;
}

//Flow removed
void platform_of1x_notify_flow_removed(const of1x_switch_t* sw, 	
						of1x_flow_remove_reason_t reason, 
						of1x_flow_entry_t* removed_flow_entry){

	ROFL_INFO(" calling %s()\n",__FUNCTION__);
	cmm_process_of1x_flow_removed(sw, (uint8_t)reason, removed_flow_entry);

}


void plaftorm_of1x_add_entry_hook(of1x_flow_entry_t* new_entry){

}

void platform_of1x_modify_entry_hook(of1x_flow_entry_t* old_entry, of1x_flow_entry_t* mod, int reset_count){

}

void platform_of1x_remove_entry_hook(of1x_flow_entry_t* entry){

}

void platform_of1x_update_stats_hook(of1x_flow_entry_t* entry) {

}
