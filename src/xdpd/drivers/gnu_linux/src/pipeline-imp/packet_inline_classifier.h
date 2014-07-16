#ifndef PACKET_IMPL_INLINE_CLASSIFIER__
#define PACKET_IMPL_INLINE_CLASSIFIER__

/**
* This file contains TODO: 
*/


//
// Check that the getter is defined
//


//Make it easy for users
#ifndef STATIC_PACKET_INLINE__
	#define STATIC_PACKET_INLINE__
#endif



//Check for the existance of GET_CLAS_STATE_PTR() MACRO
#ifndef GET_CLAS_STATE_PTR
	#error You cannot use packet_inline_classifier.h without defining GET_CLAS_STATE_PTR() MACRO
#else //Avoid infinite errors 


//Checksums calculator
static inline
void calculate_checksums_in_software(datapacket_t* pkt){

	//IP Checksum recalculation
	if( GET_CLAS_STATE_PTR(pkt) ->ipv4_recalc_checksum){
		if(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))	
			ipv4_calc_checksum(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
	}

	//Outer most IPv4 frame
	void *fipv4 = get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0);

	if (NULL != fipv4) {
		if (( GET_CLAS_STATE_PTR(pkt) ->tcp_recalc_checksum) && get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0) && fipv4) {

			tcpv4_calc_checksum(
					get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0),
					*get_ipv4_src(fipv4),
					*get_ipv4_dst(fipv4),
					*get_ipv4_proto(fipv4),
					get_pkt_len(pkt, GET_CLAS_STATE_PTR(pkt) , get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) ,0), NULL) ); // start at innermost IPv4 up to and including last frame

		} else if (( GET_CLAS_STATE_PTR(pkt) ->udp_recalc_checksum) && (get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) && fipv4) {

			udpv4_calc_checksum(
					get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0),
					*get_ipv4_src(fipv4),
					*get_ipv4_dst(fipv4),
					*get_ipv4_proto(fipv4),
					get_pkt_len(pkt, GET_CLAS_STATE_PTR(pkt) , get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0), NULL) ); // start at innermost IPv4 up to and including last frame

		} else if (( GET_CLAS_STATE_PTR(pkt) ->icmpv4_recalc_checksum) && (get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) ,0))) {

			icmpv4_calc_checksum(
				get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0),
				get_pkt_len(pkt,  GET_CLAS_STATE_PTR(pkt) , get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), NULL) );
		}
	}

	//Outer most IPv6 frame
	void *fipv6 = get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0);

	if (NULL != fipv6) {
		if (( GET_CLAS_STATE_PTR(pkt) ->tcp_recalc_checksum) && get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0) && fipv6) {

			tcpv6_calc_checksum(
					get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0),
					*get_ipv6_src(fipv6),
					*get_ipv6_dst(fipv6),
					TCP_IP_PROTO,
					get_pkt_len(pkt,  GET_CLAS_STATE_PTR(pkt) , get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) ,0), NULL) ); // start at innermost IPv6 up to and including last frame

		} else if (( GET_CLAS_STATE_PTR(pkt) ->udp_recalc_checksum) && (get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) && fipv6) {

			udpv6_calc_checksum(
					get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0),
					*get_ipv6_src(fipv6),
					*get_ipv6_dst(fipv6),
					UDP_IP_PROTO,
					get_pkt_len(pkt,  GET_CLAS_STATE_PTR(pkt) , get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0), NULL) ); // start at innermost IPv6 up to and including last frame

		} else if (( GET_CLAS_STATE_PTR(pkt) ->icmpv6_recalc_checksum) && (get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) ,0))) {

			icmpv6_calc_checksum(
					get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0),
					*get_ipv6_src(fipv6),
					*get_ipv6_dst(fipv6),
					ICMPV6_IP_PROTO,
					get_pkt_len(pkt,  GET_CLAS_STATE_PTR(pkt) , get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), NULL) );
		}
	}

	if (( GET_CLAS_STATE_PTR(pkt) ->sctp_recalc_checksum) && get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) {

			sctp_calc_checksum(
					get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0),
					get_pkt_len(pkt, GET_CLAS_STATE_PTR(pkt) , get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) ,0), NULL) );
	}
}

//Getters
STATIC_PACKET_INLINE__
uint32_t platform_packet_get_size_bytes(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	return  GET_CLAS_STATE_PTR(pkt) ->len;
#else
	return 0;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_port_in(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	return & GET_CLAS_STATE_PTR(pkt) ->port_in;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_phy_port_in(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	return & GET_CLAS_STATE_PTR(pkt) ->phy_port_in;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_eth_dst(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ether_hdr( GET_CLAS_STATE_PTR(pkt) ,0))) return NULL;
	return get_ether_dl_dst(get_ether_hdr( GET_CLAS_STATE_PTR(pkt) ,0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_eth_src(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ether_hdr( GET_CLAS_STATE_PTR(pkt) ,0))) return NULL;
	return get_ether_dl_src(get_ether_hdr( GET_CLAS_STATE_PTR(pkt) ,0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}


STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_eth_type(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	return classifier_get_eth_type( GET_CLAS_STATE_PTR(pkt) ); 
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
bool platform_packet_has_vlan(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	return (NULL != get_vlan_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return false;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}
STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_vlan_vid(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_vlan_hdr( GET_CLAS_STATE_PTR(pkt) ,0))) return NULL;
	return get_vlan_id(get_vlan_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_vlan_pcp(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_vlan_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_vlan_pcp(get_vlan_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_arp_opcode(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_arpv4_opcode(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_arp_sha(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_arpv4_dl_src(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_arp_spa(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_arpv4_ip_src(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) ,0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_arp_tha(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_arpv4_dl_dst(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) ,0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_arp_tpa(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_arpv4_ip_dst(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t platform_packet_get_ip_ecn(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if (NULL != get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))
		return get_ipv4_ecn(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
	if (NULL != get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))
		return get_ipv6_ecn(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
	return 0;
#else
	return 0;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t platform_packet_get_ip_dscp(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if (NULL != get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))
		return get_ipv4_dscp(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
	if (NULL != get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))
		return get_ipv6_dscp(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
	return 0;
#else
	return 0;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_ip_proto(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if (NULL != get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))
		return get_ipv4_proto(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
	if (NULL != get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))
		return get_ipv6_next_header(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_ipv4_src(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;

	return get_ipv4_src(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_ipv4_dst(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;

	return get_ipv4_dst(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_tcp_dst(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_tcp_dport(get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_tcp_src(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_tcp_sport(get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_udp_dst(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_udp_dport(get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_udp_src(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_udp_sport(get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_sctp_dst(datapacket_t *const pkt){

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_sctp_dport(get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_sctp_src(datapacket_t *const pkt){

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_sctp_sport(get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_icmpv4_type(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_icmpv4_type(get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_icmpv4_code(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_icmpv4_code(get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint128__t* platform_packet_get_ipv6_src(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if((NULL == get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0)))
		return NULL;

	return get_ipv6_src(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint128__t* platform_packet_get_ipv6_dst(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if((NULL == get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))){
		return NULL;
	}
	return get_ipv6_dst(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_ipv6_flabel(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if((NULL == get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_ipv6_flow_label(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint128__t* platform_packet_get_ipv6_nd_target(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	void *icmpv6_hdr;
	if ( (NULL == (icmpv6_hdr=get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) )
		return NULL;
	
	return get_icmpv6_neighbor_taddr(icmpv6_hdr);
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_ipv6_nd_sll(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	void* lla_opt_hdr;
	if( (NULL == (lla_opt_hdr = get_icmpv6_opt_lladr_source_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) )	return NULL;
	
	return get_icmpv6_ll_saddr(lla_opt_hdr);
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_ipv6_nd_tll(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	void* lla_opt_hdr;
	if( (NULL == (lla_opt_hdr = get_icmpv6_opt_lladr_target_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) )	return NULL;
	
	return get_icmpv6_ll_taddr(lla_opt_hdr);
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_ipv6_exthdr(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement	
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_icmpv6_type(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if((NULL == get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_icmpv6_type(get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_icmpv6_code(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if((NULL == get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_icmpv6_code(get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_mpls_label(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_mpls_label(get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_mpls_tc(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_mpls_tc(get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
bool platform_packet_get_mpls_bos(datapacket_t * const pkt){

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return false;
	return get_mpls_bos(get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return false; 
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_pbb_isid(datapacket_t *const pkt){

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_pbb_isid_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_pbb_isid(get_pbb_isid_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

//Tunnel id
STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_tunnel_id(datapacket_t *const pkt){

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: add implementation when supported
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}


STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_pppoe_code(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_pppoe_code(get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_pppoe_type(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_pppoe_type(get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_pppoe_sid(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_pppoe_sessid(get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_ppp_proto(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ppp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_ppp_prot(get_ppp_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_gtp_msg_type(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_gtpu_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_gtpu_msg_type(get_gtpu_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint32_t* platform_packet_get_gtp_teid(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	TM_STAMP_STAGE(pkt, TM_S4);

	if ((NULL == get_gtpu_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return NULL;
	return get_gtpu_teid(get_gtpu_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_capwap_wbid(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_capwap_rid(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_capwap_flags(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint16_t* platform_packet_get_wlan_fc(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_wlan_type(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_wlan_subtype(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint8_t* platform_packet_get_wlan_direction(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_wlan_address_1(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_wlan_address_2(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
uint64_t* platform_packet_get_wlan_address_3(datapacket_t * const pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO implement
	return NULL;
#else
	return NULL;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}


//Actions
STATIC_PACKET_INLINE__
void platform_packet_copy_ttl_in(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	void* mpls_hdr = get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0);
	void* mpls_hdr_inner = get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 1);
	if(mpls_hdr){
		if(mpls_hdr_inner){


			set_mpls_ttl(mpls_hdr_inner, *get_mpls_ttl(mpls_hdr));

		}

		/**
		* So this is theoretically what the spec says, but there is NO way for the classification to go beyond MPLs without implicitly assume IPv4/IPv6 frames... 
		*/

		#if 0
		else{

			void* ipv4_hdr = get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0);
			void* ipv6_hdr = get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0);
		
			if(ipv4_hdr)
				set_ipv4_ttl(ipv4_hdr, *get_mpls_ttl(mpls_hdr));
			if(ipv6_hdr)
				set_ipv6_hop_limit(ipv6_hdr, *get_mpls_ttl(mpls_hdr));

		}
		#endif
	}	
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_pop_vlan(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	pop_vlan(pkt,  GET_CLAS_STATE_PTR(pkt) );
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_pop_mpls(datapacket_t* pkt, uint16_t ether_type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	pop_mpls(pkt,  GET_CLAS_STATE_PTR(pkt) , ether_type);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_pop_pppoe(datapacket_t* pkt, uint16_t ether_type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	pop_pppoe(pkt,  GET_CLAS_STATE_PTR(pkt) , ether_type);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_push_pppoe(datapacket_t* pkt, uint16_t ether_type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	push_pppoe(pkt,  GET_CLAS_STATE_PTR(pkt) , ether_type);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_push_mpls(datapacket_t* pkt, uint16_t ether_type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	push_mpls(pkt,  GET_CLAS_STATE_PTR(pkt) , ether_type);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_push_vlan(datapacket_t* pkt, uint16_t ether_type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	push_vlan(pkt,  GET_CLAS_STATE_PTR(pkt) , ether_type);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_copy_ttl_out(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	void* mpls_hdr = get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0);
	void* mpls_hdr_inner = get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 1);

	//This code code be autogenerated based on the 

	if(mpls_hdr){
		if(mpls_hdr_inner){
			set_mpls_ttl(mpls_hdr, *get_mpls_ttl(mpls_hdr_inner));
		}
		/**
		* So this is theoretically what the spec says, but there is NO way for the classification to go beyond MPLs without implicitly assume IPv4/IPv6 frames... 
		*/
		
		#if 0
		else{
			void* ipv4_hdr = get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0);
			void* ipv6_hdr = get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0);
		
			if(ipv4_hdr)
				set_mpls_ttl(mpls_hdr, *get_ipv4_ttl(ipv4_hdr));
			if(ipv6_hdr)
				set_mpls_ttl(mpls_hdr, *get_ipv6_hop_limit(ipv6_hdr));

		}
		#endif	
	}	
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_dec_nw_ttl(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if(NULL != get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0)){
		dec_ipv4_ttl(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
		GET_CLAS_STATE_PTR(pkt) ->ipv4_recalc_checksum = true;

	}
	if(NULL != get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0)){
		dec_ipv6_hop_limit(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
	}
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_dec_mpls_ttl(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	dec_mpls_ttl(get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0));
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_mpls_ttl(datapacket_t* pkt, uint8_t new_ttl)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_mpls_ttl(get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0),new_ttl);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_nw_ttl(datapacket_t* pkt, uint8_t new_ttl)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if (NULL != get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0)){
		set_ipv4_ttl(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), new_ttl);
		GET_CLAS_STATE_PTR(pkt) ->ipv4_recalc_checksum = true;
	}
	if (NULL != get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0)){
		set_ipv6_hop_limit(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), new_ttl);
	}
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_eth_dst(datapacket_t* pkt, uint64_t eth_dst)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ether_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_ether_dl_dst(get_ether_hdr( GET_CLAS_STATE_PTR(pkt) , 0), eth_dst);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_eth_src(datapacket_t* pkt, uint64_t eth_src)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ether_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_ether_dl_src(get_ether_hdr( GET_CLAS_STATE_PTR(pkt) , 0), eth_src);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_eth_type(datapacket_t* pkt, uint16_t eth_type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ether_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_ether_type(get_ether_hdr( GET_CLAS_STATE_PTR(pkt) , 0), eth_type);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_vlan_vid(datapacket_t* pkt, uint16_t vlan_vid)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_vlan_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_vlan_id(get_vlan_hdr( GET_CLAS_STATE_PTR(pkt) , 0), vlan_vid);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_vlan_pcp(datapacket_t* pkt, uint8_t vlan_pcp)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_vlan_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_vlan_pcp(get_vlan_hdr( GET_CLAS_STATE_PTR(pkt) , 0), vlan_pcp);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_arp_opcode(datapacket_t* pkt, uint16_t arp_opcode)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_arpv4_opcode(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), arp_opcode);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_arp_sha(datapacket_t* pkt, uint64_t arp_sha)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_arpv4_dl_src(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), arp_sha);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_arp_spa(datapacket_t* pkt, uint32_t arp_spa)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_arpv4_ip_src(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), arp_spa);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_arp_tha(datapacket_t* pkt, uint64_t arp_tha)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_arpv4_dl_dst(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), arp_tha);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_arp_tpa(datapacket_t* pkt, uint32_t arp_tpa)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_arpv4_ip_dst(get_arpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), arp_tpa);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ip_dscp(datapacket_t* pkt, uint8_t ip_dscp)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if (NULL != get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) {
		set_ipv4_dscp(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ip_dscp);
		GET_CLAS_STATE_PTR(pkt) ->ipv4_recalc_checksum = true;
	}
	if (NULL != get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) {
		set_ipv6_dscp(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ip_dscp);

	}
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ip_ecn(datapacket_t* pkt, uint8_t ip_ecn)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if (NULL != get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0)){
		set_ipv4_ecn(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ip_ecn);
		GET_CLAS_STATE_PTR(pkt) ->ipv4_recalc_checksum = true;
	}
	if (NULL != get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0)){
		set_ipv6_ecn(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ip_ecn);
	}
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ip_proto(datapacket_t* pkt, uint8_t ip_proto)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if (NULL != get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) {
		set_ipv4_proto(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ip_proto);
		GET_CLAS_STATE_PTR(pkt) ->ipv4_recalc_checksum = true;
	}
	if (NULL != get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) {
		set_ipv6_next_header(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ip_proto);
	}
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ipv4_src(datapacket_t* pkt, uint32_t ip_src)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_ipv4_src(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ip_src);
	GET_CLAS_STATE_PTR(pkt) ->ipv4_recalc_checksum = true;
	GET_CLAS_STATE_PTR(pkt) ->tcp_recalc_checksum = true;
	GET_CLAS_STATE_PTR(pkt) ->udp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ipv4_dst(datapacket_t* pkt, uint32_t ip_dst)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_ipv4_dst(get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ip_dst);
	GET_CLAS_STATE_PTR(pkt) ->ipv4_recalc_checksum = true;
	GET_CLAS_STATE_PTR(pkt) ->tcp_recalc_checksum = true;
	GET_CLAS_STATE_PTR(pkt) ->udp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ipv6_src(datapacket_t* pkt, uint128__t ipv6_src)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_ipv6_src(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ipv6_src);
	GET_CLAS_STATE_PTR(pkt) ->tcp_recalc_checksum = true;
	GET_CLAS_STATE_PTR(pkt) ->udp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ipv6_dst(datapacket_t* pkt, uint128__t ipv6_dst)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_ipv6_dst(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ipv6_dst);
	GET_CLAS_STATE_PTR(pkt) ->tcp_recalc_checksum = true;
	GET_CLAS_STATE_PTR(pkt) ->udp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ipv6_flabel(datapacket_t* pkt, uint64_t ipv6_flabel)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_ipv6_flow_label(get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ipv6_flabel);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ipv6_nd_target(datapacket_t* pkt, uint128__t ipv6_nd_target)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_icmpv6_neighbor_taddr(get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), ipv6_nd_target);
	GET_CLAS_STATE_PTR(pkt) ->icmpv6_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ipv6_nd_sll(datapacket_t* pkt, uint64_t ipv6_nd_sll)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	void *lla_opt_hdr;
	if ( NULL == (lla_opt_hdr = get_icmpv6_opt_lladr_source_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) ) return;
	set_icmpv6_ll_saddr(lla_opt_hdr, ipv6_nd_sll);
	GET_CLAS_STATE_PTR(pkt) ->icmpv6_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ipv6_nd_tll(datapacket_t* pkt, uint64_t ipv6_nd_tll)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	void *lla_opt_hdr;
	if ( (NULL == (lla_opt_hdr = get_icmpv6_opt_lladr_target_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) ) return;
	set_icmpv6_ll_taddr(lla_opt_hdr,ipv6_nd_tll);
	GET_CLAS_STATE_PTR(pkt) ->icmpv6_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ipv6_exthdr(datapacket_t* pkt, uint16_t ipv6_exthdr)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	/*TODO Extension headers not yet implemented*/
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_icmpv6_type(datapacket_t* pkt, uint8_t icmpv6_type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_icmpv6_type(get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), icmpv6_type);
	GET_CLAS_STATE_PTR(pkt) ->icmpv6_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_icmpv6_code(datapacket_t* pkt, uint8_t icmpv6_code)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_icmpv6_code(get_icmpv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0), icmpv6_code);
	GET_CLAS_STATE_PTR(pkt) ->icmpv6_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_tcp_src(datapacket_t* pkt, uint16_t tcp_src)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_tcp_sport(get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0), tcp_src);
	GET_CLAS_STATE_PTR(pkt) ->tcp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_tcp_dst(datapacket_t* pkt, uint16_t tcp_dst)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_tcp_dport(get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0), tcp_dst);
	GET_CLAS_STATE_PTR(pkt) ->tcp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_udp_src(datapacket_t* pkt, uint16_t udp_src)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_udp_sport(get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0), udp_src);
	GET_CLAS_STATE_PTR(pkt) ->udp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_udp_dst(datapacket_t* pkt, uint16_t udp_dst)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_udp_dport(get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0), udp_dst);
	GET_CLAS_STATE_PTR(pkt) ->udp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_sctp_src(datapacket_t* pkt, uint16_t sctp_src)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_sctp_sport(get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0), sctp_src);
	GET_CLAS_STATE_PTR(pkt) ->sctp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_sctp_dst(datapacket_t* pkt, uint16_t sctp_dst)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_sctp_dport(get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) , 0), sctp_dst);
	GET_CLAS_STATE_PTR(pkt) ->sctp_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_icmpv4_type(datapacket_t* pkt, uint8_t type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_icmpv4_type(get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), type);
	GET_CLAS_STATE_PTR(pkt) ->icmpv4_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_icmpv4_code(datapacket_t* pkt, uint8_t code)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_icmpv4_code(get_icmpv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0), code);
	GET_CLAS_STATE_PTR(pkt) ->icmpv4_recalc_checksum = true;
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}


STATIC_PACKET_INLINE__
void platform_packet_set_mpls_label(datapacket_t* pkt, uint32_t label)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_mpls_label(get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0), label);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_mpls_tc(datapacket_t* pkt, uint8_t tc)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_mpls_tc(get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0), tc);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}
STATIC_PACKET_INLINE__
void platform_packet_set_mpls_bos(datapacket_t* pkt, bool bos)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_mpls_bos(get_mpls_hdr( GET_CLAS_STATE_PTR(pkt) , 0), bos);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}
STATIC_PACKET_INLINE__
void platform_packet_set_pbb_isid(datapacket_t*pkt, uint32_t pbb_isid)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_pbb_isid_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_pbb_isid(get_pbb_isid_hdr( GET_CLAS_STATE_PTR(pkt) , 0), pbb_isid);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}
STATIC_PACKET_INLINE__
void platform_packet_set_tunnel_id(datapacket_t*pkt, uint64_t tunnel_id)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}
STATIC_PACKET_INLINE__
void platform_packet_pop_pbb(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	pop_pbb(pkt,  GET_CLAS_STATE_PTR(pkt) );
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}
STATIC_PACKET_INLINE__
void platform_packet_push_pbb(datapacket_t* pkt, uint16_t ether_type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	push_pbb(pkt,  GET_CLAS_STATE_PTR(pkt) , ether_type);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}
STATIC_PACKET_INLINE__
void platform_packet_set_pppoe_type(datapacket_t* pkt, uint8_t type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_pppoe_type(get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0), type);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_pppoe_code(datapacket_t* pkt, uint8_t code)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_pppoe_code(get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0), code);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_pppoe_sid(datapacket_t* pkt, uint16_t sid)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_pppoe_sessid(get_pppoe_hdr( GET_CLAS_STATE_PTR(pkt) , 0), sid);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_ppp_proto(datapacket_t* pkt, uint16_t proto)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_ppp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_ppp_prot(get_ppp_hdr( GET_CLAS_STATE_PTR(pkt) , 0), proto);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_gtp_msg_type(datapacket_t* pkt, uint8_t msg_type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_gtpu_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_gtpu_msg_type(get_gtpu_hdr( GET_CLAS_STATE_PTR(pkt) , 0), msg_type);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_gtp_teid(datapacket_t* pkt, uint32_t teid)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	if ((NULL == get_gtpu_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) return;
	set_gtpu_teid(get_gtpu_hdr( GET_CLAS_STATE_PTR(pkt) , 0), teid);
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_capwap_wbid(datapacket_t* pkt, uint8_t wbid)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_capwap_rid(datapacket_t* pkt, uint8_t rid)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_capwap_flags(datapacket_t* pkt, uint16_t flags)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_wlan_fc(datapacket_t* pkt, uint16_t fc)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_wlan_type(datapacket_t* pkt, uint8_t type)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_wlan_subtype(datapacket_t* pkt, uint8_t subtype)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_wlan_direction(datapacket_t* pkt, uint8_t direction)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_wlan_address_1(datapacket_t* pkt, uint64_t address_1)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_wlan_address_2(datapacket_t* pkt, uint64_t address_2)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_set_wlan_address_3(datapacket_t* pkt, uint64_t address_3)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}


STATIC_PACKET_INLINE__
void platform_packet_pop_gtp(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}
STATIC_PACKET_INLINE__
void platform_packet_push_gtp(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}


STATIC_PACKET_INLINE__
void platform_packet_pop_capwap(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}
STATIC_PACKET_INLINE__
void platform_packet_push_capwap(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}


STATIC_PACKET_INLINE__
void platform_packet_pop_wlan(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}

STATIC_PACKET_INLINE__
void platform_packet_push_wlan(datapacket_t* pkt)
{

#ifndef EMPTY_PACKET_PROCESSING_ROUTINES
	//TODO: implement
#else
	return;
#endif /* EMPTY_PACKET_PROCESSING_ROUTINES */

}


#endif //GET_CLAS_STATE_PTR() check
#endif //Guards
