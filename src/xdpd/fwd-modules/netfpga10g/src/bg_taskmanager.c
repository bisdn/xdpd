#include "bg_taskmanager.h"

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

#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/datapath/afa/cmm.h>

#include "netfpga/netfpga.h"
#include "netfpga/ports.h"
#include "io/packet_io.h"

//Local static variable for background manager thread
static pthread_t bg_thread;
static bool bg_continue_execution = true;
/**
 * This piece of code is meant to manage a thread that is support for:
 * 
 * - execute the expiration of the flow entries.
 * - TODO: update the status of the ports
 * - Get the packet_in events from the kernel and send appropiate OF messages
 * - TODO: free space in the buffer pool when a buffer is too old
 * - more?
 */

/**
 * @name get_time_difference_ms
 * @brief returns the time difference between 2 timeval structs in ms
 * @param now latest time
 * @param last oldest time
 */
uint64_t get_time_difference_ms(struct timeval *now, struct timeval *last)
{
	/*diff = now -last; now > last !!*/
	struct timeval res;
	timersub(now,last,&res);
	
	return res.tv_sec * 1000 + res.tv_usec/1000;
}

/**
 * @name update_port_status
 */
rofl_result_t update_port_status(char * name){
	//TODO!	

	return ROFL_SUCCESS;
}

/**
 * @name process_timeouts
 * @brief checks if its time to process timeouts (flow entries and pool of buffers)
 * @param psw physical switch (where all the logical switches are)
 */
int process_timeouts()
{
	unsigned int i, max_switches;
	struct timeval now;
	of_switch_t** logical_switches;
	static struct timeval last_time_entries_checked={0,0} /*, last_time_pool_checked={0,0}*/;
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
					of12_full_dump_switch((of12_switch_t*)logical_switches[i]);
#endif
			}
		}
			
#ifdef DEBUG
		dummy++;
		//ROFL_DEBUG_VERBOSE("<%s:%d> Checking flow entries expirations %lu:%lu\n",__func__,__LINE__,now.tv_sec,now.tv_usec);
#endif
		last_time_entries_checked = now;
	}
#if 0
//DISABLED!	
	if(get_time_difference_ms(&now, &last_time_pool_checked)>=LSW_TIMER_BUFFER_POOL_MS){
		uint32_t buffer_id;
		datapacket_store_handle *dps=NULL;
		
		for(i=0; i<max_switches; i++){

			if(logical_switches[i] != NULL){

				dps = ( (struct logical_switch_internals*) logical_switches[i]->platform_state)->store_handle ;
				//TODO process buffers in the storage
				while(datapacket_storage_oldest_packet_needs_expiration_wrapper(dps,&buffer_id)){

					ROFL_DEBUG_VERBOSE("<%s:%d> trying to erase a datapacket from storage\n",__func__,__LINE__);

					if(datapacket_storage_get_packet_wrapper(dps,buffer_id)==NULL){
						ROFL_DEBUG_VERBOSE("<%s:%d> Error in get_packet_wrapper\n",__func__,__LINE__);
					}else{
						ROFL_DEBUG_VERBOSE("<%s:%d> datapacket expired correctly\n",__func__,__LINE__);
					}
				}
			}
		}
		
#ifdef DEBUG
		//ROFL_ERR("<%s:%d> Checking pool buffers expirations %lu:%lu\n",__func__,__LINE__,now.tv_sec,now.tv_usec);
#endif
		last_time_pool_checked = now;
	}
#endif	
	return ROFL_SUCCESS;
}

/**
 * @name x86_background_tasks_thread
 * @brief contents the infinite loop checking for ports and timeouts
 */
void* x86_background_tasks_routine(void* param)
{
	int i, efd, /*events_socket,*/ nfds;
	struct epoll_event event_list[MAX_EPOLL_EVENTS], port_events[NETFPGA_NUM_PORTS];
	char iface_name[NETFPGA_INTERFACE_NAME_LEN] = ""; //nfX\0
	switch_port_t* port;

	// program an epoll that listents to the file descriptors of the ports with a
	// timeout that makes us check 
	
	memset(event_list,0,sizeof(event_list));
	memset(&port_events,0,sizeof(port_events));
	
	efd = epoll_create1(0);

	if(efd == -1){
		ROFL_ERR("<%s:%d> Error in epoll_create1\n",__func__,__LINE__);
		return NULL;
	}
	
	//Add the 4 NetFPGA ports
	for(i=0; i< NETFPGA_NUM_PORTS; ++i){
		//Compose name nf0...nf3
		snprintf(iface_name, NETFPGA_INTERFACE_NAME_LEN, NETFPGA_INTERFACE_BASE_NAME"%d", i);
			
		//Recover port from pipeline
		port = physical_switch_get_port_by_name(iface_name);

		if( !port )
			exit(EXIT_FAILURE);		
		
		//Maintain reference back
		port_events[i].data.ptr = (void*)port;

		if( epoll_ctl(efd,EPOLL_CTL_ADD, ((netfpga_port_t*)port->platform_port_state)->fd, &port_events[i]) ==-1 ){
			ROFL_ERR("<%s:%d> Error in epoll_ctl\n",__func__,__LINE__);
			exit(EXIT_FAILURE);		
		}
	}
	
	while(bg_continue_execution){
		
		//Throttle
		nfds = epoll_wait(efd, event_list, MAX_EPOLL_EVENTS, LSW_TIMER_SLOT_MS/*timeout needs TBD somewhere else*/);


		if(nfds==-1){
			ROFL_DEBUG("<%s:%d> Epoll Failed\n",__func__,__LINE__);
			continue;
		}

		if(nfds==0){
			//TIMEOUT PASSED
			process_timeouts();
		}

		for(i=0;i<nfds;i++){
			
			if( (event_list[i].events & EPOLLERR) || (event_list[i].events & EPOLLHUP)/*||(event_list[i].events & EPOLLIN)*/){
				//error on this fd
				ROFL_ERR("<%s:%d> Error in file descriptor\n",__func__,__LINE__);
				
				close(((netfpga_port_t*)event_list[i].data.ptr)->fd); //fd gets removed automatically from efd's
				continue;
			}else{
				//something is going on with the i/o system
				netpfga_io_read_from_port(((switch_port_t*)event_list[i].data.ptr));
			}
			//check if there is a need of manage timers!
			process_timeouts();
		}
	}
	
	//Cleanup
	close(efd);
	
	//Printing some information
	ROFL_INFO("[bg] Finishing thread execution\n"); 

	//Exit
	pthread_exit(NULL);	
}

/**
 * launches the main thread
 */
rofl_result_t launch_background_tasks_manager()
{
	//Set flag
	bg_continue_execution = true;
	
	if(pthread_create(&bg_thread, NULL, x86_background_tasks_routine,NULL)<0){
		ROFL_ERR("<%s:%d> pthread_create failed\n",__func__,__LINE__);
		return ROFL_FAILURE;
	}
	return ROFL_SUCCESS;
}

rofl_result_t stop_background_tasks_manager()
{
	bg_continue_execution = false;
	pthread_join(bg_thread,NULL);
	return ROFL_SUCCESS;
}
