#include <assert.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_async_events_hooks.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_table.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>
#include <rofl/datapath/pipeline/platform/packet.h>
#include <rofl/common/utils/c_logger.h>

#include "../config.h"
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_spinlock.h>
#include <rte_ring.h>


#include "../io/pktin_dispatcher.h"
#include "../io/bufferpool.h"
#include "../io/datapacket_storage.h"
#include "../io/dpdk_datapacket.h"

datapacket_t* platform_packet_replicate__(datapacket_t* pkt, bool copy_mbuf);

using namespace xdpd::gnu_linux;

//MBUF pool
extern struct rte_mempool* pool_direct;

/*
* Hooks for configuration of the switch
*/
rofl_result_t platform_post_init_of1x_switch(of1x_switch_t* sw){

	unsigned int i;

	sw->platform_state = (of_switch_platform_state_t*)new datapacket_storage( IO_PKT_IN_STORAGE_MAX_BUF, IO_PKT_IN_STORAGE_EXPIRATION_S); // todo make this value configurable

	if(unlikely(!sw->platform_state))
		return ROFL_FAILURE;

	//Set number of buffers
	sw->pipeline.num_of_buffers = IO_PKT_IN_STORAGE_MAX_BUF;
	
	//Set the actions and matches supported by this platform
	for(i=0; i<sw->pipeline.num_of_tables; i++){
		of1x_flow_table_config_t *config = &(sw->pipeline.tables[i].config);
		//Lets set to zero the unssuported matches and actions.
		config->apply_actions &= ~(1 << OF12PAT_COPY_TTL_OUT);
		config->apply_actions &= ~(1 << OF12PAT_COPY_TTL_IN);
		config->write_actions &= ~(1 << OF12PAT_COPY_TTL_OUT);
		config->write_actions &= ~(1 << OF12PAT_COPY_TTL_IN);
		
		config->match &= ~(1UL << OF1X_MATCH_SCTP_SRC);
		config->wildcards &= ~(1UL << OF1X_MATCH_SCTP_SRC);
		config->apply_setfields &= ~(1UL << OF1X_MATCH_SCTP_SRC);
		config->write_setfields &= ~(1UL << OF1X_MATCH_SCTP_SRC);
		
		config->match &= ~(1UL << OF1X_MATCH_SCTP_DST);
		config->wildcards &= ~(1UL << OF1X_MATCH_SCTP_DST);
		config->apply_setfields &= ~(1UL << OF1X_MATCH_SCTP_DST);
		config->write_setfields &= ~(1UL << OF1X_MATCH_SCTP_DST);
		
		config->match &= ~(1UL << OF1X_MATCH_IPV6_EXTHDR);
		config->wildcards &= ~(1UL << OF1X_MATCH_IPV6_EXTHDR);
		config->apply_setfields &= ~(1UL << OF1X_MATCH_IPV6_EXTHDR);
		config->write_setfields &= ~(1UL << OF1X_MATCH_IPV6_EXTHDR);
	}

	return ROFL_SUCCESS;
}

rofl_result_t platform_pre_destroy_of1x_switch(of1x_switch_t* sw){

	delete (datapacket_storage*)sw->platform_state;
	return ROFL_SUCCESS;
}


//Async notifications

/*
* Packet in
*/


void platform_of1x_packet_in(const of1x_switch_t* sw, uint8_t table_id, datapacket_t* pkt, of_packet_in_reason_t reason)
{
	datapacket_t* pkt_replica;

	//Protect
	if(unlikely(pkt==NULL) || unlikely(sw==NULL))
		return;

	//Replicate packet but not mbuf:
	pkt_replica = platform_packet_replicate__(pkt, false);
	
	if(unlikely(!pkt_replica)){
		ROFL_DEBUG("Replicate packet(PKT_IN); could not clone pkt(%p). Dropping...\n");
		goto PKT_IN_ERROR;
	}

	ROFL_DEBUG("Trying to enqueue PKT_IN for packet(%p), switch %p\n", pkt, sw);
	
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
	}
	if(((datapacket_dpdk_t*)pkt->platform_state)->mbuf){
		rte_pktmbuf_free(((datapacket_dpdk_t*)pkt->platform_state)->mbuf);
	}
	return;

}

//Flow removed
void platform_of1x_notify_flow_removed(const of1x_switch_t* sw, 	
						of1x_flow_remove_reason_t reason, 
						of1x_flow_entry_t* removed_flow_entry){

	ROFL_INFO(" calling %s()\n",__FUNCTION__);
	cmm_process_of1x_flow_removed(sw->dpid, (uint8_t)reason, removed_flow_entry);

}


void plaftorm_of1x_add_entry_hook(of1x_flow_entry_t* new_entry){

}

void platform_of1x_modify_entry_hook(of1x_flow_entry_t* old_entry, of1x_flow_entry_t* mod, int reset_count){

}

void platform_of1x_remove_entry_hook(of1x_flow_entry_t* entry){

}

void platform_of1x_update_stats_hook(of1x_flow_entry_t* entry) {

}
