#include "c_types_pktclassifier.h"
#include <stdlib.h>
#include <string.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/common/protocol_constants.h>
#include "../packet_operations.h"
#include "../../../config.h"

/// Classify part
classify_state_t* init_classifier(datapacket_t*const  pkt){
	classify_state_t* classifier = malloc(sizeof(classify_state_t));
	memset(classifier,0,sizeof(classify_state_t));
	classifier->matches = &pkt->matches;
	return classifier;
}
void destroy_classifier(classify_state_t* clas_state){
	free(clas_state);
}

void pop_vlan(datapacket_t* pkt, classify_state_t* clas_state){

	pkt_types_t new = PT_POP_PROTO(clas_state, VLAN);  
	if(unlikely(new == PT_INVALID))
		return;

	cpc_vlan_hdr_t* vlan = get_vlan_hdr(clas_state,0);
	uint16_t ether_type = get_vlan_type(vlan);
	
	//Take header out from packet
	pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_vlan_hdr_t));

	//Set new type and base(move right)
	clas_state->type = new;
	clas_state->base += sizeof(cpc_vlan_hdr_t);

	//Set ether_type of new frame
	set_ether_type(get_ether_hdr(clas_state,0),ether_type);
}
void pop_mpls(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	
	pkt_types_t new = PT_POP_PROTO(clas_state, MPLS);  
	if(unlikely(new == PT_INVALID))
		return;

	pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_mpls_hdr_t));

	//Set new type and base(move right)
	clas_state->type = new;
	clas_state->base += sizeof(cpc_mpls_hdr_t);

	set_ether_type(get_ether_hdr(clas_state,0), ether_type);
}

void pop_pppoe(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	cpc_eth_hdr_t* ether_header;
	
	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state,0);

	switch (get_ether_type(ether_header)) {
		case ETH_TYPE_PPPOE_DISCOVERY:
		{
			pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_pppoe_hdr_t));
			pkt_types_t new = PT_POP_PROTO(clas_state, PPPOE);
			if(unlikely(new == PT_INVALID))
				return;
			//Set new type and base(move right)
			clas_state->type = new;
			clas_state->base += sizeof(cpc_pppoe_hdr_t); 
		}
			break;

		case ETH_TYPE_PPPOE_SESSION:
		{
			pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t),sizeof(cpc_pppoe_hdr_t) + sizeof(cpc_ppp_hdr_t));
			pkt_types_t new = PT_POP_PROTO(clas_state, PPP);
			if(unlikely(new == PT_INVALID))
				return;
			new = PT_POP_PROTO(clas_state, PPPOE);
			if(unlikely(new == PT_INVALID))
				return;

			//Set new type and base(move right)
			clas_state->type = new;
			clas_state->base += sizeof(cpc_pppoe_hdr_t) + sizeof(cpc_ppp_hdr_t);
		}
		break;
	}

	set_ether_type(get_ether_hdr(clas_state,0), ether_type);
	
	//re-classify packet to get the headers beyond pppoe
	classify_packet(clas_state, get_pkt_buffer(pkt), get_pkt_buffer_length(pkt),get_pkt_in_port(pkt), get_pkt_in_phy_port(pkt));
}

void pop_gtp(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	//TODO
	return;
}

void* push_vlan(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){

	uint8_t* ether_header;

	pkt_types_t new = PT_PUSH_PROTO(clas_state, VLAN);  
	if(unlikely(new == PT_INVALID))
		return NULL;

	//Recover the current ether(0)
	ether_header = get_ether_hdr(clas_state, 0);
	cpc_vlan_hdr_t* inner_vlan_header = get_vlan_hdr(clas_state, 0);
	uint16_t inner_ether_type = get_ether_type(ether_header);

	//Move bytes
	if (pkt_push(pkt, NULL, sizeof(cpc_eth_hdr_t), sizeof(cpc_vlan_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}
	
	//Set new type and base(move left)
	clas_state->type = new;
	clas_state->base -= sizeof(cpc_vlan_hdr_t);

	//Set new pkt type
	cpc_vlan_hdr_t* vlan_header = get_vlan_hdr(clas_state, 0);
	ether_header = get_ether_hdr(clas_state, 0);    
	
	if ( inner_vlan_header ) {
		set_vlan_id(vlan_header, get_vlan_id(inner_vlan_header));
		set_vlan_pcp(vlan_header, get_vlan_pcp(inner_vlan_header));
	} else {
		set_vlan_id(vlan_header,0x0000);
		set_vlan_pcp(vlan_header,0x00);
	}

	set_vlan_type(vlan_header,inner_ether_type);
	set_ether_type(ether_header, ether_type);
	
	return vlan_header;
}

void* push_mpls(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	void* ether_header;
	cpc_mpls_hdr_t* mpls_header, *inner_mpls_header;

	pkt_types_t new = PT_PUSH_PROTO(clas_state, MPLS);  
	if(unlikely(new == PT_INVALID))
		return NULL;

	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state, 0);
	inner_mpls_header = get_mpls_hdr(clas_state, 0);
	cpc_ipv4_hdr_t *ipv4_hdr = get_ipv4_hdr(clas_state, 0);
	cpc_ipv4_hdr_t *ipv6_hdr = get_ipv6_hdr(clas_state, 0);

	/*
	 * this invalidates ether(0), as it shifts ether(0) to the left
	 */
	if (pkt_push(pkt, (void*)(ether_header + sizeof(cpc_eth_hdr_t)),0 , sizeof(cpc_mpls_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}
	
	//Set new type and base(move left)
	clas_state->type = new;
	clas_state->base -= sizeof(cpc_mpls_hdr_t);

	ether_header=get_ether_hdr(clas_state, 0);
	set_ether_type(ether_header, ether_type);
	
	//Set default values
	mpls_header = get_mpls_hdr(clas_state, 0);
	if (inner_mpls_header){
		set_mpls_bos(mpls_header, false);
		set_mpls_label(mpls_header, get_mpls_label(inner_mpls_header));
		set_mpls_tc(mpls_header, get_mpls_tc(inner_mpls_header));
		set_mpls_ttl(mpls_header, get_mpls_ttl(inner_mpls_header));
	} else {
		set_mpls_bos(mpls_header, true);
		set_mpls_label(mpls_header, 0x0000);
		set_mpls_tc(mpls_header, 0x00);
		if ( ipv4_hdr ){
			set_mpls_ttl(mpls_header, get_ipv4_ttl(ipv4_hdr));
		}else if ( ipv6_hdr ){
			set_mpls_ttl(mpls_header, get_ipv6_hop_limit(ipv6_hdr));
		}else
			set_mpls_ttl(mpls_header, 0x00);
	}

	return mpls_header;
}

void* push_pppoe(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	
	void* ether_header;
	//unsigned int current_length;
	pkt_types_t new = PT_PUSH_PROTO(clas_state, PPPOE);  
	if(unlikely(new == PT_INVALID))
		return NULL;

	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state, 0);
	//current_length = ether_header->framelen(); 
	
	cpc_pppoe_hdr_t *n_pppoe = NULL; 
	cpc_ppp_hdr_t *n_ppp = NULL; 

	switch (ether_type) {
		case ETH_TYPE_PPPOE_SESSION:
		{
			unsigned int bytes_to_insert = sizeof(cpc_pppoe_hdr_t) + sizeof(cpc_ppp_hdr_t);

			/*
			 * this invalidates ether(0), as it shifts ether(0) to the left
			 */
			if (pkt_push(pkt, NULL, sizeof(cpc_eth_hdr_t), bytes_to_insert) == ROFL_FAILURE){
				// TODO: log error
				return NULL;
			}
			//Set new pkt type
			clas_state->type = PT_PUSH_PROTO(clas_state, PPP);
			assert(clas_state->type != PT_INVALID);
			clas_state->base -= bytes_to_insert; 

			/*
			 * adjust ether(0): move one pppoe tag to the left
			 */
			ether_header-=sizeof(cpc_mpls_hdr_t); //We change also the local pointer
			set_ether_type(ether_header, ETH_TYPE_PPPOE_SESSION);
			
			set_ppp_prot(n_ppp, 0x0000);
		}
			break;

		case ETH_TYPE_PPPOE_DISCOVERY:
		{
			unsigned int bytes_to_insert = sizeof(cpc_pppoe_hdr_t);
			
			/*
			 * this invalidates ether(0), as it shifts ether(0) to the left
			 */
			if (pkt_push(pkt, (void*)(ether_header+sizeof(cpc_eth_hdr_t)),0, bytes_to_insert) == ROFL_FAILURE){
				// TODO: log error
				return NULL;
			}
			//Set new pkt type
			clas_state->type = new;
			clas_state->base -= bytes_to_insert; 

			set_ether_type(get_ether_hdr(clas_state, 0), ETH_TYPE_PPPOE_DISCOVERY);

			
		}
			break;
	}

	/*
	 * set default values in pppoe tag
	 */
	set_pppoe_code(n_pppoe, 0x00);
	set_pppoe_sessid(n_pppoe, 0x0000);
	set_pppoe_type(n_pppoe, PPPOE_TYPE);
	set_pppoe_vers(n_pppoe, PPPOE_VERSION);

	return NULL;
}

void* push_gtp(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	return NULL;
}

void dump_pkt_classifier(classify_state_t* clas_state){
	//TODO ROFL_DEBUG(DRIVER_NAME" [c_pktclassifier] datapacketx86(%p) soframe: %p framelen: %zu\n", this, pkt->get_buffer(), pkt->get_buffer_length());
	ROFL_DEBUG(DRIVER_NAME" [c_types_pktclassifier] Dump packet state(%p) TODO!!\n",clas_state);
}

size_t get_pkt_len(datapacket_t* pkt, classify_state_t* clas_state, void *from, void *to){
	assert(to == NULL);	
	return clas_state->len -( ((uint8_t*)from) - clas_state->base);
}
