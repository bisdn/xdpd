#include "stats.h"
#include "regs.h"
#include "flow_entry.h"


rofl_result_t netfpga_update_entry_stats(of1x_flow_entry_t* entry){
	
	uint32_t* aux;	
	netfpga_device_t* nfpga= netfpga_get();
	netfpga_flow_entry_t* hw_entry;
	netfpga_flow_entry_stats_t stats;

	if(!entry)
		return ROFL_FAILURE;

	//Recover the hw entry 
	hw_entry = (netfpga_flow_entry_t*) entry->platform_state;
	
	if(!hw_entry)
		return ROFL_FAILURE;
	
	//Wait for the netfpga to be ready
	netfpga_wait_reg_ready(nfpga);

	//Clear stats -> Really necessary?
	memset(&stats,0,sizeof(stats));

	//Write command
	if( hw_entry->type == NETFPGA_FE_WILDCARDED )
		netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_WILDCARD_BASE + hw_entry->hw_pos);
	else
		netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_EXACT_BASE + hw_entry->hw_pos);
	

	//Write whatever => Trigger command
	netfpga_write_reg(nfpga, NETFPGA_OF_READ_ORDER_REG, 0x1); // Write whatever the value

	//Wait for the netfpga to be ready (again!)
	netfpga_wait_reg_ready(nfpga);

	//Now read and fill
	aux = (uint32_t*)&stats;
	netfpga_read_reg(nfpga, NETFPGA_OF_STATS_BASE_REG,aux);
	netfpga_read_reg(nfpga, NETFPGA_OF_STATS_BASE_REG+1,(aux+1));

	//Update main entry and return
	
	entry->stats.s.counters.packet_count = stats.pkt_counter;  
	entry->stats.s.counters.byte_count = stats.byte_counter;  
	//XDPD_DEBUG("\n entry stats: %x, %x",stats.pkt_counter,stats.byte_counter );


	//TODO time?? last-seen 7 bit??

	return ROFL_SUCCESS;	
}








rofl_result_t netfpga_read_misc_stats(uint32_t misc_stats[]){


	
	unsigned int 	i;
	netfpga_device_t* nfpga= netfpga_get();


	for (i=0;i<NETFPGA_NUMBER_OF_MISC_STATS;i++)
		{
		netfpga_read_reg(nfpga, NETFPGA_OF_BASE_ADDR+i,&(misc_stats[i]));
		
		//XDPD_DEBUG("\n stat: %x",misc_stats[i]);
		
		
		}

	return ROFL_SUCCESS;	
}



void displacy_misc_stats(uint32_t misc_stats[]){ //created for debuging
XDPD_DEBUG("\n Current stats are: ");
XDPD_DEBUG("\n pkt dropped from port 0: 	%x",misc_stats[0]);
XDPD_DEBUG("\n pkt dropped from port 1: 	%x",misc_stats[1]);
XDPD_DEBUG("\n pkt dropped from port 2: 	%x",misc_stats[2]);
XDPD_DEBUG("\n pkt dropped from port 3: 	%x",misc_stats[3]);
XDPD_DEBUG("\n pkt dropped from port PC: 	%x",misc_stats[4]);
XDPD_DEBUG("\n exact hit:		 	%x",misc_stats[5]);
XDPD_DEBUG("\n exact miss: 			%x",misc_stats[6]);
XDPD_DEBUG("\n wildcard hit:		 	%x",misc_stats[7]);
XDPD_DEBUG("\n wildcard miss: 			%x",misc_stats[8]);
XDPD_DEBUG("\n pkt parsed as L2 frame from port 0:	%x",misc_stats[9]);
XDPD_DEBUG("\n pkt parsed as L2 frame from port 1: 	%x",misc_stats[10]);
XDPD_DEBUG("\n pkt parsed as L2 frame from from port 2: %x",misc_stats[11]);
XDPD_DEBUG("\n pkt parsed as L2 frame from from port 3: %x",misc_stats[12]);
XDPD_DEBUG("\n pkt parsed as L2 frame from from PC port:%x",misc_stats[13]);
XDPD_DEBUG("\n pkt processed and forwarded from port 0: %x",misc_stats[14]);
XDPD_DEBUG("\n pkt processed and forwarded from port 1: %x",misc_stats[15]);
XDPD_DEBUG("\n pkt processed and forwarded from port 2: %x",misc_stats[16]);
XDPD_DEBUG("\n pkt processed and forwarded from port 3: %x",misc_stats[17]);
XDPD_DEBUG("\n pkt processed and forwarded from PC port:%x",misc_stats[18]);
XDPD_DEBUG("\n pkt parsed as ARP from port 0: 		%x",misc_stats[19]);
XDPD_DEBUG("\n pkt parsed as ARP from port 1: 		%x",misc_stats[20]);
XDPD_DEBUG("\n pkt parsed as ARP from port 2: 		%x",misc_stats[21]);
XDPD_DEBUG("\n pkt parsed as ARP from port 3: 		%x",misc_stats[22]);
XDPD_DEBUG("\n pkt parsed as ARP from PC port: 		%x",misc_stats[23]);
XDPD_DEBUG("\n pkt parsed as IP, TCP/IP, UDP/IP from port 0: 	%x",misc_stats[24]);
XDPD_DEBUG("\n pkt parsed as IP, TCP/IP, UDP/IP from port 1: 	%x",misc_stats[25]);
XDPD_DEBUG("\n pkt parsed as IP, TCP/IP, UDP/IP from port 2: 	%x",misc_stats[26]);
XDPD_DEBUG("\n pkt parsed as IP, TCP/IP, UDP/IP from port 3: 	%x",misc_stats[27]);
XDPD_DEBUG("\n pkt parsed as IP, TCP/IP, UDP/IP from PC port: 	%x",misc_stats[28]);




}



rofl_result_t netfpga_clean_misc_stats(){

	unsigned int 	i;
	netfpga_device_t* nfpga= netfpga_get();
	uint32_t value;
	memset(&value,0x00,sizeof(value));

	for (i=0;i<NETFPGA_NUMBER_OF_MISC_STATS;i++){
		if (netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR+i,value)!= ROFL_SUCCESS) {
			XDPD_DEBUG("\n Failled on cleanning stat number: 	%x",i);
			return ROFL_FAILURE;
		}
		
	}
	XDPD_DEBUG("\n MISC STATS CLEANED!! ");
	return ROFL_SUCCESS;	
}



