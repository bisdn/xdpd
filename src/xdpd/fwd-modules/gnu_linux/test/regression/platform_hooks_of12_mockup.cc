/*
 * platform_hooks_of12.c
 *
 *  Created on: Feb 7, 2013
 *      Author: tobi
 */

#include <assert.h>
#include <rofl/datapath/pipeline/openflow/openflow12/of12_async_events_hooks.h>
#include <rofl/datapath/pipeline/openflow/openflow12/of12_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_flow_table.h>
#include <rofl/datapath/afa/openflow/openflow12/of12_cmm.h>


#include "io/datapacketx86.h"
#include "io/datapacketx86_c_wrapper.h"
#include "io/datapacket_storage_c_wrapper.h"
#include "io/bufferpool.h"
#include "ls_internal_state.h"


#define DATAPACKET_STORE_EXPIRATION_TIME 60

/*
* Hooks for configuration of the switch
*/
rofl_result_t platform_post_init_of12_switch(of12_switch_t* sw){
	//Set OF12 flow table 
	unsigned int i;
	of12_flow_table_t* table;

	table = sw->pipeline->tables; 
	
	for(i=0;i<sw->pipeline->num_of_tables;++i){
		//Set appropiate flags
		table->default_action = OF12_TABLE_MISS_CONTROLLER;
	}
	
	//Create GNU/Linux FWD_Module additional state (platform state)
	struct logical_switch_internals* ls_int = (struct logical_switch_internals*)calloc(1, sizeof(struct logical_switch_internals));

	ls_int->ringbuffer = new_ringbuffer();
	ls_int->store_handle = create_datapacket_store(1024, DATAPACKET_STORE_EXPIRATION_TIME); // todo make this value configurable

	sw->platform_state = (of_switch_platform_state_t*)ls_int;
	
	return ROFL_SUCCESS;
}

rofl_result_t platform_pre_destroy_of12_switch(of12_switch_t* sw){

	//delete ring buffers and storage (delete switch platform state)
	delete_ringbuffer(((struct logical_switch_internals*)sw->platform_state)->ringbuffer);
	destroy_datapacket_store(((struct logical_switch_internals*)sw->platform_state)->store_handle);
	free(sw->platform_state);
	
	return ROFL_SUCCESS;
}

/*
* Packet in
*/

void platform_of12_packet_in(const of12_switch_t* sw, uint8_t table_id, datapacket_t* pkt, of_packet_in_reason_t reason)
{
	assert(OF_VERSION_12 == sw->of_ver);

	//This is a mockup prevent out of buffers 
	bufferpool::release_buffer(pkt);
	
}

//Flow removed
void platform_of12_notify_flow_removed(const of12_switch_t* sw, 	
						of12_flow_remove_reason_t reason, 
						of12_flow_entry_t* removed_flow_entry){

}


