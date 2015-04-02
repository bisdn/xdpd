#include "netfpga.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <rofl/common/utils/c_logger.h>
#include <assert.h>

#include "regs.h"
#include "flow_entry.h"

#define NETFPGA_DEVNAME "/dev/nf10"

//Pointer to the unique (static) netfpga_device_t
static netfpga_device_t* nfpga=NULL;


/*
 * Internal HW stuff
 */

//Dump wildcard entries
rofl_result_t netfpga_dump_wildcard_hw_entries(){

       unsigned int i,j;
       uint32_t* aux;
       netfpga_flow_entry_t* entry;

       //Create an empty entry
       entry = netfpga_init_flow_entry();      
       
       //Wait for the netfpga to be ready
       netfpga_wait_reg_ready(nfpga);

       //Loop over all entries
       for(j =0; j < NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE; j++){

               entry->hw_pos =j;

               //PUT something strange
               memset(entry->matches, 0xEE, sizeof(*(entry->matches)));
               memset(entry->masks, 0xDD, sizeof(*(entry->masks)));
               memset(entry->actions, 0xCC, sizeof(*(entry->actions)));
               
               //Set Row address
               if(netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_WILDCARD_BASE + j) != ROFL_SUCCESS)
                       return ROFL_FAILURE;
       
               //Write whatever => Trigger read
               if(netfpga_write_reg(nfpga, NETFPGA_OF_READ_ORDER_REG, 0x1) != ROFL_SUCCESS)
                       return ROFL_FAILURE;

       
               //Read matches
               aux = (uint32_t*)entry->matches;
               for (i = 0; i < NETFPGA_FLOW_ENTRY_MATCHES_WORD_LEN; ++i) {
                       if(netfpga_read_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_BASE_REG + i, (aux+i)) != ROFL_SUCCESS)
                               return ROFL_FAILURE;
               }

               //Read masks
               aux = (uint32_t*)entry->masks;
               for (i = 0; i < NETFPGA_FLOW_ENTRY_WILDCARD_WORD_LEN; ++i) {
                       if(netfpga_read_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_MASK_BASE_REG + i, (aux+i)) != ROFL_SUCCESS)
                               return ROFL_FAILURE;
               }
       
               //Read actions
               aux = (uint32_t*)entry->actions;
               for (i = 0; i < NETFPGA_FLOW_ENTRY_ACTIONS_WORD_LEN; ++i) {
                       if(netfpga_read_reg(nfpga, NETFPGA_OF_LOOKUP_ACTION_BASE_REG + i, (aux+i)) != ROFL_SUCCESS)     
                               return ROFL_FAILURE;
               }
       
               //XXX: dump     
               
       }

       return ROFL_SUCCESS;
}

//Specific add command for wildcard entries
static rofl_result_t netfpga_add_entry_hw(netfpga_flow_entry_t* entry){

	unsigned int i;
	uint32_t* aux;
	
	//Wait for the netfpga to be ready
	netfpga_wait_reg_ready(nfpga);

	//Set Row address
	if(entry->type == NETFPGA_FE_FIXED ){
		//ROFL_DEBUG(DEFAULT, "\n  %s : % d  FIXED ENTRY  \n ", __FILE__,__LINE__);
		
		if(netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_EXACT_BASE + entry->hw_pos) != ROFL_SUCCESS)//NETFPGA_EXACT_BASE			0x0000
			return ROFL_FAILURE;
	}else{
		
		//ROFL_DEBUG(DEFAULT, "\n  %s : % d  WILD CARD  \n ", __FILE__,__LINE__);
		if(netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_WILDCARD_BASE + entry->hw_pos) != ROFL_SUCCESS)
			return ROFL_FAILURE;
	}
	




	//Write matches 
	aux = (uint32_t*)entry->matches;
	for (i = 0; i < NETFPGA_FLOW_ENTRY_MATCHES_WORD_LEN; ++i) {
		if(netfpga_write_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_BASE_REG + i, *(aux+i)) != ROFL_SUCCESS)//NETFPGA_OF_LOOKUP_CMP_BASE_REG		NETFPGA_OF_BASE_ADDR+0x22

			return ROFL_FAILURE;
	}

	if( entry->type == NETFPGA_FE_WILDCARDED){
		aux = (uint32_t*)entry->masks;
		for (i = 0; i < NETFPGA_FLOW_ENTRY_WILDCARD_WORD_LEN; ++i) {
			if(netfpga_write_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_MASK_BASE_REG + i, *(aux+i)) != ROFL_SUCCESS)
				return ROFL_FAILURE;
		}
	}
	//sprawdzić z mapa pamieci!!
	//Write actions
	aux = (uint32_t*)entry->actions;
	for (i = 0; i < NETFPGA_FLOW_ENTRY_ACTIONS_WORD_LEN; ++i) {
		if(netfpga_write_reg(nfpga, NETFPGA_OF_LOOKUP_ACTION_BASE_REG + i, *(aux+i)) != ROFL_SUCCESS)	//NETFPGA_OF_LOOKUP_ACTION_BASE_REG	NETFPGA_OF_BASE_ADDR+0x32
			return ROFL_FAILURE;
	}

	//if( entry->type == NETFPGA_FE_WILDCARDED ){
		//Reset the stats for the pos 
		if(netfpga_write_reg(nfpga, NETFPGA_OF_STATS_BASE_REG, 0x0) != ROFL_SUCCESS)
			return ROFL_FAILURE;
		if(netfpga_write_reg(nfpga, NETFPGA_OF_STATS_BASE_REG+1, 0x0) != ROFL_SUCCESS)
			return ROFL_FAILURE;
	//}

	//Write whatever => Trigger load to table
	if(netfpga_write_reg(nfpga, NETFPGA_OF_WRITE_ORDER_REG, 0x1) != ROFL_SUCCESS) 
		return ROFL_FAILURE; 

	return ROFL_SUCCESS;
}

//Specific delete commands for wildcarded entries
static rofl_result_t netfpga_delete_entry_hw(unsigned int pos){

	rofl_result_t result;

	//Creating empty tmp entry	
	netfpga_flow_entry_t* hw_entry = netfpga_init_flow_entry();
	hw_entry->hw_pos=pos;

	if(!hw_entry)
		return ROFL_FAILURE; 
	
	//Perform the add (delete actually)
	result = netfpga_add_entry_hw(hw_entry);

	//Destroy tmp entry
	netfpga_destroy_flow_entry(hw_entry);	
	
	return result;
}

//Setup the catch-all DMA entries (packet_in) and packet out entries
static rofl_result_t netfpga_init_dma_mechanism(){




	unsigned int i;
	netfpga_flow_entry_t* entry;

	//Create an empty entry
	entry = netfpga_init_flow_entry();	

	//Wildcard ALL except ports
	memset(entry->masks, 0xFF, sizeof(*entry->masks));
	entry->masks->src_port = 0x0;
	entry->type = NETFPGA_FE_WILDCARDED; 

	//Insert catch all entries (PKT_IN)
	for (i = 0; i < 4; ++i) {
		entry->matches->src_port = 0x1 << (i * 2);
		entry->actions->forward_bitmask = 0x1 << ((i * 2) + 1);
		entry->hw_pos =(NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE - 4) + i; 				 	
		
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;
	}

	//Insert output entries (PKT_OUT)  
	for (i = 0; i < 4; ++i) {
		entry->matches->src_port = 0x1 << ((i * 2) + 1);
		entry->actions->forward_bitmask = 0x1 << (i * 2);
		entry->hw_pos =(NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE - 8) + i; 	 	
	
		//Install entry
		if(netfpga_add_entry_hw(entry) != ROFL_SUCCESS)
			return ROFL_FAILURE;
	}

	//Destroy tmp entry
	netfpga_destroy_flow_entry(entry);	

	return ROFL_SUCCESS;
}


/*
 * External interfaces
 */

//Getter only used internally in the netfpga code
netfpga_device_t* netfpga_get(){
	return nfpga;
}

//Initializes the netfpga shared state, including appropiate state of registers and bootstrap. 
rofl_result_t netfpga_init(){

	if(nfpga){
		ROFL_DEBUG(DEFAULT, "Double call to netfpga_init()\n");
		assert(0);
		return ROFL_SUCCESS; //Skip
	}

	nfpga = (netfpga_device_t*)malloc(sizeof(*nfpga));
	memset(nfpga, 0, sizeof(*nfpga));


	//Open fd
	nfpga->fd = open(NETFPGA_DEVNAME, O_RDWR);
	if( ( nfpga->fd) < 0)
		return ROFL_FAILURE;

	//Reset counters
	nfpga->num_of_wildcarded_entries= nfpga->num_of_exact_entries = 0;

	//Init mutex
	pthread_mutex_init(&nfpga->mutex, NULL);

	//Delete all entries
	if(netfpga_delete_all_entries() != ROFL_SUCCESS)
		return ROFL_FAILURE;

	//Init DMA
	if(netfpga_init_dma_mechanism() != ROFL_SUCCESS)
		return ROFL_FAILURE;

	//FIXME: set registers	
/*
	// cleaning statistisc imposible due to read only stats registers in OFSWITCH
	uint32_t misc_stats[NETFPGA_NUMBER_OF_MISC_STATS];
	netfpga_read_misc_stats(misc_stats);
	displacy_misc_stats(misc_stats);
	
	//clean misculenious stats
	if(netfpga_clean_misc_stats() != ROFL_SUCCESS)
		return ROFL_FAILURE;
	netfpga_read_misc_stats(misc_stats);
	displacy_misc_stats(misc_stats);
*/
	ROFL_DEBUG(DEFAULT, "\n   END OF NETFPGA INITIALIZATION !!!!!!!!!!!!!!!!!!!!!!!! \n");
	return ROFL_SUCCESS;
}

//Destroys state of the netfpga, and restores it to the original state (state before init) 
rofl_result_t netfpga_destroy(){

	switch_port_t* port[4];
	port[0] = physical_switch_get_port_by_name("nf0");
	port[1] = physical_switch_get_port_by_name("nf1");
	port[2] = physical_switch_get_port_by_name("nf2");
	port[3] = physical_switch_get_port_by_name("nf3");
	
	if(ROFL_FAILURE==netfpga_destroy_port(port[0])) ROFL_DEBUG(DEFAULT, "netfpga_destroy_port() failled");
	if(ROFL_FAILURE==netfpga_destroy_port(port[1])) ROFL_DEBUG(DEFAULT, "netfpga_destroy_port() failled");
	if(ROFL_FAILURE==netfpga_destroy_port(port[2])) ROFL_DEBUG(DEFAULT, "netfpga_destroy_port() failled");
	if(ROFL_FAILURE==netfpga_destroy_port(port[3])) ROFL_DEBUG(DEFAULT, "netfpga_destroy_port() failled");

	//for(int i=0;i<NETFPGA_LAST_PORT-1;i++){		
	//	if(ROFL_FAILURE==netfpga_destroy_port(port[i])) ROFL_DEBUG(DEFAULT, "netfpga_destroy_port() failled");
	//}

	if(!nfpga){
		ROFL_DEBUG(DEFAULT, "netfpga_destroy() called without netfpga being initialized!\n");
		assert(0);
		return ROFL_SUCCESS; //Skip
	}	

	//Destroy mutex
	pthread_mutex_destroy(&nfpga->mutex);


	//FIXME set registers

	close(nfpga->fd);
	free(nfpga);	
	
	return ROFL_SUCCESS;
}

//Lock netfpga so that other threads cannot do operations of it. 
void netfpga_lock(){
	pthread_mutex_lock(&nfpga->mutex);
}

//Unlock netfpga so that other threads can do operations of it. 
void netfpga_unlock(){
	pthread_mutex_unlock(&nfpga->mutex);
}




//Set table behaviour
rofl_result_t netfpga_set_table_behaviour(void){
	//FIXME
	return ROFL_SUCCESS;
}

//Add flow entry to table 
rofl_result_t netfpga_add_flow_entry(of1x_flow_entry_t* entry){
	//ROFL_DEBUG(DEFAULT, "%s : %d GOT FLOW ENTRY! \n", __FILE__,__LINE__);
	//Map entry to a hw entry

	//ROFL_DEBUG(DEFAULT, "",entry->matchs->    ,            ,             );
	netfpga_flow_entry_t* hw_entry = netfpga_generate_hw_flow_entry(nfpga, entry);


	if(!hw_entry)
		return ROFL_FAILURE;
	
	//Do the association with the netfpga state
	entry->platform_state = (of1x_flow_entry_platform_state_t*) hw_entry;

	//Write to hw
	if( netfpga_add_entry_hw(hw_entry) != ROFL_SUCCESS ){
		//Remove reference (release slot) in the table
		//Acquired in netfpga_generate_hw_flow_entry()
		if( hw_entry->type == NETFPGA_FE_WILDCARDED )
			nfpga->hw_wildcard_table[hw_entry->hw_pos] = NULL;
		else
			nfpga->hw_exact_table[hw_entry->hw_pos] = NULL;
			
		return ROFL_FAILURE;
	}

	//Increment counters
	if( hw_entry->type == NETFPGA_FE_WILDCARDED )
		nfpga->num_of_wildcarded_entries++;	
	else
		nfpga->num_of_exact_entries++;	

	return ROFL_SUCCESS;
}

//Deletes an specific entry defined by *entry 
rofl_result_t netfpga_delete_flow_entry(of1x_flow_entry_t* entry){

	//Recover the position
	netfpga_flow_entry_t* hw_entry = (netfpga_flow_entry_t*)entry->platform_state;

	//Check
	if(!hw_entry)
		return ROFL_FAILURE;

	//Delete entry in HW
	if( netfpga_delete_entry_hw(hw_entry->hw_pos) != ROFL_SUCCESS )
		return ROFL_FAILURE;
	
	

	//Decrement counters
	if( hw_entry->type == NETFPGA_FE_WILDCARDED )
		nfpga->num_of_wildcarded_entries--;	
	else
		nfpga->num_of_exact_entries--;	


	//Destroy
	netfpga_destroy_flow_entry(hw_entry);

	//Unset platform state
	entry->platform_state = NULL;	

	return ROFL_SUCCESS;
}

//Deletes all entries within a table 
rofl_result_t netfpga_delete_all_entries(void){

	unsigned int i;	
	netfpga_flow_entry_t* entry;

	//Create an empty entry
	entry = netfpga_init_flow_entry();	

	//Attempt to delete all entries in the table
	//for fixed flow entries
	entry->type = NETFPGA_FE_FIXED;
	for(i=0; i< NETFPGA_OPENFLOW_EXACT_TABLE_SIZE; ++i){
		entry->hw_pos = i;
		netfpga_delete_entry_hw(entry->hw_pos);	
	}
	
	//Attempt to delete all entries in the table
	//for wildcarded flow entries
	entry->type = NETFPGA_FE_WILDCARDED;
	for(i=NETFPGA_WILDCARD_BASE ; i< (NETFPGA_WILDCARD_BASE + NETFPGA_OPENFLOW_WILDCARD_FULL_TABLE_SIZE - NETFPGA_RESERVED_FOR_CPU2NETFPGA) ; ++i){ 
		entry->hw_pos = i;
		netfpga_delete_entry_hw(entry->hw_pos);	
	}

	//Create an empty entry
	netfpga_destroy_flow_entry(entry);	

	return ROFL_SUCCESS;
}

