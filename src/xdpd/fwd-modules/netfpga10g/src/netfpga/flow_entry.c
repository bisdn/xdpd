#include "flow_entry.h"


//Creates a (empty) flow entry (mappable to HW) 
netfpga_flow_entry_t* netfpga_init_entry(){

	netfpga_flow_entry_t* entry;
	
	entry = malloc(sizeof(netfpga_flow_entry_t));
	if(!entry)
		return NULL;

	//Allocate matches
	entry->matches = malloc(sizeof(netfpga_flow_entry_matches_t));
	if(!entry->matches){
		free(entry);
		return NULL;	
	}
	
	//Allocate wildcards
	entry->mask = malloc(sizeof(netfpga_flow_entry_matches_mask_t));
	if(!entry->mask){
		free(entry->matches);
		free(entry);
		return NULL;	
	}

	//Allocate actions	
	entry->actions = malloc(sizeof(netfpga_flow_entry_actions_t));
	if(!entry->actions){
		free(entry->matches);
		free(entry->mask);
		free(entry);
		return NULL;	
	}
	
	//Init all	
	memset(entry,0,sizeof(*entry));
	memset(entry->matches, 0, sizeof(*(entry->matches)));
	memset(entry->mask, 0, sizeof(*(entry->mask)));
	memset(entry->actions, 0, sizeof(*(entry->actions)));
	
	return entry;
}


//Destroys an entry previously created via netfpga_init_entry() 

void netfpga_destroy_entry(netfpga_flow_entry_t* entry){

	if(!entry)
		return;
	
	free(entry->matches);
	free(entry->mask);
	free(entry->actions);
	free(entry);
}


