#ifndef PACKET_OFFLOADS_DPDK__
#define PACKET_OFFLOADS_DPDK__

//Checksums calculator
static inline
void set_checksums_in_hw(datapacket_t* pkt){

/* 
   http://dpdk.org/doc/api/rte__mbuf_8h.html
   To use hardware L4 checksum offload, the user needs to:

   fill l2_len and l3_len in mbuf
   set the flags PKT_TX_TCP_CKSUM, PKT_TX_SCTP_CKSUM or PKT_TX_UDP_CKSUM
   set the flag PKT_TX_IPV4 or PKT_TX_IPV6
   calculate the pseudo header checksum and set it in the L4 header (only for TCP or UDP). See rte_ipv4_phdr_cksum() and rte_ipv6_phdr_cksum().
   For SCTP, set the crc field to 0.Disable L4 cksum of TX pkt.
*/

	struct rte_mbuf* mbuf_origin;
	mbuf_origin = ((datapacket_dpdk_t*)pkt->platform_state)->mbuf;

	uint16_t l4_checksum = 0;

	void *fipv4 = get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0);
        void *fipv6 = get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0);
	
	datapacket_dpdk_t* dpdk = (datapacket_dpdk_t*)pkt->platform_state);

	if(fipv4){

	        mbuf_origin->l2_len = fipv4 - get_buffer_dpdk(dpdk);
		mbuf_origin->ol_flags |= PKT_TX_IPV4;

		if ( is_recalculate_checksum_flag_set( GET_CLAS_STATE_PTR(pkt) , RECALCULATE_TCP_CHECKSUM_IN_SW ) && get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) {
                        mbuf_origin->ol.flags |= PKT_TX_TCP_CKSUM;
	                mbuf_origin->l3_len = get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0) - fipv4;

			l4_checksum = rte_ipv4_phdr_cksum((ipv4_hdr *)fipv4, mbuf_origin->ol_flags);
			cpc_tcp_hdr->checksum = l4_checksum;
               
		 } else if ( is_recalculate_checksum_flag_set( GET_CLAS_STATE_PTR(pkt) , RECALCULATE_UDP_CHECKSUM_IN_SW ) && (get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) {
			mbuf_origin->ol_flags |= PKT_TX_UDP_CKSUM;        
                        mbuf_origin->l3_len = get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0) - fipv4;

			l4_checksum = rte_ipv4_phdr_cksum((ipv4_hdr *)fipv4, mbuf_origin->ol_flags);
		}                
	}

	if(fipv6){

	        mbuf_origin->l2_len = fipv4 - get_buffer_dpdk(dpdk);
                mbuf_origin->ol_flags |= PKT_TX_IPV6;

		if ( is_recalculate_checksum_flag_set( GET_CLAS_STATE_PTR(pkt) , RECALCULATE_TCP_CHECKSUM_IN_SW ) && get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0)) {
                        mbuf_origin->ol.flags |= PKT_TX_TCP_CKSUM;
        	        mbuf_origin->l3_len = get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0) - fipv6;
		
			l4_checksum = rte_ipv6_phdr_cksum((ipv6_hdr *)fipv6, mbuf_origin->ol_flags);

       		} else if ( is_recalculate_checksum_flag_set( GET_CLAS_STATE_PTR(pkt) , RECALCULATE_UDP_CHECKSUM_IN_SW ) && (get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) {
                        mbuf_origin->ol_flags |= PKT_TX_UDP_CKSUM;               
                        mbuf_origin->l3_len = get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0) - fipv6;
       		
			l4_checksum = rte_ipv6_phdr_cksum((ipv6_hdr *)fipv6, mbuf_origin->ol_flags);
        	}     
	}

	if ( is_recalculate_checksum_flag_set( GET_CLAS_STATE_PTR(pkt) , RECALCULATE_SCTP_CHECKSUM_IN_SW ) && (get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) ,0))) {
        	//TODO
                mbuf_origin->ol_flags |= PKT_TX_SCTP_CKSUM

      	}



}

#endif //PACKET_OFFLOADS_DPDK__
