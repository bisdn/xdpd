#include "netfpga.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <rofl/common/utils/c_logger.h>
#include <assert.h>

#include "regs.h"
#include "flow_entry.h"

#define NETFPGA_DEVNAME "/dev/nf10"

//Pointer to the unique (static) netfpga_dev_info_t
static netfpga_dev_info_t* nfpga=NULL;

//Initializes the netfpga shared state, including appropiate state of registers and bootstrap.
rofl_result_t netfpga_init(){

	if(nfpga){
		ROFL_DEBUG("Double call to netfpga_init()\n");
		assert(0);
		return ROFL_SUCCESS; //Skip
	}

	nfpga = (netfpga_dev_info_t*)malloc(sizeof(*nfpga));
	//memset(*nfpga, 0, sizeof(*nfpga));


	//Open fd
	nfpga->fd = open(NETFPGA_DEVNAME, O_RDWR);
	if( ( nfpga->fd) < 0)
		return ROFL_FAILURE;

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

	//FIXME set registers

	close(nfpga->fd);
	free(nfpga);	
	
	return ROFL_SUCCESS;
}

//Deletes all entries within a table 

rofl_result_t netfpga_set_table_behaviour(void){
	//FIXME
	
	return ROFL_SUCCESS;
}


//Specific delete commands
static rofl_result_t netfpga_delete_exact_entry(unsigned int pos){

	//FIXME: implement	
	
	return ROFL_SUCCESS;
}

static rofl_result_t netfpga_delete_wildcard_entry(unsigned int pos){

	//FIXME: implement	
	
	return ROFL_SUCCESS;
}

rofl_result_t netfpga_delete_all_entries(void){

	unsigned int i;	
	netfpga_flow_entry_t* entry;

	//Create an empty entry
	entry = netfpga_init_entry();	

	//Attempt to delete all entries in the table
	//for fixed flow entries
	entry->type = NETFPGA_FE_FIXED;

	for(i=0; i< NETFPGA_OPENFLOW_EXACT_TABLE_SIZE; ++i){
		entry->hw_pos = i;
		netfpga_delete_exact_entry(i);	
	}
	
	//Attempt to delete all entries in the table
	//for wildcarded flow entries
	entry->type = NETFPGA_FE_WILDCARDED;
	
	for(i=0; i< (NETFPGA_OPENFLOW_WILDCARD_TABLE_SIZE - NETFPGA_RESERVED_FOR_CPU2NETFPGA) ; ++i){
		entry->hw_pos = i;
		netfpga_delete_wildcard_entry(i);	
	}

	//Create an empty entry
	netfpga_destroy_entry(entry);	

	return ROFL_SUCCESS;
}


