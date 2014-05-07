#include "bg_taskmanager.h"

#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <algorithm>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/hal/driver.h>
#include <rofl/datapath/hal/cmm.h>
#include "io/bufferpool.h"
#include "io/iface_manager.h"
#include "io/datapacket_storage.h"
#include "util/time_utils.h"

using namespace xdpd::gnu_linux;

//Local static variable for background manager thread
static pthread_t bg_thread;
static bool bg_continue_execution = true;

/**
 * This piece of code is meant to manage a thread that does:
 * 
 * - the expiration of the flow entries.
 * - the update the status of the ports
 * - purge old buffers in the buffer storage of a logical switch(pkt-in) 
 * - more?
 */

/**
 * @name process_timeouts
 * @brief checks if its time to process timeouts (flow entries and datapacket storage)
 * @param psw physical switch (where all the logical switches are)
 */
int process_timeouts(){

	datapacket_t* pkt;
	unsigned int i, max_switches;
	struct timeval now;
	of_switch_t** logical_switches;
	static struct timeval last_time_entries_checked={0,0}, last_time_pool_checked={0,0};
	gettimeofday(&now,NULL);

	//Retrieve the logical switches list
	logical_switches = physical_switch_get_logical_switches(&max_switches);
	
	if(get_time_difference_ms(&now, &last_time_entries_checked)>=LSW_TIMER_SLOT_MS)
	{
#ifdef DEBUG
		static int dummy = 0;
#endif

		//TIMERS FLOW ENTRIES
		for(i=0; i<max_switches; i++)
		{

			if(logical_switches[i] != NULL){
				of_process_pipeline_tables_timeout_expirations(logical_switches[i]);
				
#ifdef DEBUG
				if(dummy%20 == 0)
					of1x_full_dump_switch((of1x_switch_t*)logical_switches[i], false);
#endif
			}
		}
			
#ifdef DEBUG
		dummy++;
		//ROFL_DEBUG_VERBOSE(DRIVER_NAME"[bg] Checking flow entries expirations %lu:%lu\n",now.tv_sec,now.tv_usec);
#endif
		last_time_entries_checked = now;
	}
	
	if(get_time_difference_ms(&now, &last_time_pool_checked)>=LSW_TIMER_BUFFER_POOL_MS){
		uint32_t buffer_id;
		datapacket_storage* dps=NULL;
		
		for(i=0; i<max_switches; i++){

			if(logical_switches[i] != NULL){

				//Recover storage pointer
				dps = (datapacket_storage*)logical_switches[i]->platform_state;

				//Loop until the oldest expired packet is taken out
				while(dps->oldest_packet_needs_expiration(&buffer_id)){

					ROFL_DEBUG_VERBOSE(DRIVER_NAME"[bg] Trying to erase a datapacket from storage: %u\n", buffer_id);

					if( (pkt = dps->get_packet(buffer_id) ) == NULL ){
						ROFL_DEBUG_VERBOSE(DRIVER_NAME"[bg] Error in get_packet_wrapper %u\n", buffer_id);
					}else{
						ROFL_DEBUG_VERBOSE(DRIVER_NAME"[bg] Datapacket expired correctly %u\n", buffer_id);
						//Return mbuf to the pool
						rte_pktmbuf_free(((datapacket_dpdk_t*)pkt->platform_state)->mbuf);
						//Return buffer to bufferpool
						bufferpool::release_buffer(pkt);
					}
				}
			}
		}
		
#ifdef DEBUG
		//ROFL_ERR(DRIVER_NAME"[bg] Checking pool buffers expirations %lu:%lu\n",now.tv_sec,now.tv_usec);
#endif
		last_time_pool_checked = now;
	}
	
	return ROFL_SUCCESS;
}

/**
 * @name x86_background_tasks_thread
 * @brief contents the infinite loop checking for ports and timeouts
 */
void* x86_background_tasks_routine(void* param){

	static struct timeval last_time_stats_updated={0,0}, last_time_links_updated={0,0}, now;

	while(bg_continue_execution){
		
		gettimeofday(&now,NULL);
		
		//check timers expiration 
		process_timeouts();
		
		//Update link state(s)
		if(get_time_difference_ms(&now, &last_time_links_updated)>=BG_UPDATE_PORT_LINKS_MS){
			iface_manager_update_links();
			last_time_links_updated = now;
		}
	
		//Update port stats
		if(get_time_difference_ms(&now, &last_time_stats_updated)>=BG_UPDATE_PORT_STATS_MS){
			iface_manager_update_stats();
			last_time_stats_updated = now;
		}
			
		//Throttle
		usleep(LSW_TIMER_SLOT_MS*1000);
	}
	
	//Printing some information
	ROFL_DEBUG(DRIVER_NAME"[bg] Finishing thread execution\n"); 

	//Exit
	pthread_exit(NULL);	
}

/**
 * launches the main thread
 */
rofl_result_t launch_background_tasks_manager(){
	//Set flag
	bg_continue_execution = true;

	if(pthread_create(&bg_thread, NULL, x86_background_tasks_routine,NULL)<0){
		ROFL_ERR(DRIVER_NAME"[bg] pthread_create failed, errno(%d): %s\n", errno, strerror(errno));
		return ROFL_FAILURE;
	}
	return ROFL_SUCCESS;
}

rofl_result_t stop_background_tasks_manager(){
	bg_continue_execution = false;
	pthread_join(bg_thread,NULL);
	return ROFL_SUCCESS;
}
