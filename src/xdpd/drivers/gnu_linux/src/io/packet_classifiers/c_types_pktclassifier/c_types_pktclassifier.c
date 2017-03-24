#include "c_types_pktclassifier.h"
#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/common/protocol_constants.h>
#include "../packet_operations.h"
#include "../../../config.h"

#include <utils/c_logger.h>

void pop_pbb(datapacket_t* pkt, classifier_state_t* clas_state){

	pkt_types_t new = PT_POP_PROTO(clas_state, ISID); //Use a nice PBB wrapper instead  
	if(unlikely(new == PT_INVALID))
		return;

	//Take header out from packet
	pkt_pop(pkt, NULL,0, sizeof(cpc_eth_hdr_t)+sizeof(cpc_pbb_isid_hdr_t));

	//Set new type and base(move right)
	clas_state->type = new; // NEU

	//reclassify
	parse_ethernet(clas_state, clas_state->base, clas_state->len);
}

void pop_vlan(datapacket_t* pkt, classifier_state_t* clas_state){

	pkt_types_t new = PT_POP_PROTO(clas_state, VLAN);  
	if(unlikely(new == PT_INVALID))
		return;

	cpc_vlan_hdr_t* vlan = get_vlan_hdr(clas_state,0);
	uint16_t ether_type = *get_vlan_type(vlan);
	
	//Take header out from packet
	pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_vlan_hdr_t));

	//Set new type and base(move right)
	clas_state->type = new;

	//Set ether_type of new frame
	set_ether_type(get_ether_hdr(clas_state,0),ether_type);
}
void pop_mpls(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){
	
	pkt_types_t new = PT_POP_PROTO(clas_state, MPLS);  
	if(unlikely(new == PT_INVALID))
		return;

	pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_mpls_hdr_t));

	//Set new type and base(move right)
	clas_state->type = new;

	set_ether_type(get_ether_hdr(clas_state,0), ether_type);

	//reclassify
	parse_ethernet(clas_state, clas_state->base, clas_state->len);
}

void pop_pppoe(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){
	cpc_eth_hdr_t* ether_header;
	
	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state,0);

	switch ( *get_ether_type(ether_header) ) {
		case ETH_TYPE_PPPOE_DISCOVERY:
		{
			pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_pppoe_hdr_t));
			pkt_types_t new = PT_POP_PROTO(clas_state, PPPOE);
			if(unlikely(new == PT_INVALID))
				return;
			//Set new type and base(move right)
			clas_state->type = new;
		}
			break;

		case ETH_TYPE_PPPOE_SESSION:
		{
			pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t),sizeof(cpc_pppoe_hdr_t) + sizeof(cpc_ppp_hdr_t));
			pkt_types_t new = PT_POP_PROTO(clas_state, PPP);
			if(unlikely(new == PT_INVALID))
				return;
			clas_state->type = new;
			new = PT_POP_PROTO(clas_state, PPPOE);
			if(unlikely(new == PT_INVALID))
				return;

			//Set new type and base(move right)
			clas_state->type = new;
		}
		break;
	}

	set_ether_type(get_ether_hdr(clas_state,0), ether_type);
	
	//re-classify packet to get the headers beyond pppoe
	classify_packet(clas_state, clas_state->base, clas_state->len, clas_state->port_in, clas_state->phy_port_in);
}

void* push_pbb(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){
	void *ether_header, *inner_ether_header;

	uint16_t offset = sizeof(cpc_eth_hdr_t)+sizeof(cpc_pbb_isid_hdr_t);
	pkt_types_t new = PT_PUSH_PROTO(clas_state, ISID); //Put a nice trace  
	if(unlikely(new == PT_INVALID))
		return NULL;

	inner_ether_header = get_ether_hdr(clas_state, 0);

	//Move bytes
	if (pkt_push(pkt, NULL, 0, offset) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}

	//Set new type and base(move left)
	clas_state->type = new;

	//Set new pkt type
	ether_header = get_ether_hdr(clas_state, 0);    
	
	set_ether_dl_dst(ether_header, *get_ether_dl_dst(inner_ether_header));
	set_ether_dl_src(ether_header, *get_ether_dl_src(inner_ether_header));
	//set_ether_type(ether_header, ether_type); //Never use set_ether_type => now is 0x0, and will interpret frame as 802.3
	*((uint16_t*)((uint8_t*)ether_header+12)) = ether_type; 

	return get_pbb_isid_hdr(clas_state,0);
}

void* push_vlan(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){

	uint8_t* ether_header;

	pkt_types_t new = PT_PUSH_PROTO(clas_state, VLAN);  
	if(unlikely(new == PT_INVALID))
		return NULL;

	//Recover the current ether(0)
	ether_header = get_ether_hdr(clas_state, 0);
	cpc_vlan_hdr_t* inner_vlan_header = get_vlan_hdr(clas_state, 0);
	uint16_t inner_ether_type = *get_ether_type(ether_header);

	//Move bytes
	if (pkt_push(pkt, NULL, sizeof(cpc_eth_hdr_t), sizeof(cpc_vlan_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}
	
	//Set new type and base(move left)
	clas_state->type = new;

	//Set new pkt type
	cpc_vlan_hdr_t* vlan_header = get_vlan_hdr(clas_state, 0);
	ether_header = get_ether_hdr(clas_state, 0);    
	
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

void* push_mpls(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){
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

	ether_header=get_ether_hdr(clas_state, 0);
	set_ether_type(ether_header, ether_type);
	
	//Set default values
	mpls_header = get_mpls_hdr(clas_state, 0);
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

void* push_pppoe(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){
	
	void* ether_header;
        size_t payload_length;
	//unsigned int current_length;
	pkt_types_t new = PT_PUSH_PROTO(clas_state, PPPOE);  
	if(unlikely(new == PT_INVALID))
		return NULL;

	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state, 0);
        payload_length = clas_state->len - sizeof(cpc_eth_hdr_t) + sizeof(cpc_ppp_hdr_t);
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
			clas_state->type = new;
			clas_state->type = PT_PUSH_PROTO(clas_state, PPP);
			assert(clas_state->type != PT_INVALID);

			/*
			 * adjust ether(0): move one pppoe tag to the left
			 */
			ether_header-=bytes_to_insert; //We change also the local pointer
			set_ether_type(ether_header, ETH_TYPE_PPPOE_SESSION);
			
			n_ppp = get_ppp_hdr(clas_state,0);
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

			set_ether_type(get_ether_hdr(clas_state, 0), ETH_TYPE_PPPOE_DISCOVERY);

			
		}
			break;
	}

	/*
	 * set default values in pppoe tag
	 */
	n_pppoe = get_pppoe_hdr(clas_state,0);
	set_pppoe_code(n_pppoe, 0x00);
	set_pppoe_sessid(n_pppoe, 0x0000);
	set_pppoe_type(n_pppoe, PPPOE_TYPE);
	set_pppoe_vers(n_pppoe, PPPOE_VERSION);
        set_pppoe_length(n_pppoe, htobe16(payload_length));

	return NULL;
}

void* push_gtp(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){

	void* ether_header;
	cpc_gtphu_t* gtp_header = (cpc_gtphu_t*)0;
	cpc_udp_hdr_t* udp_header = (cpc_udp_hdr_t*)0;
	cpc_ipv4_hdr_t* ipv4_header = (cpc_ipv4_hdr_t*)0;
	cpc_ipv6_hdr_t* ipv6_header = (cpc_ipv6_hdr_t*)0;
	uint8_t ip_ttl = 0;
	size_t payloadlen = 0;
	int DF_flag = false;

	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state, 0);

	//retrieve the current payload length
	uint16_t* current_ether_type = get_ether_type(ether_header);
	switch (*current_ether_type) {
	case ETH_TYPE_IPV4:{
		if ((ipv4_header = get_ipv4_hdr(clas_state, 0)) != NULL) {
			payloadlen = be16toh(*get_ipv4_length(ipv4_header)); // no options supported,
			// contrary to IPv6, IPv4 length field includes IPv4 header itself
			ip_ttl = *get_ipv4_ttl(ipv4_header);
			DF_flag = has_ipv4_DF_bit_set(ipv4_header);
		}else{
			return NULL;
		}
	}break;
	case ETH_TYPE_IPV6:{
		if ((ipv6_header = get_ipv6_hdr(clas_state, 0)) != NULL) {
			payloadlen = be16toh(*get_ipv6_payload_length(ipv6_header)) + sizeof(cpc_ipv6_hdr_t); // no extension headers
			ip_ttl = *get_ipv6_hop_limit(ipv6_header);
		} else {
			return NULL;
		}
	}break;
	default:{
		return NULL;
	};
	}

	//ether_type defines the new header to be pushed: IPV4/UDP/GTPU or IPV6/UDP/GTPU
	switch (ether_type) {
		case ETH_TYPE_IPV4:
		{
			//unsigned int offset = get_ipv4_hdr(clas_state, 0) - get_ether_hdr(clas_state, 0);
			unsigned int offset = sizeof(cpc_eth_hdr_t);
			unsigned int bytes_to_insert = sizeof(cpc_ipv4_hdr_t) +
											sizeof(cpc_udp_hdr_t) +
											sizeof(cpc_gtpu_base_hdr_t);
			uint16_t ident = *get_ipv4_ident(get_ipv4_hdr(clas_state, 0));
			//payloadlen = clas_state->len - offset;

			pkt_types_t new = PT_PUSH_PROTO(clas_state, GTPU4);
			if(unlikely(new == PT_INVALID))
				return NULL;

			/*
			 * this invalidates ether(0), as it shifts ether(0) to the left
			 */
			if (pkt_push(pkt, NULL, offset, bytes_to_insert) == ROFL_FAILURE){
				// TODO: log error
				return NULL;
			}
			//Set new pkt type
			clas_state->type = new;
			//clas_state->base -= sizeof(cpc_ipv4_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);
			//clas_state->len += sizeof(cpc_ipv4_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);
			assert(clas_state->type != PT_INVALID);

			/*
			 * adjust ether(0): move one IPV4/UDP/GTPU tag to the left
			 */
			ether_header-=bytes_to_insert; //We change also the local pointer
			set_ether_type(ether_header, ETH_TYPE_IPV4);

			ipv4_header = (cpc_ipv4_hdr_t*)get_ipv4_hdr(clas_state, 0);
			set_ipv4_version(ipv4_header, 4 << 4);
			set_ipv4_ihl(ipv4_header, sizeof(cpc_ipv4_hdr_t)/sizeof(uint32_t));
			set_ipv4_dscp(ipv4_header, 0);
			set_ipv4_ecn(ipv4_header, 0);
			set_ipv4_length(ipv4_header, htobe16(sizeof(cpc_ipv4_hdr_t) +
													sizeof(cpc_udp_hdr_t) +
													sizeof(cpc_gtpu_base_hdr_t) +
													payloadlen));
			set_ipv4_proto(ipv4_header, IP_PROTO_UDP);
			set_ipv4_ident(ipv4_header, ident);
			set_ipv4_src(ipv4_header, 0);
			set_ipv4_dst(ipv4_header, 0);
			set_ipv4_ttl(ipv4_header, ip_ttl);
			if (DF_flag) {
				set_ipv4_DF_bit(ipv4_header);
			}

			set_recalculate_checksum(clas_state, RECALCULATE_IPV4_CHECKSUM_IN_SW);

		}
			break;
		case ETH_TYPE_IPV6:
		{
			//unsigned int offset = get_ipv6_hdr(clas_state, 0) - get_ether_hdr(clas_state, 0);
			unsigned int offset = sizeof(cpc_eth_hdr_t);
			unsigned int bytes_to_insert = sizeof(cpc_ipv6_hdr_t) +
											sizeof(cpc_udp_hdr_t) +
											sizeof(cpc_gtpu_base_hdr_t);
			//payloadlen = clas_state->len - offset;

			pkt_types_t new = PT_PUSH_PROTO(clas_state, GTPU6);
			if(unlikely(new == PT_INVALID))
				return NULL;

			/*
			 * this invalidates ether(0), as it shifts ether(0) to the left
			 */
			if (pkt_push(pkt, NULL, offset, bytes_to_insert) == ROFL_FAILURE){
				// TODO: log error
				return NULL;
			}
			//Set new pkt type
			clas_state->type = new;
			//clas_state->base -= sizeof(cpc_ipv6_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(struct cpc_gtpu_base_hdr_t);
			//clas_state->len += sizeof(cpc_ipv6_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(struct cpc_gtpu_base_hdr_t);
			assert(clas_state->type != PT_INVALID);

			/*
			 * adjust ether(0): move one IPV6/UDP/GTPU tag to the left
			 */
			ether_header-=bytes_to_insert; //We change also the local pointer
			set_ether_type(ether_header, ETH_TYPE_IPV6);

			uint128__t null_addr;
			memset(null_addr.val, 0, sizeof(null_addr));

			ipv6_header = (cpc_ipv6_hdr_t*)get_ipv6_hdr(clas_state, 0);
			set_ipv6_version(ipv6_header, 6 << 4);
			set_ipv6_dscp(ipv6_header, 0);
			set_ipv6_ecn(ipv6_header, 0);
			set_ipv6_dst(ipv6_header, null_addr);
			set_ipv6_src(ipv6_header, null_addr);
			set_ipv6_flow_label(ipv6_header, 0);
			set_ipv6_hop_limit(ipv6_header, ip_ttl);
			set_ipv6_payload_length(ipv6_header, htobe16(sizeof(cpc_udp_hdr_t) +
															sizeof(cpc_gtpu_base_hdr_t) +
															payloadlen));
			set_ipv6_traffic_class(ipv6_header, 0);
			set_ipv6_next_header(ipv6_header, IP_PROTO_UDP);

			// no checksum in IPv6
		}
			break;
	}

	/*
	 * set default values in UDP tag
	 */
	udp_header = get_udp_hdr(clas_state,0);
	set_udp_dport(udp_header, UDP_DST_PORT_GTPU); // necessary for re-classifying this packet (see below)
	set_udp_sport(udp_header, UDP_DST_PORT_GTPU);
	set_udp_length(udp_header, htobe16(sizeof(cpc_udp_hdr_t) +
										sizeof(cpc_gtpu_base_hdr_t) +
										payloadlen));
	set_recalculate_checksum(clas_state, RECALCULATE_UDP_CHECKSUM_IN_SW);

	/*
	 * set default values in GTPU tag
	 */
	gtp_header = get_gtpu_hdr(clas_state,0);
	set_gtpu_version(gtp_header, /*GTPU_VERSION_1=*/				(1 << 5));
	set_gtpu_pt_flag(gtp_header, /*PT flag at bit 4=*/				(1 << 4));
	set_gtpu_e_flag(gtp_header, /*extension header flag at bit 2=*/	 0);
	set_gtpu_s_flag(gtp_header, /*seqno flag at bit 1=*/			 0);
	set_gtpu_pn_flag(gtp_header, /*n-pdu number flag at bit 0=*/	 0);
	set_gtpu_msg_type(gtp_header, 0);
	set_gtpu_length(gtp_header, htobe16(payloadlen));
	set_gtpu_teid(gtp_header, 0);
	//we only push the GTP base header here
	//set_gtpu_seq_no(gtp_header, 0);
	//set_gtpu_npdu_no(gtp_header, 0);

	// do not re-classify, unnecessary

	return NULL;
}

void pop_gtp(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){

	cpc_eth_hdr_t* ether_header = (cpc_eth_hdr_t*)0;
	cpc_gtpu_base_hdr_t* gtp_header = (cpc_gtpu_base_hdr_t*)0;
	uint16_t* current_ether_type = (uint16_t*)0;

	ether_header = get_ether_hdr(clas_state, 0);
	if (!ether_header) {
		// TODO: log error
		return;
	}

	gtp_header = get_gtpu_hdr(clas_state, 0);
	if (!gtp_header) {
		// TODO: log error
		return;
	}

	current_ether_type = get_ether_type(ether_header);
	if (!current_ether_type) {
		// TODO: log error
		return;
	}
	switch (*current_ether_type) {
	case ETH_TYPE_IPV4: {
		pkt_types_t new = PT_POP_PROTO(clas_state, GTPU4);
		if(unlikely(new == PT_INVALID))
			return;

		//Take header out from packet
		pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_ipv4_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtpu_base_hdr_t));

		//Set new type and base(move right)
		clas_state->type = new;
		// TODO: question: setting clas_state->base/len seems to be necessary for DPDK, but
		// gnu_linux is manipulating these parameters automatically.
		//clas_state->base += sizeof(cpc_ipv4_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);
		//clas_state->len -= sizeof(cpc_ipv4_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);

	} break;
	case ETH_TYPE_IPV6: {
		pkt_types_t new = PT_POP_PROTO(clas_state, GTPU6);
		if(unlikely(new == PT_INVALID))
			return;

		//Take header out from packet
		pkt_pop(pkt, NULL,/*offset=*/sizeof(cpc_eth_hdr_t), sizeof(cpc_ipv6_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtpu_base_hdr_t));

		//Set new type and base(move right)
		clas_state->type = new;
		//clas_state->base += sizeof(cpc_ipv6_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);
		//clas_state->len -= sizeof(cpc_ipv6_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);

	} break;
	}

	//Set ether_type of new frame
	set_ether_type(get_ether_hdr(clas_state,0),ether_type);

	//reclassify
	//classify_packet(clas_state, clas_state->base, clas_state->len, clas_state->port_in, clas_state->phy_port_in);
	parse_ethernet(clas_state, clas_state->base, clas_state->len);
}

void* push_gre(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){

	void* ether_header;
	cpc_gre_hdr_t* gre_header = (cpc_gre_hdr_t*)0;
	cpc_ipv4_hdr_t* ipv4_header = (cpc_ipv4_hdr_t*)0;
	cpc_ipv6_hdr_t* ipv6_header = (cpc_ipv6_hdr_t*)0;
	uint8_t ip_ttl = 64;
	size_t payloadlen = clas_state->len;
	int DF_flag = true;

	//Recover the ether(0)
	ether_header = get_ether_hdr(clas_state, 0);
	uint64_t dl_dst = *get_ether_dl_dst(ether_header);
	uint64_t dl_src = *get_ether_dl_src(ether_header);

	//ether_type defines the new header to be pushed: IPV4/UDP/GTPU or IPV6/UDP/GTPU
	switch (ether_type) {
		case ETH_TYPE_IPV4:
		{
			//unsigned int offset = get_ipv4_hdr(clas_state, 0) - get_ether_hdr(clas_state, 0);
			unsigned int offset = 0;
			unsigned int bytes_to_insert = sizeof(cpc_eth_hdr_t) +
											sizeof(cpc_ipv4_hdr_t) +
											sizeof(cpc_gre_base_hdr_t) + 2*sizeof(uint32_t); // basehdr+csum+key, no seqno!

			pkt_types_t new = PT_PUSH_PROTO(clas_state, GRE4);
			if(unlikely(new == PT_INVALID))
				return NULL;

			/*
			 * this invalidates ether(0), as it shifts ether(0) to the left
			 */
			if (pkt_push(pkt, NULL, offset, bytes_to_insert) == ROFL_FAILURE){
				// TODO: log error
				return NULL;
			}
			//Set new pkt type
			clas_state->type = new;
			//clas_state->base -= sizeof(cpc_ipv4_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);
			//clas_state->len += sizeof(cpc_ipv4_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);
			assert(clas_state->type != PT_INVALID);

			/*
			 * adjust ether(0): move one IPV4/UDP/GTPU tag to the left
			 */
			ether_header = get_ether_hdr(clas_state, 0);
			set_ether_dl_dst(ether_header, dl_dst);
			set_ether_dl_src(ether_header, dl_src);
			//Never use set_ether_type => now is 0x0, and will interpret frame as 802.3
			*((uint16_t*)((uint8_t*)ether_header+12)) = ether_type;

			ipv4_header = (cpc_ipv4_hdr_t*)get_ipv4_hdr(clas_state, 0);
			set_ipv4_version(ipv4_header, 4 << 4);
			set_ipv4_ihl(ipv4_header, sizeof(cpc_ipv4_hdr_t)/sizeof(uint32_t));
			set_ipv4_dscp(ipv4_header, 0);
			set_ipv4_ecn(ipv4_header, 0);
			set_ipv4_length(ipv4_header, htobe16(sizeof(cpc_ipv4_hdr_t) +
													sizeof(cpc_gre_base_hdr_t) + 2*sizeof(uint32_t) + // basehdr+csum+key, no seqno!
													payloadlen));
			set_ipv4_proto(ipv4_header, IP_PROTO_GRE);
			set_ipv4_ident(ipv4_header, 0);
			set_ipv4_src(ipv4_header, 0);
			set_ipv4_dst(ipv4_header, 0);
			set_ipv4_ttl(ipv4_header, ip_ttl);
			if (DF_flag) {
				set_ipv4_DF_bit(ipv4_header);
			}

			set_recalculate_checksum(clas_state, RECALCULATE_IPV4_CHECKSUM_IN_SW);

		}
			break;
		case ETH_TYPE_IPV6:
		{
			//unsigned int offset = get_ipv6_hdr(clas_state, 0) - get_ether_hdr(clas_state, 0);
			unsigned int offset = 0;
			unsigned int bytes_to_insert = sizeof(cpc_eth_hdr_t) +
											sizeof(cpc_ipv6_hdr_t) +
											sizeof(cpc_gre_base_hdr_t) + 2*sizeof(uint32_t); // basehdr+csum+key, no seqno!
			//payloadlen = clas_state->len - offset;

			pkt_types_t new = PT_PUSH_PROTO(clas_state, GRE6);
			if(unlikely(new == PT_INVALID))
				return NULL;

			/*
			 * this invalidates ether(0), as it shifts ether(0) to the left
			 */
			if (pkt_push(pkt, NULL, offset, bytes_to_insert) == ROFL_FAILURE){
				// TODO: log error
				return NULL;
			}
			//Set new pkt type
			clas_state->type = new;
			//clas_state->base -= sizeof(cpc_ipv6_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(struct cpc_gtpu_base_hdr_t);
			//clas_state->len += sizeof(cpc_ipv6_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(struct cpc_gtpu_base_hdr_t);
			assert(clas_state->type != PT_INVALID);

			/*
			 * adjust ether(0): move one IPV6/UDP/GTPU tag to the left
			 */
			ether_header = get_ether_hdr(clas_state, 0);
			set_ether_dl_dst(ether_header, dl_dst);
			set_ether_dl_src(ether_header, dl_src);
			//Never use set_ether_type => now is 0x0, and will interpret frame as 802.3
			*((uint16_t*)((uint8_t*)ether_header+12)) = ether_type;

			uint128__t null_addr;
			memset(null_addr.val, 0, sizeof(null_addr));

			ipv6_header = (cpc_ipv6_hdr_t*)get_ipv6_hdr(clas_state, 0);
			set_ipv6_version(ipv6_header, 6 << 4);
			set_ipv6_dscp(ipv6_header, 0);
			set_ipv6_ecn(ipv6_header, 0);
			set_ipv6_dst(ipv6_header, null_addr);
			set_ipv6_src(ipv6_header, null_addr);
			set_ipv6_flow_label(ipv6_header, 0);
			set_ipv6_hop_limit(ipv6_header, ip_ttl);
			set_ipv6_payload_length(ipv6_header, htobe16(sizeof(cpc_gre_base_hdr_t) + 2*sizeof(uint32_t) + payloadlen)); // basehdr+csum+key, no seqno!
			set_ipv6_traffic_class(ipv6_header, 0);
			set_ipv6_next_header(ipv6_header, IP_PROTO_GRE);

			// no checksum in IPv6
		}
			break;
	}

	/*
	 * set default values in GRE tag
	 */
	gre_header = get_gre_hdr(clas_state,0);
	set_gre_version(gre_header, 0);
	set_gre_csum_flag(gre_header, GRE_CSUM_FLAG_MASK);
	set_gre_key_flag(gre_header, GRE_KEY_FLAG_MASK);
	set_gre_seqno_flag(gre_header, 0); // no seqno!
	set_gre_prot_type(gre_header, GRE_PROT_TYPE_TRANSPARENT_ETHERNET_BRIDGING); // 0x6558, TODO: support IP over GRE
	set_gre_key(gre_header, 0);

	set_recalculate_checksum(clas_state, RECALCULATE_GRE_CHECKSUM_IN_SW);

	// do not re-classify, unnecessary

	return NULL;
}

void pop_gre(datapacket_t* pkt, classifier_state_t* clas_state, uint16_t ether_type){

	cpc_eth_hdr_t* ether_header = (cpc_eth_hdr_t*)0;
	cpc_gre_hdr_t* gre_header = (cpc_gre_hdr_t*)0;
	uint16_t* current_ether_type = (uint16_t*)0;

	ether_header = get_ether_hdr(clas_state, 0);
	if (!ether_header) {
		// TODO: log error
		return;
	}

	gre_header = get_gre_hdr(clas_state, 0);
	if (!gre_header) {
		// TODO: log error
		return;
	}

	if (unlikely(*get_gre_prot_type(gre_header) != GRE_PROT_TYPE_TRANSPARENT_ETHERNET_BRIDGING))
		return;

	current_ether_type = get_ether_type(ether_header);
	if (!current_ether_type) {
		// TODO: log error
		return;
	}
	switch (*current_ether_type) {
	case ETH_TYPE_IPV4: {
		pkt_types_t new = PT_POP_PROTO(clas_state, GRE4);
		if(unlikely(new == PT_INVALID))
			return;

		// TODO: check for RFC1701 compatibility mode

		unsigned int num_of_bytes = sizeof(cpc_eth_hdr_t) + sizeof(cpc_ipv4_hdr_t) + sizeof(cpc_gre_base_hdr_t);

		if ((*get_gre_csum_flag(gre_header)) & GRE_CSUM_FLAG_MASK) {
			num_of_bytes += sizeof(uint32_t);
		}
		if ((*get_gre_key_flag(gre_header)) & GRE_KEY_FLAG_MASK) {
			num_of_bytes += sizeof(uint32_t);
		}
		if ((*get_gre_seqno_flag(gre_header)) & GRE_SEQNO_FLAG_MASK) {
			num_of_bytes += sizeof(uint32_t);
		}

		//Take header out from packet
		pkt_pop(pkt, NULL,/*offset=*/0, num_of_bytes);

		//Set new type and base(move right)
		clas_state->type = new;
		// TODO: question: setting clas_state->base/len seems to be necessary for DPDK, but
		// gnu_linux is manipulating these parameters automatically.
		//clas_state->base += sizeof(cpc_ipv4_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);
		//clas_state->len -= sizeof(cpc_ipv4_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);

	} break;
	case ETH_TYPE_IPV6: {
		pkt_types_t new = PT_POP_PROTO(clas_state, GRE6);
		if(unlikely(new == PT_INVALID))
			return;

		// TODO: check for RFC1701 compatibility mode

		unsigned int num_of_bytes = sizeof(cpc_eth_hdr_t) + sizeof(cpc_ipv6_hdr_t) + sizeof(cpc_gre_base_hdr_t);

		if ((*get_gre_csum_flag(gre_header)) & GRE_CSUM_FLAG_MASK) {
			num_of_bytes += sizeof(uint32_t);
		}
		if ((*get_gre_key_flag(gre_header)) & GRE_KEY_FLAG_MASK) {
			num_of_bytes += sizeof(uint32_t);
		}
		if ((*get_gre_seqno_flag(gre_header)) & GRE_SEQNO_FLAG_MASK) {
			num_of_bytes += sizeof(uint32_t);
		}

		//Take header out from packet
		pkt_pop(pkt, NULL,/*offset=*/0, num_of_bytes);

		//Set new type and base(move right)
		clas_state->type = new;
		//clas_state->base += sizeof(cpc_ipv6_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);
		//clas_state->len -= sizeof(cpc_ipv6_hdr_t)+sizeof(cpc_udp_hdr_t)+sizeof(cpc_gtphu_t);

	} break;
	}

	//reclassify
	//classify_packet(clas_state, clas_state->base, clas_state->len, clas_state->port_in, clas_state->phy_port_in);
	parse_ethernet(clas_state, clas_state->base, clas_state->len);
}

void dump_pkt_classifier(classifier_state_t* clas_state){
	XDPD_DEBUG(DRIVER_NAME" [c_types_pktclassifier] Dump packet state(%p) TODO!!\n",clas_state);
}

size_t get_pkt_len(datapacket_t* pkt, classifier_state_t* clas_state, void *from, void *to){
	assert(to == NULL);	
	return clas_state->len -( ((uint8_t*)from) - clas_state->base);
}
