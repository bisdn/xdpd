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

#define NETFPGA_READY_WAIT_TIME_US 50000  //50ms

//Specific add command for wildcard entries
static rofl_result_t netfpga_add_entry_hw(netfpga_flow_entry_t* entry){

	unsigned int i;
	uint32_t* aux;
	uint32_t reg_val;

	//Wait for the netfpga to be ready
	netfpga_read_reg(nfpga, NETFPGA_OF_ACC_RDY_REG,&reg_val);
	while ( !reg_val&0x01 ){
		//Not ready loop
		usleep(NETFPGA_READY_WAIT_TIME_US);
		netfpga_read_reg(nfpga, NETFPGA_OF_ACC_RDY_REG,&reg_val);
	}

	//Set Row address
	if(entry->type == NETFPGA_FE_FIXED )
		netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_EXACT_BASE + entry->hw_pos);
	else
		netfpga_write_reg(nfpga, NETFPGA_OF_BASE_ADDR_REG, NETFPGA_WILDCARD_BASE + entry->hw_pos);

	//Write matches 
	aux = (uint32_t*)entry->masks;
	for (i = 0; i < NETFPGA_FLOW_ENTRY_MATCHES_WORD_LEN; ++i) {
		netfpga_write_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_BASE_REG + i, *(aux+i));
	}

	if( entry->type == NETFPGA_FE_WILDCARDED ){
		//Write masks
		aux = (uint32_t*)entry->masks;
		for (i = 0; i < NETFPGA_FLOW_ENTRY_WILDCARD_WORD_LEN; ++i) {
			netfpga_write_reg(nfpga, NETFPGA_OF_LOOKUP_CMP_MASK_BASE_REG + i, *(aux+i));
		}
	}
	
	//Write actions
	aux = (uint32_t*)entry->masks;
	for (i = 0; i < NETFPGA_FLOW_ENTRY_ACTIONS_WORD_LEN; ++i) {
		netfpga_write_reg(nfpga, NETFPGA_OF_LOOKUP_ACTION_BASE_REG + i, *(aux+i));
	}

	if( entry->type == NETFPGA_FE_WILDCARDED ){
		//Reset the stats for the pos 
		netfpga_write_reg(nfpga, NETFPGA_OF_STATS_BASE_REG, 0x0);
		netfpga_write_reg(nfpga, NETFPGA_OF_STATS_BASE_REG+1, 0x0);
	}

	//Write whatever => Trigger load to table
	netfpga_write_reg(nfpga, NETFPGA_OF_WRITE_ORDER_REG, 0x1); // Write whatever the value

	return ROFL_SUCCESS;
}

//Specific delete commands for wildcarded entries
static rofl_result_t netfpga_delete_entry_hw(unsigned int pos){

	rofl_result_t result;

	//Creating empty tmp entry	
	netfpga_flow_entry_t* hw_entry = netfpga_init_flow_entry();
	
	if(!hw_entry)
		return ROFL_FAILURE; 
	
	//Perform the add (delete actually)
	result = netfpga_add_entry_hw(hw_entry);

	//Destroy tmp entry
	netfpga_destroy_flow_entry(hw_entry);	
	
	return result;
}


/*
 * External interfaces
 */

//Initializes the netfpga shared state, including appropiate state of registers and bootstrap.
rofl_result_t netfpga_init(){

	if(nfpga){
		ROFL_DEBUG("Double call to netfpga_init()\n");
		assert(0);
		return ROFL_SUCCESS; //Skip
	}

	nfpga = (netfpga_device_t*)malloc(sizeof(*nfpga));
	//memset(*nfpga, 0, sizeof(*nfpga));


	//Open fd
	nfpga->fd = open(NETFPGA_DEVNAME, O_RDWR);
	if( ( nfpga->fd) < 0)
		return ROFL_FAILURE;

	//Reset counters
	nfpga->num_of_wildcarded_fm = nfpga->num_of_exact_fm = 0;

	//Init mutex
	pthread_mutex_init(&nfpga->mutex, NULL);

	//FIXME: set registers	

	return ROFL_SUCCESS;
}

//Destroys state of the netfpga, and restores it to the original state (state before init) 
rofl_result_t netfpga_destroy(){

	if(!nfpga){
		ROFL_DEBUG("netfpga_destroy() called without netfpga being initialized!\n");
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
rofl_result_t netfpga_add_flow_entry(of12_flow_entry_t* entry){

	//Map entry to a hw entry
	netfpga_flow_entry_t* hw_entry = netfpga_generate_hw_flow_entry(entry);

	if(!hw_entry)
		return ROFL_FAILURE;
	
	//Do the association
	entry->platform_state = (of12_flow_entry_platform_state_t*) hw_entry;

	//Write to hw
	netfpga_add_entry_hw(hw_entry);
		
	//This might not be true but...
	return ROFL_SUCCESS;
}

//Deletes an specific entry defined by *entry 
rofl_result_t netfpga_delete_flow_entry(of12_flow_entry_t* entry){

	//Recover the position
	netfpga_flow_entry_t* hw_entry = (netfpga_flow_entry_t*)entry->platform_state;

	//Check
	if(!hw_entry)
		return ROFL_FAILURE;

	//Delete entry in HW
	netfpga_delete_entry_hw(hw_entry->hw_pos);
	
	//Destroy
	netfpga_destroy_flow_entry(hw_entry);

	entry->platform_state = NULL;	

	//This might not be true but...
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
	for(i=0; i< (NETFPGA_OPENFLOW_WILDCARD_TABLE_SIZE - NETFPGA_RESERVED_FOR_CPU2NETFPGA) ; ++i){
		entry->hw_pos = i;
		netfpga_delete_entry_hw(entry->hw_pos);	
	}

	//Create an empty entry
	netfpga_destroy_flow_entry(entry);	

	return ROFL_SUCCESS;
}

