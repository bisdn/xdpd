#include <assert.h>
#include <unistd.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_async_events_hooks.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_table.h>
#include <rofl/datapath/hal/openflow/openflow1x/of1x_cmm.h>
#include <rofl/common/utils/c_logger.h>

#include "../config.h"
#include "../io/bufferpool.h"
#include "../io/datapacketx86.h"
#include "../io/datapacket_storage.h"
#include "../processing/ls_internal_state.h"
#include "../io/pktin_dispatcher.h"

//Time measurements
#include "../util/time_measurements.h"

using namespace xdpd::gnu_linux;

/*
* Hooks for configuration of the switch
*/
rofl_result_t platform_post_init_of1x_switch(of1x_switch_t* sw){
	unsigned int i;
	
	//Create GNU/Linux FWD_Module additional state (platform state)
	switch_platform_state_t* ls_int = (switch_platform_state_t*)calloc(1, sizeof(switch_platform_state_t));

	//Create input queues
	for(i=0;i<IO_RX_THREADS_PER_LSI;i++){
		ls_int->input_queues[i] = new circular_queue<datapacket_t>(PROCESSING_INPUT_QUEUE_SLOTS);
	}

	ls_int->pkt_in_queue = new circular_queue<datapacket_t>(PROCESSING_PKT_IN_QUEUE_SLOTS);
	ls_int->storage = new datapacket_storage( IO_PKT_IN_STORAGE_MAX_BUF, IO_PKT_IN_STORAGE_EXPIRATION_S); // todo make this value configurable

	sw->platform_state = (of_switch_platform_state_t*)ls_int;

	//Set number of buffers
	sw->pipeline.num_of_buffers = IO_PKT_IN_STORAGE_MAX_BUF;
	
	//Set the actions and matches supported by this platform
	for(i=0; i<sw->pipeline.num_of_tables; i++){
		of1x_flow_table_config_t *config = &(sw->pipeline.tables[i].config);
	
		//Lets set to zero the unssuported matches and actions.
		bitmap128_unset(&config->apply_actions, OF1X_AT_COPY_TTL_OUT);
		bitmap128_unset(&config->write_actions, OF1X_AT_COPY_TTL_OUT);
		
		bitmap128_unset(&config->match, OF1X_MATCH_SCTP_SRC);
		bitmap128_unset(&config->wildcards, OF1X_MATCH_SCTP_SRC);
		
		bitmap128_unset(&config->match, OF1X_MATCH_SCTP_DST);
		bitmap128_unset(&config->wildcards, OF1X_MATCH_SCTP_DST);
		
		bitmap128_unset(&config->match, OF1X_MATCH_IPV6_EXTHDR);
		bitmap128_unset(&config->wildcards, OF1X_MATCH_IPV6_EXTHDR);

		//TODO: PBB, TUNNEL_ID zero them when they are set to 1 in ROFL_pipeline
	}

	return ROFL_SUCCESS;
}

rofl_result_t platform_pre_destroy_of1x_switch(of1x_switch_t* sw){
	
	unsigned int i;

	switch_platform_state_t* ls_int =  (switch_platform_state_t*)sw->platform_state;

	//There should NOT be any PKT_INs pending
	if(ls_int->pkt_in_queue->size() != 0)
		assert(0);	
	
	//Delete ring buffers and storage (delete switch platform state)
	for(i=0;i<IO_RX_THREADS_PER_LSI;i++){
		delete ls_int->input_queues[i]; 
	}
	delete ls_int->pkt_in_queue;
	delete ls_int->storage;
	free(sw->platform_state);
	
	return ROFL_SUCCESS;
}


//Async notifications

/*
* Packet in
*/

void platform_of1x_packet_in(const of1x_switch_t* sw, uint8_t table_id, datapacket_t* pkt, uint16_t send_len, of_packet_in_reason_t reason)
{
	datapacketx86* pkt_x86;
	switch_platform_state_t* ls_state = (switch_platform_state_t*)sw->platform_state;

	ROFL_DEBUG(DRIVER_NAME" Enqueuing PKT_IN event for packet(%p) in switch: %s\n",pkt,sw->name);
	
	//Recover platform state and fill it so that state can be recovered afterwards
	pkt_x86 = (datapacketx86*)pkt->platform_state;
	pkt_x86->pktin_table_id = table_id;
	pkt_x86->pktin_reason = reason;
	pkt_x86->pktin_send_len = send_len;
	
	//Timestamp SB6_PRE	
	TM_STAMP_STAGE(pkt, TM_SB5_PRE);
		
	//Enqueue
	if( ls_state->pkt_in_queue->non_blocking_write(pkt) == ROFL_SUCCESS ){
		//Notify
		notify_packet_in();
			
		//Timestamp SB6_SUCCESS	
		TM_STAMP_STAGE(pkt, TM_SB5_SUCCESS);
	}else{
		ROFL_DEBUG(DRIVER_NAME" PKT_IN for packet(%p) could not be sent for sw:%s (PKT_IN queue full). Dropping..\n",pkt,sw->name);
		//Return to the bufferpool
		bufferpool::release_buffer(pkt);

		//Timestamp SB6_FAILURE
		TM_STAMP_STAGE(pkt, TM_SB5_FAILURE);
	}
}

//Flow removed
void platform_of1x_notify_flow_removed(const of1x_switch_t* sw, 	
						of1x_flow_remove_reason_t reason, 
						of1x_flow_entry_t* removed_flow_entry){

	hal_cmm_process_of1x_flow_removed(sw->dpid, (uint8_t)reason, removed_flow_entry);

}


void plaftorm_of1x_add_entry_hook(of1x_flow_entry_t* new_entry){

}

void platform_of1x_modify_entry_hook(of1x_flow_entry_t* old_entry, of1x_flow_entry_t* mod, int reset_count){

}

void platform_of1x_remove_entry_hook(of1x_flow_entry_t* entry){

}

void
platform_of1x_update_stats_hook(of1x_flow_entry_t* entry)
{

}
