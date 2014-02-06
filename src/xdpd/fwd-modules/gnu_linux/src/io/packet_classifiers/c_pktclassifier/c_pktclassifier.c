#include "c_pktclassifier.h"
#include <stdlib.h>
#include <string.h>
#include <rofl/common/utils/c_logger.h>
#include "../packet_operations.h"

void parse_ethernet(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_vlan(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_mpls(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_pppoe(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_ppp(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_arpv4(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_ipv4(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_icmpv4(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_ipv6(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_icmpv6(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_tcp(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_udp(classify_state_t* clas_state, uint8_t *data, size_t datalen);
void parse_gtp(classify_state_t* clas_state, uint8_t *data, size_t datalen);


/// Classify part
classify_state_t* init_classifier(datapacket_t*const  pkt){

	classify_state_t* classifier = malloc(sizeof(classify_state_t));
	memset(classifier,0,sizeof(classify_state_t));

	assert(pkt != NULL);
	classifier->matches = &pkt->matches;

	return classifier;
}
void destroy_classifier(classify_state_t* clas_state){
	free(clas_state);
}

void classify_packet(classify_state_t* clas_state, uint8_t* data, size_t len, uint32_t port_in, uint32_t phy_port_in){
	if(clas_state->is_classified)
		reset_classifier(clas_state);
	parse_ethernet(clas_state, data, len);
	clas_state->is_classified = true;
	
	//Fill in the matches
	clas_state->matches->pkt_size_bytes = len;
	clas_state->matches->port_in = port_in;
	clas_state->matches->phy_port_in = phy_port_in;
}

void reset_classifier(classify_state_t* clas_state){

	packet_matches_t* matches = clas_state->matches;
	memset(clas_state,0,sizeof(classify_state_t));
	clas_state->matches = matches;

	if(likely(matches != NULL))
		memset(clas_state->matches,0,sizeof(packet_matches_t));
}

void parse_ethernet(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_eth_hdr_t))){return;}

	//Data pointer	
	cpc_eth_hdr_t* ether = (cpc_eth_hdr_t *)data;

	//Set frame
	unsigned int num_of_ether = clas_state->num_of_headers[HEADER_TYPE_ETHER];
	clas_state->headers[FIRST_ETHER_FRAME_POS + num_of_ether].frame = ether;
	clas_state->headers[FIRST_ETHER_FRAME_POS + num_of_ether].present = true;
	clas_state->headers[FIRST_ETHER_FRAME_POS + num_of_ether].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_ETHER] = num_of_ether+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_eth_hdr_t);
	datalen -= sizeof(cpc_eth_hdr_t);

	clas_state->eth_type = get_ether_type(ether);

	//Initialize eth packet matches
	clas_state->matches->eth_type = get_ether_type(ether); //This MUST be here
	clas_state->matches->eth_src = get_ether_dl_src(ether);
	clas_state->matches->eth_dst = get_ether_dl_dst(ether);

	switch (clas_state->eth_type) {
		case VLAN_CTAG_ETHER:
		case VLAN_STAG_ETHER:
		case VLAN_ITAG_ETHER:
			{
				parse_vlan(clas_state, data, datalen);
			}
			break;
		case MPLS_ETHER:
		case MPLS_ETHER_UPSTREAM:
			{
				parse_mpls(clas_state, data, datalen);
			}
			break;
		case PPPOE_ETHER_DISCOVERY:
		case PPPOE_ETHER_SESSION:
			{
				parse_pppoe(clas_state, data, datalen);
			}
			break;
		case ARPV4_ETHER:
			{
				parse_arpv4(clas_state, data, datalen);
			}
			break;
		case IPV4_ETHER:
			{
				parse_ipv4(clas_state, data, datalen);
			}
			break;
		case IPV6_ETHER:
			{
				parse_ipv6(clas_state, data,datalen);
			}
			break;
		default:
			{
				
			}
			break;
	}


}

void parse_vlan(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_vlan_hdr_t))) { return; }

	//Data pointer	
	cpc_vlan_hdr_t* vlan = (cpc_vlan_hdr_t *)data;

	//Set frame
	unsigned int num_of_vlan = clas_state->num_of_headers[HEADER_TYPE_VLAN];
	clas_state->headers[FIRST_VLAN_FRAME_POS + num_of_vlan].frame = vlan;
	clas_state->headers[FIRST_VLAN_FRAME_POS + num_of_vlan].present = true;
	clas_state->headers[FIRST_VLAN_FRAME_POS + num_of_vlan].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_VLAN] = num_of_vlan+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_vlan_hdr_t);
	datalen -= sizeof(cpc_vlan_hdr_t);

	clas_state->eth_type = get_vlan_type(vlan);

	switch (clas_state->eth_type) {
		case VLAN_CTAG_ETHER:
		case VLAN_STAG_ETHER:
		case VLAN_ITAG_ETHER:
			{
				parse_vlan(clas_state, data, datalen);
			}
			break;
		case MPLS_ETHER:
		case MPLS_ETHER_UPSTREAM:
			{
				parse_mpls(clas_state, data, datalen);
			}
			break;
		case PPPOE_ETHER_DISCOVERY:
		case PPPOE_ETHER_SESSION:
			{
				parse_pppoe(clas_state, data, datalen);
			}
			break;
		case ARPV4_ETHER:
			{
				parse_arpv4(clas_state, data, datalen);
			}
			break;
		case IPV4_ETHER:
			{
				parse_ipv4(clas_state, data, datalen);
			}
			break;
		default:
			{

			}
			break;
	}

	//Initialize vlan packet matches
	clas_state->matches->has_vlan = true;
	clas_state->matches->eth_type = get_vlan_type(vlan);
	clas_state->matches->vlan_vid = get_vlan_id(vlan);
	clas_state->matches->vlan_pcp = get_vlan_pcp(vlan);
}

void parse_mpls(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	
	if (unlikely(datalen < sizeof(cpc_mpls_hdr_t))) { return; }

	cpc_mpls_hdr_t* mpls = (cpc_mpls_hdr_t*)data;
	
	//Set frame
	unsigned int num_of_mpls = clas_state->num_of_headers[HEADER_TYPE_MPLS];
	clas_state->headers[FIRST_MPLS_FRAME_POS + num_of_mpls].frame = mpls;
	clas_state->headers[FIRST_MPLS_FRAME_POS + num_of_mpls].present = true;
	clas_state->headers[FIRST_MPLS_FRAME_POS + num_of_mpls].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_MPLS] = num_of_mpls+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_mpls_hdr_t);
	datalen -= sizeof(cpc_mpls_hdr_t);

	if (! get_mpls_bos(mpls)){

		parse_mpls(clas_state,data, datalen);

	}else{
		
		//TODO: We could be trying to guess if payload is IPv4/v6 and continue parsing...
	}

	//Initialize mpls packet matches
	clas_state->matches->mpls_bos = get_mpls_bos(mpls);
	clas_state->matches->mpls_label = get_mpls_label(mpls); 
	clas_state->matches->mpls_tc = get_mpls_tc(mpls); 
}
void parse_pppoe(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_pppoe_hdr_t))) { return; }

	cpc_pppoe_hdr_t* pppoe = (cpc_pppoe_hdr_t*)data;

	//Set frame
	unsigned int num_of_pppoe = clas_state->num_of_headers[HEADER_TYPE_PPPOE];
	clas_state->headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].frame = pppoe;
	clas_state->headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].present = true;
	clas_state->headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_PPPOE] = num_of_pppoe+1;
	
	switch (clas_state->eth_type) {
		case PPPOE_ETHER_DISCOVERY:
			{
				datalen -= sizeof(cpc_pppoe_hdr_t);
#if 0
//TODO?
				uint16_t pppoe_len = get_pppoe_length(pppoe) > datalen ? datalen : get_pppoe_length(pppoe);

				/*
				 * parse any pppoe service tags
				 */
				pppoe->unpack(data, sizeof(cpc_pppoe_hdr_t) + pppoe_len);


				/*
				 * any remaining bytes after the pppoe tags => padding?
				 */
				if (datalen > pppoe->tags.length())
				{
					//TODO?: Continue parsing??	
				}
#endif
			}
			break;
		case PPPOE_ETHER_SESSION:
			{
				//Increment pointers and decrement remaining payload size
				data += sizeof(cpc_pppoe_hdr_t);
				datalen -= sizeof(cpc_pppoe_hdr_t);

				parse_ppp(clas_state,data, datalen);
			}
			break;
		default:
			{
				// log error?
			}
			break;
	}

	//Initialize pppoe packet matches
	clas_state->matches->pppoe_code = get_pppoe_code(pppoe);
	clas_state->matches->pppoe_type = get_pppoe_type(pppoe);
	clas_state->matches->pppoe_sid = get_pppoe_sessid(pppoe);
}

void parse_ppp(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	
	if (unlikely(datalen < sizeof(cpc_ppp_hdr_t))) { return; }

	cpc_ppp_hdr_t* ppp = (cpc_ppp_hdr_t*)data;
	
	//Set frame
	unsigned int num_of_ppp = clas_state->num_of_headers[HEADER_TYPE_PPP];
	clas_state->headers[FIRST_PPP_FRAME_POS + num_of_ppp].frame = ppp; 
	clas_state->headers[FIRST_PPP_FRAME_POS + num_of_ppp].present = true;
	clas_state->headers[FIRST_PPP_FRAME_POS + num_of_ppp].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_PPP] = num_of_ppp+1;

	//Increment pointers and decrement remaining payload size
	switch (get_ppp_prot(ppp)) {
		case PPP_PROT_IPV4:
			{
				//Increment pointers and decrement remaining payload size
				data += sizeof(cpc_ppp_hdr_t);
				datalen -= sizeof(cpc_ppp_hdr_t);

				parse_ipv4(clas_state, data, datalen);
			}
			break;
		default:
			{
				//TODO? ppp->unpack(data, datalen);
			}
			break;
	}

	//Initialize ppp packet matches
	clas_state->matches->ppp_proto = get_ppp_prot(ppp);
}

void parse_arpv4(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	
	if (unlikely(datalen < sizeof(cpc_arpv4_hdr_t))) { return; }
	
	cpc_arpv4_hdr_t* arpv4 = (cpc_arpv4_hdr_t*)data;

	//Set frame
	unsigned int num_of_arpv4 = clas_state->num_of_headers[HEADER_TYPE_ARPV4];
	clas_state->headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].frame = arpv4;
	clas_state->headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].present = true;
	clas_state->headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_ARPV4] = num_of_arpv4+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_arpv4_hdr_t);
	datalen -= sizeof(cpc_arpv4_hdr_t);

	if (datalen > 0){
		//TODO: something?
	}

	//Initialize arpv4 packet matches
	clas_state->matches->arp_opcode = get_arpv4_opcode(arpv4);
	clas_state->matches->arp_sha =  get_arpv4_dl_dst(arpv4);
	clas_state->matches->arp_spa =  get_arpv4_ip_src(arpv4);
	clas_state->matches->arp_tha =  get_arpv4_dl_dst(arpv4);
	clas_state->matches->arp_tpa =  get_arpv4_ip_dst(arpv4);
}

void parse_ipv4(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	if (unlikely(datalen < sizeof(cpc_ipv4_hdr_t))) { return; }
	
	//Set reference
	cpc_ipv4_hdr_t *ipv4 = (cpc_ipv4_hdr_t*)data; 

	//Set frame
	unsigned int num_of_ipv4 = clas_state->num_of_headers[HEADER_TYPE_IPV4];
	clas_state->headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].frame = ipv4;
	clas_state->headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].present = true;
	clas_state->headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_IPV4] = num_of_ipv4+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_ipv4_hdr_t);
	datalen -= sizeof(cpc_ipv4_hdr_t);

	if (has_ipv4_MF_bit_set(ipv4)){
		// TODO: fragment handling

		return;
	}

	// FIXME: IP header with options


	switch (get_ipv4_proto(ipv4)) {
		case IPV4_IP_PROTO:
			{
				parse_ipv4(clas_state, data, datalen);
			}
			break;
		case ICMPV4_IP_PROTO:
			{
				parse_icmpv4(clas_state, data, datalen);
			}
			break;
		case UDP_IP_PROTO:
			{
				parse_udp(clas_state, data, datalen);
			}
			break;
		case TCP_IP_PROTO:
			{
				parse_tcp(clas_state, data, datalen);
			}
			break;
#if 0
		case SCTP_IP_PROTO:
			{
				parse_sctp(clas_state, data, datalen);
			}
			break;
#endif
		default:
			{
			
			}
			break;
	}

	//Initialize ipv4 packet matches
	clas_state->matches->ip_proto = get_ipv4_proto(ipv4);
	clas_state->matches->ip_dscp = get_ipv4_dscp(ipv4);
	clas_state->matches->ip_ecn = get_ipv4_ecn(ipv4);
	clas_state->matches->ipv4_src = get_ipv4_src(ipv4);
	clas_state->matches->ipv4_dst = get_ipv4_dst(ipv4);
}

void parse_icmpv4(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_icmpv4_hdr_t))) { return; }

	//Set reference
	cpc_icmpv4_hdr_t *icmpv4 = (cpc_icmpv4_hdr_t*)data; 

	//Set frame
	unsigned int num_of_icmpv4 = clas_state->num_of_headers[HEADER_TYPE_ICMPV4];
	clas_state->headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].frame = icmpv4; 
	clas_state->headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].present = true;
	clas_state->headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_ICMPV4] = num_of_icmpv4+1;

	//Set reference

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_icmpv4_hdr_t);
	datalen -= sizeof(cpc_icmpv4_hdr_t);


	if (datalen > 0){
		//TODO: something?	
	}

	//Initialize ipv4 packet matches
	clas_state->matches->icmpv4_code = get_icmpv4_code(icmpv4);
	clas_state->matches->icmpv4_type = get_icmpv4_type(icmpv4);
}

void parse_ipv6(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	
	if(unlikely(datalen < sizeof(cpc_ipv6_hdr_t))) { return; }
	
	//Set reference
	cpc_ipv6_hdr_t *ipv6 = (cpc_ipv6_hdr_t*)data; 

	//Set frame
	unsigned int num_of_ipv6 = clas_state->num_of_headers[HEADER_TYPE_IPV6];
	clas_state->headers[FIRST_IPV6_FRAME_POS + num_of_ipv6].frame = ipv6;
	clas_state->headers[FIRST_IPV6_FRAME_POS + num_of_ipv6].present = true;
	clas_state->headers[FIRST_IPV6_FRAME_POS + num_of_ipv6].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_IPV6] = num_of_ipv6+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_ipv6_hdr_t);
	datalen -= sizeof(cpc_ipv6_hdr_t);

	// FIXME: IP header with options

	switch (get_ipv6_next_header(ipv6)) {
		case IPV4_IP_PROTO:
			{
				parse_ipv4(clas_state, data, datalen);
			}
			break;
		case ICMPV4_IP_PROTO:
			{
				parse_icmpv4(clas_state, data, datalen);
			}
			break;
		case IPV6_IP_PROTO:
			{
				parse_ipv6(clas_state, data, datalen);
			}
			break;
		case ICMPV6_IP_PROTO:
			{
				parse_icmpv6(clas_state, data, datalen);
			}
			break;
		case UDP_IP_PROTO:
			{
				parse_udp(clas_state, data, datalen);
			}
			break;
		case TCP_IP_PROTO:
			{
				parse_tcp(clas_state, data, datalen);
			}
			break;
#if 0
		case SCTP_IP_PROTO:
			{
				parse_sctp(data, datalen);
			}
			break;
#endif
		default:
			{
			
			}
			break;
	}

	//Initialize ipv4 packet matches
	clas_state->matches->ip_proto = get_ipv6_next_header(ipv6);
	clas_state->matches->ip_dscp = get_ipv6_dscp(ipv6);
	clas_state->matches->ip_ecn = get_ipv6_ecn(ipv6);
	clas_state->matches->ipv6_src = get_ipv6_src(ipv6);
	clas_state->matches->ipv6_dst = get_ipv6_dst(ipv6);
	clas_state->matches->ipv6_flabel = get_ipv6_flow_label(ipv6);
}

void parse_icmpv6(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_icmpv6_hdr_t))) { return; }

	cpc_icmpv6_hdr_t* icmpv6 = (cpc_icmpv6_hdr_t*)data;
	
	//Set frame
	unsigned int num_of_icmpv6 = clas_state->num_of_headers[HEADER_TYPE_ICMPV6];
	clas_state->headers[FIRST_ICMPV6_FRAME_POS + num_of_icmpv6].frame = icmpv6;
	clas_state->headers[FIRST_ICMPV6_FRAME_POS + num_of_icmpv6].present = true;
	clas_state->headers[FIRST_ICMPV6_FRAME_POS + num_of_icmpv6].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_ICMPV6] = num_of_icmpv6+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_icmpv6_hdr_t);
	datalen -= sizeof(cpc_icmpv6_hdr_t);

	if (datalen > 0){
		//TODO: something?	
	}

	//Initialize icmpv6 packet matches
	clas_state->matches->icmpv6_code = get_icmpv6_code(icmpv6);
	clas_state->matches->icmpv6_type = get_icmpv6_type(icmpv6);
	clas_state->matches->ipv6_nd_target = get_icmpv6_neighbor_taddr(icmpv6);
	clas_state->matches->ipv6_nd_sll = get_icmpv6_ll_saddr(icmpv6);
	clas_state->matches->ipv6_nd_tll = get_icmpv6_ll_taddr(icmpv6);
	//clas_state->matches->ipv6_exthdr = ; 
}

void parse_tcp(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	if (unlikely(datalen < sizeof(cpc_tcp_hdr_t))) { return; }

	cpc_tcp_hdr_t* tcp = (cpc_tcp_hdr_t*)data;
	
	//Set frame
	unsigned int num_of_tcp = clas_state->num_of_headers[HEADER_TYPE_TCP];
	clas_state->headers[FIRST_TCP_FRAME_POS + num_of_tcp].frame = tcp;
	clas_state->headers[FIRST_TCP_FRAME_POS + num_of_tcp].present = true;
	clas_state->headers[FIRST_TCP_FRAME_POS + num_of_tcp].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_TCP] = num_of_tcp+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_tcp_hdr_t);
	datalen -= sizeof(cpc_tcp_hdr_t);

	if (datalen > 0){
		//TODO: something 
	}
	
	//Initialize tcp packet matches
	clas_state->matches->tcp_src = get_tcp_sport(tcp);
	clas_state->matches->tcp_dst = get_tcp_dport(tcp);
}

void parse_udp(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_udp_hdr_t))) { return; }

	cpc_udp_hdr_t *udp = (cpc_udp_hdr_t*)data; 
	
	//Set frame
	unsigned int num_of_udp = clas_state->num_of_headers[HEADER_TYPE_UDP];
	clas_state->headers[FIRST_UDP_FRAME_POS + num_of_udp].frame = udp;
	clas_state->headers[FIRST_UDP_FRAME_POS + num_of_udp].present = true;
	clas_state->headers[FIRST_UDP_FRAME_POS + num_of_udp].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_UDP] = num_of_udp+1;

	//Set reference
	
	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_udp_hdr_t);
	datalen -= sizeof(cpc_udp_hdr_t);

	if (datalen > 0){
		switch (get_udp_dport(udp)) {
		case GTPU_UDP_PORT: {
			parse_gtp(clas_state, data, datalen);
		} break;
		default: {
			//TODO: something
		} break;
		}
	}

	//Initialize udp packet matches
	clas_state->matches->udp_src = get_udp_sport(udp);
	clas_state->matches->udp_dst = get_udp_dport(udp);
}

void parse_gtp(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_gtphu_t))) { return; }

	cpc_gtphu_t *gtp = (cpc_gtphu_t*)data; 
		
	//Set frame
	unsigned int num_of_gtp = clas_state->num_of_headers[HEADER_TYPE_GTP];
	clas_state->headers[FIRST_GTP_FRAME_POS + num_of_gtp].frame = gtp;
	clas_state->headers[FIRST_GTP_FRAME_POS + num_of_gtp].present = true;
	clas_state->headers[FIRST_GTP_FRAME_POS + num_of_gtp].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_GTP] = num_of_gtp+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_gtphu_t);
	datalen -= sizeof(cpc_gtphu_t);

	if (datalen > 0){
		//TODO: something
	}

	//Initialize udp packet matches
	clas_state->matches->gtp_msg_type = get_gtpu_msg_type(gtp);
	clas_state->matches->gtp_teid = get_gtpu_teid(gtp);
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

	uint16_t ether_type = get_vlan_type(vlan);
	
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

	switch (get_ether_type(ether_header)) {
		case PPPOE_ETHER_DISCOVERY:
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

		case PPPOE_ETHER_SESSION:
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
	uint16_t inner_ether_type = get_ether_type(ether_header);

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
	clas_state->headers[FIRST_VLAN_FRAME_POS].length =  get_buffer_length(pkt) + sizeof(cpc_vlan_hdr_t) - sizeof(cpc_eth_hdr_t);
	//ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fvlanframe::vlan_hdr_t));
	//headers[FIRST_VLAN_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fvlanframe::vlan_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));

	/*
	 * set default values in vlan tag
	 */
	cpc_vlan_hdr_t* vlan_header = get_vlan_hdr(clas_state, 0);
	if ( get_vlan_hdr(clas_state, 1) ) {
		set_vlan_id(vlan_header, get_vlan_id(get_vlan_hdr(clas_state, 1)));
		set_vlan_pcp(vlan_header, get_vlan_pcp(get_vlan_hdr(clas_state, 1)));
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
	cpc_mpls_hdr_t* mpls_header;
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
	clas_state->headers[FIRST_MPLS_FRAME_POS].length = get_buffer_length(pkt) + sizeof(cpc_mpls_hdr_t) - sizeof(cpc_eth_hdr_t);
	//ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fmplsframe::mpls_hdr_t));
	//headers[FIRST_MPLS_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fmplsframe::mpls_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));

	set_ether_type(ether_header, ether_type);
	/*
	 * set default values in mpls tag
	 */
	mpls_header = get_mpls_hdr(clas_state, 0);

	if (get_mpls_hdr(clas_state, 1)){
		set_mpls_bos(mpls_header, false);
		set_mpls_label(mpls_header, get_mpls_label(get_mpls_hdr(clas_state, 1)));
		set_mpls_tc(mpls_header, get_mpls_tc(get_mpls_hdr(clas_state, 1)));
		set_mpls_ttl(mpls_header, get_mpls_ttl(get_mpls_hdr(clas_state, 1)));
	} else {
		set_mpls_bos(mpls_header, true);
		set_mpls_label(mpls_header, 0x0000);
		set_mpls_tc(mpls_header, 0x00);
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
		case PPPOE_ETHER_SESSION:
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
			ether_header-=sizeof(cpc_mpls_hdr_t); //We change also the local pointer
			set_ether_type(ether_header, PPPOE_ETHER_SESSION);

			/*
			 * append the new fpppoeframe instance to ether(0)
			 */
			push_header(clas_state, HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);
			push_header(clas_state, HEADER_TYPE_PPP, FIRST_PPP_FRAME_POS, FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES);
	
			n_pppoe = (cpc_pppoe_hdr_t*)clas_state->headers[FIRST_PPPOE_FRAME_POS].frame;
			n_ppp = (cpc_ppp_hdr_t*)clas_state->headers[FIRST_PPP_FRAME_POS].frame;

			//Now reset frames
			clas_state->headers[FIRST_PPPOE_FRAME_POS].frame = ether_header + sizeof(cpc_eth_hdr_t);
			clas_state->headers[FIRST_PPPOE_FRAME_POS].length = get_buffer_length(pkt) + sizeof(cpc_pppoe_hdr_t) - sizeof(cpc_eth_hdr_t);
			clas_state->headers[FIRST_PPP_FRAME_POS].frame = ether_header + sizeof(cpc_eth_hdr_t) + sizeof(cpc_pppoe_hdr_t);
			clas_state->headers[FIRST_PPP_FRAME_POS].length = get_buffer_length(pkt) + sizeof(cpc_ppp_hdr_t) - sizeof(cpc_eth_hdr_t) - sizeof(cpc_pppoe_hdr_t);
			//ether_header->reset(ether_header->soframe(), current_length + bytes_to_insert);
			//n_pppoe->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), ether_header->framelen() - sizeof(struct rofl::fetherframe::eth_hdr_t) );
			//n_ppp->reset(n_pppoe->soframe() + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t), n_pppoe->framelen() - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
	
			/*
			 * TODO: check if this is an appropiate fix 
			 */
			//TODO? n_pppoe->set_hdr_length(pkt->get_buffer_length() - sizeof(struct rofl::fetherframe::eth_hdr_t) - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
			set_ppp_prot(n_ppp, 0x0000);
		}
			break;

		case PPPOE_ETHER_DISCOVERY:
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
			ether_header-=sizeof(cpc_mpls_hdr_t); //We change also the local pointer
			set_ether_type(get_ether_hdr(clas_state, 0), PPPOE_ETHER_DISCOVERY);

			/*
			 * append the new fpppoeframe instance to ether(0)
			 */
			push_header(clas_state, HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);
			//Now reset frame
			clas_state->headers[FIRST_PPPOE_FRAME_POS].frame = ether_header + sizeof(cpc_eth_hdr_t);
			//Size of ethernet needs to be extended with +PPPOE size 
			//PPPOE size needs to be ether_header->size + PPPOE - ether_header
			clas_state->headers[FIRST_PPPOE_FRAME_POS].length = get_buffer_length(pkt) + sizeof(cpc_pppoe_hdr_t) - sizeof(cpc_eth_hdr_t);
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
	//TODO ROFL_DEBUG("datapacketx86(%p) soframe: %p framelen: %zu\n", this, pkt->get_buffer(), pkt->get_buffer_length());
	ROFL_DEBUG("Dump packet state(%p) TODO!!\n",clas_state);
}

size_t get_pkt_len(datapacket_t* pkt, classify_state_t* clas_state, void *from, void *to){

	unsigned int total_length = get_buffer_length(pkt);
	void* eth = get_ether_hdr(clas_state, 0);

	if(!from)
		return total_length;

	if(!to)
		return (size_t)(total_length - (from - eth));

	return (size_t)(to - from);
}

