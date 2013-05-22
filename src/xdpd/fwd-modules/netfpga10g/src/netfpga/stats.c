#include "stats.h"
#include "regs.h"
#include "flow_entry.h"


rofl_result_t netfpga_update_entry_stats(of12_flow_entry_t* entry){
	
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
	entry->stats.packet_count = stats.pkt_counter;  
	entry->stats.byte_count = stats.byte_counter;  
	
	//TODO time?? last-seen 7 bit??

	return ROFL_SUCCESS;	
}
