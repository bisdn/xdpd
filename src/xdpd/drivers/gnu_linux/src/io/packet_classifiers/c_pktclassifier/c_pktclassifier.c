#include "c_pktclassifier.h"
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

	assert(pkt != NULL);
	return classifier;
}
void destroy_classifier(classify_state_t* clas_state){
	free(clas_state);
}

// Remove header from classifier state
void pop_header(classify_state_t* clas_state, enum header_type type, unsigned int start, unsigned int end){
	if(clas_state->num_of_headers[type] == 0){
		//Do nothing
		assert(0);
		return;
	}else if(clas_state->num_of_headers[type] == 1){
		//Easy, just mark as 
		clas_state->headers[start].present = false;
		//headers[start].prev = headers[start].next = NULL;
		return;
	}else{
		//Move stuff around

		header_container_t copy = clas_state->headers[start];

		//Rotate
		unsigned i;
		for( i=start; i<end-1; i++ ){
			clas_state->headers[i] = clas_state->headers[i+1];
		}
		
		//Reset copy
		copy.present = false;
		//copy.prev = copy.next = NULL;
		
		//Set last item
		clas_state->headers[end-1] = copy;		
	}
	//Decrement header type counter	
	clas_state->num_of_headers[type]--;	
}

void pop_vlan(datapacket_t* pkt, classify_state_t* clas_state){
	//cpc_eth_hdr_t* ether_header;
	
	// outermost vlan tag, if any, following immediately the initial ethernet header
	if(clas_state->num_of_headers[HEADER_TYPE_VLAN] == 0 || !clas_state->headers[FIRST_VLAN_FRAME_POS].present)
		return;

	cpc_vlan_hdr_t* vlan = (cpc_vlan_hdr_t*) clas_state->headers[FIRST_VLAN_FRAME_POS].frame;

	if (!vlan)
		return;

	//WARNING check if get_vlan_type returns NULL?
	uint16_t ether_type = *get_vlan_type(vlan);
	
	//Take header out from packet
	pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_vlan_hdr_t));

	//Take header out from classifier state
	pop_header(clas_state, HEADER_TYPE_VLAN, FIRST_VLAN_FRAME_POS, FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES);

	//Recover the ether(0)
	//ether_header = (cpc_eth_hdr_t*)ether(clas_state,0);
	
	//Set ether_type of new frame
	shift_ether(clas_state, 0, sizeof(cpc_vlan_hdr_t)); //shift right
	set_ether_type(get_ether_hdr(clas_state,0),ether_type);
	//ether_header->reset(ether_header->soframe(), ether_header->framelen() - sizeof(struct rofl::fvlanframe::vlan_hdr_t));
}
void pop_mpls(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	// outermost mpls tag, if any, following immediately the initial ethernet header
	
	//cpc_eth_hdr_t* ether_header;

	if (clas_state->num_of_headers[HEADER_TYPE_MPLS] == 0 || !clas_state->headers[FIRST_MPLS_FRAME_POS].present)
		return;
	
	cpc_mpls_hdr_t* mpls = (cpc_mpls_hdr_t*) clas_state->headers[FIRST_MPLS_FRAME_POS].frame;
	
	if (!mpls)
		return;

	//Recover the ether(0)
	//ether_header = (cpc_eth_hdr_t*) ether(clas_state,0);
	
	pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_mpls_hdr_t));

	//Take header out
	pop_header(clas_state, HEADER_TYPE_MPLS, FIRST_MPLS_FRAME_POS, FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES);

	shift_ether(clas_state, 0, sizeof(cpc_mpls_hdr_t)); //shift right
	set_ether_type(get_ether_hdr(clas_state,0), ether_type);
	//ether_header->reset(ether_header->soframe(), current_length - sizeof(struct rofl::fmplsframe::mpls_hdr_t));
}
void pop_pppoe(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	cpc_eth_hdr_t* ether_header;
	
	// outermost mpls tag, if any, following immediately the initial ethernet header
	if(clas_state->num_of_headers[HEADER_TYPE_PPPOE] == 0 || !clas_state->headers[FIRST_PPPOE_FRAME_POS].present)
		return;

	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state,0);

	//WARNING check if get_ether_type returns NULL?
	switch (*get_ether_type(ether_header)) {
		case ETH_TYPE_PPPOE_DISCOVERY:
		{
			pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_pppoe_hdr_t));
			if (get_pppoe_hdr(clas_state, 0)) {
				//Take header out
				pop_header(clas_state, HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);

				pop_header(clas_state, HEADER_TYPE_PPP, FIRST_PPP_FRAME_POS, FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES);
			}
			shift_ether(clas_state, 0, sizeof(cpc_pppoe_hdr_t));// shift right
		}
			break;

		case ETH_TYPE_PPPOE_SESSION:
		{
			pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t),sizeof(cpc_pppoe_hdr_t) + sizeof(cpc_ppp_hdr_t));
			if (get_pppoe_hdr(clas_state, 0)) {
				pop_header(clas_state, HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);
			}
			if (get_ppp_hdr(clas_state, 0)) {
				pop_header(clas_state, HEADER_TYPE_PPP, FIRST_PPP_FRAME_POS, FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES);
			}
			shift_ether(clas_state, 0 ,sizeof(cpc_pppoe_hdr_t) + sizeof(cpc_ppp_hdr_t));//shift right
		}
		break;
	}

	set_ether_type(get_ether_hdr(clas_state,0), ether_type);
	//ether_header->reset(ether_header->soframe(), ether_header->framelen() - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
}

void pop_gtp(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	// assumption: UDP -> GTP

	// an ip header must be present
	if((clas_state->num_of_headers[HEADER_TYPE_IPV4] == 0) || (!clas_state->headers[FIRST_IPV4_FRAME_POS].present) || (clas_state->num_of_headers[HEADER_TYPE_IPV4] > 1))
		return;

	// a udp header must be present
	if((clas_state->num_of_headers[HEADER_TYPE_UDP] == 0) || (!clas_state->headers[FIRST_UDP_FRAME_POS].present) || (clas_state->num_of_headers[HEADER_TYPE_UDP] > 1))
		return;

	// a gtp header must be present
	if(clas_state->num_of_headers[HEADER_TYPE_GTP] == 0 || !clas_state->headers[FIRST_GTP_FRAME_POS].present)
		return;


	// determine effective length of GTP header
	size_t pop_length = sizeof(cpc_ipv4_hdr_t) + sizeof(cpc_udp_hdr_t);
	//gtp(0)->get_hdr_length(); // based on flags set to 1 in GTP header

	//Remove bytes from packet
	pkt_pop(pkt, get_ipv4_hdr(clas_state, 0), 0, pop_length);

	//Take headers out
	pop_header(clas_state, HEADER_TYPE_GTP, FIRST_GTP_FRAME_POS, FIRST_GTP_FRAME_POS+MAX_GTP_FRAMES);
	pop_header(clas_state, HEADER_TYPE_UDP, FIRST_UDP_FRAME_POS, FIRST_UDP_FRAME_POS+MAX_UDP_FRAMES);
	pop_header(clas_state, HEADER_TYPE_IPV4, FIRST_IPV4_FRAME_POS, FIRST_IPV4_FRAME_POS+MAX_IPV4_FRAMES);

	int i;
	for ( i = MAX_VLAN_FRAMES-1; i >= 0; ++i) {
		shift_vlan(clas_state, i, pop_length); //shift_right
		//vlan(i)->reset(vlan(i)->soframe(), vlan(i)->framelen() - pop_length);
	}
	for ( i = MAX_ETHER_FRAMES-1; i >= 0; ++i) {
		shift_ether(clas_state, i, pop_length);//shift_right
		//ether(i)->reset(ether(i)->soframe(), ether(i)->framelen() - pop_length);
	}

	if (get_vlan_hdr(clas_state, -1)) {
		set_vlan_type(get_vlan_hdr(clas_state, -1), ether_type);
	} else {
		set_ether_type(get_ether_hdr(clas_state, -1), ether_type);
	}
}

void push_header(classify_state_t* clas_state, enum header_type type, unsigned int start, unsigned int end){
	unsigned int i;
	header_container_t *header;

	if(clas_state->headers[start].present){
		header_container_t copy = clas_state->headers[end-1];
		
		//Rotate frames (push them)
		for(i=start;i<end-1;i++){
			clas_state->headers[i] = clas_state->headers[i+1];
		}

		clas_state->headers[start] = copy;
	}

	//Get the copy	
	header = &clas_state->headers[start];
	
	//Set presence
	header->present = true;
	//header->copy.prev = NULL; 
	
	//Increment header type counter	
	clas_state->num_of_headers[type]++;	
}

void* push_vlan(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	void* ether_header;
	//unsigned int current_length;

	if ((NULL == get_ether_hdr(clas_state, 0)) || clas_state->num_of_headers[HEADER_TYPE_VLAN] == MAX_VLAN_FRAMES ){
		return NULL;
	}
	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state, 0);
	//WARNING check if get_ether_type returns NULL?
	uint16_t inner_ether_type = *get_ether_type(ether_header);

	/*
	 * this invalidates ether(0), as it shifts ether(0) to the left
	 */
	if (pkt_push(pkt, NULL, sizeof(cpc_eth_hdr_t), sizeof(cpc_vlan_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}

	/*
	 * adjust ether(0): move one vlan tag to the left
	 */
	shift_ether(clas_state, 0, 0-sizeof(cpc_vlan_hdr_t)); // shift left
	ether_header-=sizeof(cpc_vlan_hdr_t); //We change also the local pointer
	
	/*
	 * append the new fvlanframe 
	 */
	push_header(clas_state, HEADER_TYPE_VLAN, FIRST_VLAN_FRAME_POS, FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES);
	
	//Now reset frame 
	clas_state->headers[FIRST_VLAN_FRAME_POS].frame = ether_header + sizeof(cpc_eth_hdr_t);
	clas_state->headers[FIRST_VLAN_FRAME_POS].length =  get_pkt_buffer_length(pkt) + sizeof(cpc_vlan_hdr_t) - sizeof(cpc_eth_hdr_t);
	//ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fvlanframe::vlan_hdr_t));
	//headers[FIRST_VLAN_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fvlanframe::vlan_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));

	/*
	 * set default values in vlan tag
	 */
	cpc_vlan_hdr_t* vlan_header = get_vlan_hdr(clas_state, 0);
	cpc_vlan_hdr_t* inner_vlan_header = get_vlan_hdr(clas_state, 1);
	
	//WARNING check if getters return NULL?
	if ( inner_vlan_header ) {
		set_vlan_id(vlan_header, *get_vlan_id(inner_vlan_header));
		set_vlan_pcp(vlan_header, *get_vlan_pcp(inner_vlan_header));
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
	//unsigned int current_length;

	if(!clas_state->is_classified || NULL == get_ether_hdr(clas_state, 0)){
		assert(0);	//classify(clas_state);
		return NULL;
	}
	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state, 0);
	//current_length = ether_header->framelen(); 
	
	/*
	 * this invalidates ether(0), as it shifts ether(0) to the left
	 */
	if (pkt_push(pkt, (void*)(ether_header + sizeof(cpc_eth_hdr_t)),0 , sizeof(cpc_mpls_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}

	/*
	 * adjust ether(0): move one mpls tag to the left
	 */
	shift_ether(clas_state, 0, 0-sizeof(cpc_mpls_hdr_t));//shift left
	ether_header-=sizeof(cpc_mpls_hdr_t); //We change also the local pointer

	/*
	 * append the new fmplsframe instance to ether(0)
	 */
	push_header(clas_state, HEADER_TYPE_MPLS, FIRST_MPLS_FRAME_POS, FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES);
	
	//Now reset frame
	clas_state->headers[FIRST_MPLS_FRAME_POS].frame = ether_header + sizeof(cpc_eth_hdr_t);
	//Size of ethernet needs to be extended with + MPLS size 
	//MPLS size needs to be ether_header->size + MPLS - ether_header
	clas_state->headers[FIRST_MPLS_FRAME_POS].length = get_pkt_buffer_length(pkt) + sizeof(cpc_mpls_hdr_t) - sizeof(cpc_eth_hdr_t);
	//ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fmplsframe::mpls_hdr_t));
	//headers[FIRST_MPLS_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fmplsframe::mpls_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));

	set_ether_type(ether_header, ether_type);
	/*
	 * set default values in mpls tag
	 */
	mpls_header = get_mpls_hdr(clas_state, 0);
	inner_mpls_header = get_mpls_hdr(clas_state, 1);
	cpc_ipv4_hdr_t *ipv4_hdr = get_ipv4_hdr(clas_state, 0);
	cpc_ipv4_hdr_t *ipv6_hdr = get_ipv6_hdr(clas_state, 0);

	//WARNING check if getters return NULL?
	if (inner_mpls_header){
		set_mpls_bos(mpls_header, false);
		set_mpls_label(mpls_header, *get_mpls_label(inner_mpls_header));
		set_mpls_tc(mpls_header, *get_mpls_tc(inner_mpls_header));
		set_mpls_ttl(mpls_header, *get_mpls_ttl(inner_mpls_header));
	} else {
		set_mpls_bos(mpls_header, true);
		set_mpls_label(mpls_header, 0x0000);
		set_mpls_tc(mpls_header, 0x00);
		if ( ipv4_hdr ){
			set_mpls_ttl(mpls_header, *get_ipv4_ttl(ipv4_hdr));
		}else if ( ipv6_hdr ){
			set_mpls_ttl(mpls_header, *get_ipv6_hop_limit(ipv6_hdr));
		}else
			set_mpls_ttl(mpls_header, 0x00);
	}

	return mpls_header;
}

void* push_pppoe(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	
	void* ether_header;
	//unsigned int current_length;

	if(!clas_state->is_classified || NULL == get_ether_hdr(clas_state, 0)){
		assert(0);	//classify(clas_state);
		return NULL;
	}
	
	if (get_pppoe_hdr(clas_state, 0)){
		// TODO: log error => pppoe tag already exists
		return NULL;
	}

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

			/*
			 * adjust ether(0): move one pppoe tag to the left
			 */
			shift_ether(clas_state, 0, 0-bytes_to_insert); // shift left
			ether_header-=bytes_to_insert; //We change also the local pointer
			set_ether_type(ether_header, ETH_TYPE_PPPOE_SESSION);

			/*
			 * append the new fpppoeframe instance to ether(0)
			 */
			push_header(clas_state, HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);
			push_header(clas_state, HEADER_TYPE_PPP, FIRST_PPP_FRAME_POS, FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES);
	
			//Now reset frames
			clas_state->headers[FIRST_PPPOE_FRAME_POS].frame = ether_header + sizeof(cpc_eth_hdr_t);
			clas_state->headers[FIRST_PPPOE_FRAME_POS].length = get_pkt_buffer_length(pkt) + sizeof(cpc_pppoe_hdr_t) - sizeof(cpc_eth_hdr_t);
			clas_state->headers[FIRST_PPP_FRAME_POS].frame = ether_header + sizeof(cpc_eth_hdr_t) + sizeof(cpc_pppoe_hdr_t);
			clas_state->headers[FIRST_PPP_FRAME_POS].length = get_pkt_buffer_length(pkt) + sizeof(cpc_ppp_hdr_t) - sizeof(cpc_eth_hdr_t) - sizeof(cpc_pppoe_hdr_t);
			//ether_header->reset(ether_header->soframe(), current_length + bytes_to_insert);
			//n_pppoe->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), ether_header->framelen() - sizeof(struct rofl::fetherframe::eth_hdr_t) );
			//n_ppp->reset(n_pppoe->soframe() + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t), n_pppoe->framelen() - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
	
			n_pppoe = (cpc_pppoe_hdr_t*)clas_state->headers[FIRST_PPPOE_FRAME_POS].frame;
			n_ppp = (cpc_ppp_hdr_t*)clas_state->headers[FIRST_PPP_FRAME_POS].frame;
			/*
			 * TODO: check if this is an appropiate fix 
			 */
			//TODO? n_pppoe->set_hdr_length(pkt->get_buffer_length() - sizeof(struct rofl::fetherframe::eth_hdr_t) - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
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

			/*
			 * adjust ether(0): move one pppoe tag to the left
			 */
			shift_ether(clas_state, 0, 0-bytes_to_insert);//shift left
			ether_header-=bytes_to_insert; //We change also the local pointer
			set_ether_type(get_ether_hdr(clas_state, 0), ETH_TYPE_PPPOE_DISCOVERY);

			/*
			 * append the new fpppoeframe instance to ether(0)
			 */
			push_header(clas_state, HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);
			//Now reset frame
			clas_state->headers[FIRST_PPPOE_FRAME_POS].frame = ether_header + sizeof(cpc_eth_hdr_t);
			//Size of ethernet needs to be extended with +PPPOE size 
			//PPPOE size needs to be ether_header->size + PPPOE - ether_header
			clas_state->headers[FIRST_PPPOE_FRAME_POS].length = get_pkt_buffer_length(pkt) + sizeof(cpc_pppoe_hdr_t) - sizeof(cpc_eth_hdr_t);
			//ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
			//headers[FIRST_PPPOE_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));

			n_pppoe = (cpc_pppoe_hdr_t*)clas_state->headers[FIRST_PPPOE_FRAME_POS].frame;


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
	ROFL_DEBUG(DRIVER_NAME" [c_pktclassifier] Dump packet state(%p) TODO!!\n",clas_state);
}

size_t get_pkt_len(datapacket_t* pkt, classify_state_t* clas_state, void *from, void *to){

	unsigned int total_length = get_pkt_buffer_length(pkt);
	void* eth = get_ether_hdr(clas_state, 0);

	if(!from)
		return total_length;

	if(!to)
		return (size_t)(total_length - (from - eth));

	return (size_t)(to - from);
}

