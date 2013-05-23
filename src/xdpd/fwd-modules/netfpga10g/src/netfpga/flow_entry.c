#include "flow_entry.h"

//Other headers
#include <arpa/inet.h>
#include "netfpga.h"
#include "../util/crc32.h"


//Creates a (empty) flow entry (mappable to HW) 
netfpga_flow_entry_t* netfpga_init_flow_entry(){

	netfpga_flow_entry_t* entry;
	
	entry = malloc(sizeof(netfpga_flow_entry_t));
	if(!entry)
		return NULL;

	//memset entry
	memset(entry,0,sizeof(*entry));
	
	//Allocate matches
	entry->matches = malloc(sizeof(netfpga_flow_entry_matches_t));
	if(!entry->matches){
		free(entry);
		return NULL;	
	}
	
	//Allocate wildcards
	entry->masks = malloc(sizeof(netfpga_flow_entry_matches_mask_t));
	if(!entry->masks){
		free(entry->matches);
		free(entry);
		return NULL;	
	}

	//Allocate actions	
	entry->actions = malloc(sizeof(netfpga_flow_entry_actions_t));
	if(!entry->actions){
		free(entry->matches);
		free(entry->masks);
		free(entry);
		return NULL;	
	}
	
	//Init all	
	memset(entry->matches, 0, sizeof(*(entry->matches)));
	memset(entry->masks, 0, sizeof(*(entry->masks)));
	memset(entry->actions, 0, sizeof(*(entry->actions)));
	
	return entry;
}


//Destroys an entry previously created via netfpga_init_entry() 
void netfpga_destroy_flow_entry(netfpga_flow_entry_t* entry){

	if(!entry)
		return;
	
	free(entry->matches);
	free(entry->masks);
	free(entry->actions);
	free(entry);
}

//
// Flow mod translation
//
#define VID_BITMASK 0x0FFF
#define VID_PCP_BITMASK 0xE000
#define VID_PCP_SHIFT_BITS 13
#define TOS_ECN_BITMASK 0x03
#define TOS_DSCP_BITMASK 0xFC
#define TOS_DSCP_SHIFT_BITS 2 


static rofl_result_t netfpga_flow_entry_map_matches(netfpga_flow_entry_t* entry, of12_flow_entry_t* of12_entry){
	
	of12_match_t* match;
	uint32_t tmp_mask;
	uint8_t* aux; //FIXME: clean this
	netfpga_flow_entry_matches_t* matches = entry->matches;
	netfpga_flow_entry_matches_mask_t* masks = entry->masks;

	//Go through all the matches and set entry matches
	for(match = of12_entry->matchs; match;match = match->next){

		switch(match->type){

			case OF12_MATCH_IN_PORT:
				matches->src_port = /*htons*/( ( ((utern8_t*)match->value)->value) );
				masks->src_port = 0xFF; //Exact
				break;
 			case OF12_MATCH_ETH_DST:
				//Does not require byte moving. 6 bytes are in the lower bytes of the uint64_t
				aux = (uint8_t*) &((utern8_t*) match->value)->value;
				matches->eth_dst = *( (netfpga_align_mac_addr_t*)(aux+2) ); //This is nasty, I know and apologize...
				memset(&masks->eth_src,0xF,sizeof(masks->eth_src)); //TODO: add mask...
				break;
 			case OF12_MATCH_ETH_SRC:
				//Does not require byte moving. 6 bytes are in the lower bytes of the uint64_t
				aux = (uint8_t*) &((utern8_t*) match->value)->value;
				matches->eth_src = *( (netfpga_align_mac_addr_t*)(aux+2) ); //This is nasty, I know and apologize...
				memset(&masks->eth_dst,0xF,sizeof(masks->eth_dst)); //TODO: add mask...
				break;
 			case OF12_MATCH_ETH_TYPE:
				matches->eth_type = /*htons*/( ( ((utern16_t*)match->value)->value) );	
				masks->eth_type = 0xFFFF;
				break;
 			case OF12_MATCH_VLAN_PCP:
				matches->vlan_id |= ( ((utern8_t*)match->value)->value << VID_PCP_SHIFT_BITS) &VID_PCP_BITMASK;
				masks->vlan_id |= 0x7<< VID_PCP_SHIFT_BITS;
 				break;	
			case OF12_MATCH_VLAN_VID:
				matches->vlan_id |= /*htons*/(((utern16_t*)match->value)->value) & VID_BITMASK;	
				masks->vlan_id |= VID_BITMASK; //Exact
				break;
 			case OF12_MATCH_IP_DSCP:
				matches->ip_tos |= ( (((utern8_t*)match->value)->value)<<TOS_DSCP_SHIFT_BITS ) & TOS_DSCP_BITMASK;
				masks->ip_tos |= TOS_DSCP_BITMASK;  
				break;
 			case OF12_MATCH_IP_ECN:
				matches->ip_tos |= ( ((utern8_t*)match->value)->value) & TOS_ECN_BITMASK;
				masks->ip_tos |= TOS_ECN_BITMASK;  
				break;
 			case OF12_MATCH_IP_PROTO:
				matches->ip_proto = ((utern8_t*)match->value)->value;	
				masks->ip_proto = 0xFF;
				break;
 			case OF12_MATCH_IPV4_SRC:
				
				matches->ip_src = /*htonl*/( ((utern32_t*)match->value)->value );
				tmp_mask = /*htonl*/( ((utern32_t*)match->value)->mask );
				if(tmp_mask != 0xFFFF && tmp_mask != 0x0)
					//Is wildcarded
					entry->type = NETFPGA_FE_WILDCARDED;
  
				masks->ip_src = tmp_mask;
				break;
 			case OF12_MATCH_IPV4_DST:
				matches->ip_dst = /*htonl*/( ((utern32_t*)match->value)->value );

				tmp_mask = /*htonl*/( ((utern32_t*)match->value)->mask );
				if(tmp_mask != 0xFFFF && tmp_mask != 0x0)
					//Is wildcarded
					entry->type = NETFPGA_FE_WILDCARDED;
 
				masks->ip_dst = tmp_mask;
				break;
 			
			case OF12_MATCH_TCP_SRC:
 			case OF12_MATCH_UDP_SRC:
				matches->transp_src =  /*htons*/( ((utern16_t*)match->value)->value );
				masks->transp_src = 0xFFFF;
				break;
 			case OF12_MATCH_TCP_DST:
 			case OF12_MATCH_UDP_DST:
				matches->transp_dst =  /*htons*/( ((utern16_t*)match->value)->value );	
				masks->transp_dst = 0xFFFF;
				break;
			
			default: //Skip
				break;

		}	
	}

	
	return ROFL_SUCCESS;
}

static rofl_result_t netfpga_flow_entry_map_actions(netfpga_flow_entry_t* entry, of12_flow_entry_t* of12_entry){

	unsigned int i;
	uint16_t port;
	void* indirect;
	of12_packet_action_t* action;
	netfpga_flow_entry_actions_t* actions = entry->actions;

	//If entry has not actions we are done (should we really install it down there?)
	if(!of12_entry->inst_grp.instructions[OF12_IT_APPLY_ACTIONS-1].apply_actions)
		return ROFL_SUCCESS;

	action = of12_entry->inst_grp.instructions[OF12_IT_APPLY_ACTIONS-1].apply_actions->head;
	
	if(!action){
		assert(0);
		return ROFL_FAILURE;
	}


	//Loop over apply actions only
	for(; action; action = action->next){
	
		//FIXME: quick hack (no aliasing in forced in all xdpd, for good reasons). Fix it!
		indirect = (void*)action->field;	

		switch(action->type){

			case OF12_AT_SET_FIELD_ETH_DST:
				actions->eth_dst = *( (netfpga_align_mac_addr_t*) (((uint8_t*) indirect)) ); //This is nasty, I know and apologize...
				actions->action_flags |= (1 << NETFPGA_AT_SET_DL_DST);	
				break;
			case OF12_AT_SET_FIELD_ETH_SRC:
				actions->eth_src = *( (netfpga_align_mac_addr_t*) (((uint8_t*) indirect)) ); //This is nasty, I know and apologize...
				actions->action_flags |= (1 << NETFPGA_AT_SET_DL_SRC);	
				break;

			case OF12_AT_SET_FIELD_VLAN_VID:
				actions->vlan_id |= /*htons*/(action->field) & VID_BITMASK;	
				actions->action_flags |= (1 << NETFPGA_AT_SET_VLAN_VID);	
				break;
			case OF12_AT_SET_FIELD_VLAN_PCP:
				actions->vlan_id |= (*(((uint8_t*)indirect)) << VID_PCP_SHIFT_BITS)&VID_PCP_BITMASK;
				actions->action_flags |= (1 << NETFPGA_AT_SET_VLAN_PCP);	
				break;
			case OF12_AT_SET_FIELD_IP_DSCP:
				actions->ip_tos |= ( (*(((uint8_t*)indirect)))<<TOS_DSCP_SHIFT_BITS ) & TOS_DSCP_BITMASK;
				actions->action_flags |= (1 << NETFPGA_AT_SET_NW_TOS);	
				break;
			case OF12_AT_SET_FIELD_IP_ECN:
				actions->ip_tos |= (*(((uint8_t*)indirect))) & TOS_ECN_BITMASK;
				actions->action_flags |= (1 << NETFPGA_AT_SET_NW_TOS);	
				break;


			case OF12_AT_SET_FIELD_IPV4_SRC:
				actions->ip_src = /*htonl*/(*(((uint32_t*)indirect) ));
				actions->action_flags |= (1 << NETFPGA_AT_SET_TP_SRC);	
				break;
			case OF12_AT_SET_FIELD_IPV4_DST:
				actions->ip_dst = /*htonl*/(*(((uint32_t*)indirect) ));
				actions->action_flags |= (1 << NETFPGA_AT_SET_TP_DST);	
				break;
			case OF12_AT_SET_FIELD_TCP_SRC:
				actions->transp_src = /*htons*/(*(((uint16_t*)indirect) ));
				actions->action_flags |= (1 << NETFPGA_AT_SET_TP_SRC);	
				break;
			case OF12_AT_SET_FIELD_TCP_DST:
				actions->transp_dst = /*htons*/(*(((uint16_t*)indirect) ));
				actions->action_flags |= (1 << NETFPGA_AT_SET_TP_DST);	
				break;
			case OF12_AT_SET_FIELD_UDP_SRC:
				actions->transp_src = /*htons*/(*(((uint16_t*)indirect) ));
				break;
			case OF12_AT_SET_FIELD_UDP_DST:
				actions->transp_dst = /*htons*/(*(((uint16_t*)indirect) ));
				break;
			case OF12_AT_OUTPUT:
				port = *(((uint16_t*)indirect) )&0xFFFF;

				if ((port >= NETFPGA_FIRST_PORT) && (port <= NETFPGA_LAST_PORT)) {
					//Send to specific port 
					actions->forward_bitmask |= (1 << ((port - NETFPGA_FIRST_PORT) * 2));
				}else if (port == NETFPGA_IN_PORT) {
					//Send back to in-port	
					actions->forward_bitmask |= (entry->matches->src_port);
				}else if(port == NETFPGA_ALL_PORTS || port == NETFPGA_FLOOD_PORT) {
					//Send to all ports except in-port	
					for(i = NETFPGA_FIRST_PORT; i <= NETFPGA_LAST_PORT; ++i) {
						if(entry->matches->src_port != (0x1 << (i * 2))) {
							actions->forward_bitmask |= (0x1 << (i * 2));
						}
					}
				}else{
					//Wrong port!
					assert(0);
				}
				break;
			
			default:
				break;
		}
	}
	
	
	return ROFL_FAILURE;
}

//Calculate exact position
static void netfpga_set_hw_position_exact(netfpga_device_t* nfpga, netfpga_flow_entry_t* hw_entry){
	
	uint32_t hash;
	uint32_t pos; 
	struct crc32 crc;

	crc32_init(&crc, NETFPGA_POLYNOMIAL1);
	hash = crc32_calculate(&crc, hw_entry->matches, sizeof(*hw_entry->matches));

	//Calculate the index based on the hash.  The last bits of hash is the index in the table
	pos = (NETFPGA_OPENFLOW_EXACT_TABLE_SIZE-1) & hash;

	if (nfpga->hw_exact_table[pos] == NULL) {
		hw_entry->hw_pos = pos;
		nfpga->hw_exact_table[pos] = hw_entry;
		return;
	}

	//Use fallback polynomial
	crc32_init(&crc, NETFPGA_POLYNOMIAL2);
	hash = crc32_calculate(&crc, hw_entry->matches, sizeof(*hw_entry->matches));
	
	// the bottom fixed bits of hash == the index into the table
	pos = (NETFPGA_OPENFLOW_EXACT_TABLE_SIZE-1) & hash;

	//Check existing entry
	if (nfpga->hw_exact_table[pos] != NULL) {
		//FIXME: if is the same, the previous entry should be removed... Which is the collision probability?
	}
	
	hw_entry->hw_pos = pos;
	nfpga->hw_exact_table[pos] = hw_entry;
}

//Determine wildcard position
/*
* FIXME: this is simply wrong. In EXACT table is correct assuming no order, since is exact, but in wildcard
* priority should be taken into account. This implies some sort of either brute force moving of entries once
* an insertion of a higher priority entries needs to happen, or better, implement priority in HW.
*
* I am just following the reference implementation here
*/
static void netfpga_set_hw_position_wildcard(netfpga_device_t* nfpga, netfpga_flow_entry_t* hw_entry){

	unsigned int i;
	
	for(i=0; i<NETFPGA_OPENFLOW_WILDCARD_TABLE_SIZE; ++i){
		if( nfpga->hw_wildcard_table[i] == NULL){
			hw_entry->hw_pos = i;
			nfpga->hw_exact_table[i] = hw_entry;
			return;
		}
	}		

	//This cannot happen, since pre-condition should be checked by the fwd_module before calling rofl-pipeline entry insertion
	assert(0);
}

netfpga_flow_entry_t* netfpga_generate_hw_flow_entry(netfpga_device_t* nfpga, of12_flow_entry_t* of12_entry){

	netfpga_flow_entry_t* entry;

	//Create the entry container	
	entry = netfpga_init_flow_entry();
	if(!entry)
		return NULL;

	//Do the translation matches
	if(netfpga_flow_entry_map_matches(entry, of12_entry) != ROFL_SUCCESS){
		netfpga_destroy_flow_entry(entry);
		return NULL;
	}
	
	//Do the translation actions 
	if(netfpga_flow_entry_map_actions(entry, of12_entry) != ROFL_SUCCESS){
		netfpga_destroy_flow_entry(entry);
		return NULL;
	}

	//Determine the position of the entry
	if( entry->type == NETFPGA_FE_WILDCARDED )
		netfpga_set_hw_position_wildcard(nfpga, entry);
	else
		netfpga_set_hw_position_exact(nfpga, entry);


	return entry;	
}
