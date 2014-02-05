#include "bg_taskmanager.h"

#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
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
#include "netfpga/stats.h"
#include "netfpga/flow_entry.h"
#include "io/packet_io.h"
#include "io/bufferpool.h"
#include "netfpga/regs.h"
#include "pipeline-imp/ls_internal_state.h"
#include "util/time_utils.h"


using namespace xdpd::gnu_linux;

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

void update_misc_stats(){
	
	uint32_t misc_stats[NETFPGA_NUMBER_OF_MISC_STATS];
	netfpga_read_misc_stats(misc_stats);
	//displacy_misc_stats(misc_stats);

	//conberting netfpga stats to port stats structure

	switch_port_t* port[4];
	port[0] = physical_switch_get_port_by_name("nf0");
	port[1] = physical_switch_get_port_by_name("nf1");
	port[2] = physical_switch_get_port_by_name("nf2");
	port[3] = physical_switch_get_port_by_name("nf3");
	
	for(int i=0;i<NETFPGA_LAST_PORT-1;i++){		

		port[i]->stats.rx_packets=(uint64_t)misc_stats[i+0x09]+(uint64_t)misc_stats[i+0x13]+(uint64_t)misc_stats[i+0x18];/* Number of received packets. */
		port[i]->stats.tx_packets=(uint64_t)misc_stats[i+0x0e];     /* Number of transmitted packets. */
		port[i]->stats.rx_dropped=(uint64_t)misc_stats[i];     /* Number of packets dropped by RX. */

	}

}

int process_timeouts()
{
	datapacket_t* pkt;
	unsigned int i, max_switches;
	struct timeval now;
	of_switch_t** logical_switches;
	static struct timeval last_time_entries_checked={0,0} , last_time_pool_checked={0,0};
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
					of1x_full_dump_switch((of1x_switch_t*)logical_switches[i]);
#endif
			}
		}
			
#ifdef DEBUG
		dummy++;
		//ROFL_DEBUG_VERBOSE("<%s:%d> Checking flow entries expirations %lu:%lu\n",__func__,__LINE__,now.tv_sec,now.tv_usec);
#endif
		last_time_entries_checked = now;
	}
	
	if(get_time_difference_ms(&now, &last_time_pool_checked)>=LSW_TIMER_BUFFER_POOL_MS){
		uint32_t buffer_id;
		datapacket_storage *dps=NULL;
		
		for(i=0; i<max_switches; i++){

			if(logical_switches[i] != NULL){

				dps = ( (struct logical_switch_internals*) logical_switches[i]->platform_state)->storage;
				//TODO process buffers in the storage
				while(dps->oldest_packet_needs_expiration(&buffer_id)){

					ROFL_DEBUG_VERBOSE("<%s:%d> trying to erase a datapacket from storage\n",__func__,__LINE__);
					if( (pkt = dps->get_packet(buffer_id) ) == NULL ){
						ROFL_DEBUG_VERBOSE("Error in get_packet_wrapper %u\n", buffer_id);
					}else{
						ROFL_DEBUG_VERBOSE("Datapacket expired correctly %u\n", buffer_id);
						//Return buffer to bufferpool
						bufferpool::release_buffer(pkt);
					}

				}
			}
		}
		
#ifdef DEBUG
		//ROFL_ERR("<%s:%d> Checking pool buffers expirations %lu:%lu\n",__func__,__LINE__,now.tv_sec,now.tv_usec);
#endif
		last_time_pool_checked = now;
	}

	


	return ROFL_SUCCESS;
}

/**
 * @name x86_background_tasks_thread
 * @brief contents the infinite loop checking for ports and timeouts
 */
void* x86_background_tasks_routine(void* param)
{
	int i, efd, nfds; /*events_socket,*/ //nfds
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
	




	fd_set	handle_set;

	FD_ZERO (&handle_set);
       


	//Add the 4 NetFPGA ports
	for(i=0; i< NETFPGA_NUM_PORTS; ++i){
		//Compose name nf0...nf3
		snprintf(iface_name, NETFPGA_INTERFACE_NAME_LEN, NETFPGA_INTERFACE_BASE_NAME"%d", i);


		//ROFL_DEBUG("interface name %s ", iface_name );
	
		//Recover port from pipeline
		port = physical_switch_get_port_by_name(iface_name);
		port_events[i].events = EPOLLIN | EPOLLET; //read and edge trigered

		if( !port )
			exit(EXIT_FAILURE);		
		
		//Maintain reference back
		port_events[i].data.ptr = (void*)port;
				
		if( epoll_ctl(efd, EPOLL_CTL_ADD, ((netfpga_port_t*)port->platform_port_state)->fd, &port_events[i]) < 0  ){
			ROFL_ERR("<%s:%d> Error in epoll_ctl\n",__func__,__LINE__);
			exit(EXIT_FAILURE);		
		}
		
		



		


	}

	//FIXME: add the NETLINK stuff
	
	while(bg_continue_execution){
		update_misc_stats();

/////////////////////////////////////////////////////////////TEST update_entry_stats function///////////////////////////////
/*
	netfpga_flow_entry_t* hw_entry;
	hw_entry = netfpga_init_flow_entry();
	

	for (i = 0; i < 32; ++i) {
		hw_entry = netfpga_init_flow_entry();

		memset(hw_entry,0x00,sizeof(netfpga_flow_entry_t));	

		hw_entry->hw_pos = i; 	
		
		hw_entry->type = NETFPGA_FE_WILDCARDED; 
		
		of1x_flow_entry_t* of1x_entry=new of1x_flow_entry_t;
		memset(of1x_entry,0,sizeof(of1x_flow_entry_t));
		of1x_entry->platform_state=(of1x_flow_entry_platform_state_t*)hw_entry;
		ROFL_DEBUG("\n entry number %d", i);
		netfpga_update_entry_stats(of1x_entry);

		netfpga_destroy_flow_entry(hw_entry);
		delete  of1x_entry;
	} 

*/			
//////////////////////////////////////////////////////////////END OF TEST/////////////////////////////////////////////////////////////			




	
		//Throttle
		//update_misc_stats();
		
		nfds = epoll_wait(efd, event_list, MAX_EPOLL_EVENTS, LSW_TIMER_SLOT_MS  /*temporaly changet to infinite*/ /*timeout needs TBD somewhere else*/);
		//ROFL_DEBUG(" After epoll_wait \n\n\n\n");


		if(nfds==-1){ //ERROR in select
			ROFL_DEBUG("<%s:%d> Epoll Failed\n",__func__,__LINE__);
			continue;
		}

		if(nfds==0){ 
			//TIMEOUT PASSED
			//ROFL_DEBUG("bg_taskmanager epoll gave 0 - TIMEOUT PASSED");
			process_timeouts();
		}
		
		//ROFL_DEBUG("bg_taskmanager epoll gave %d \n \n", nfds );
	
		for(i=0;i<nfds;i++){
			
		
			
			if( (event_list[i].events & EPOLLERR) || (event_list[i].events & EPOLLHUP)){
				//error on this fd
				ROFL_ERR("<%s:%d> Error in file descriptor\n",__func__,__LINE__);
				
				close(((netfpga_port_t*)event_list[i].data.ptr)->fd); //fd gets removed automatically from efd's
				continue;
			}else{
				
				//ROFL_DEBUG("event_list[i].data.ptr %p \n",event_list[i].data.ptr);
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




