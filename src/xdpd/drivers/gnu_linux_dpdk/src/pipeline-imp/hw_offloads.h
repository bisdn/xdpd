#ifndef PACKET_OFFLOADS_DPDK__
#define PACKET_OFFLOADS_DPDK__

/**
* @file offloads_dpdk.h
* @author Fran González<fran@bisdn.de>
*
* @brief Function for setting the hardware to compute checksum offloads
*/

#include <rte_byteorder.h> // For rte_be_to_cpu_16

/*
*
*/
static inline uint16_t
get_16b_sum(uint16_t *ptr16, uint32_t nr)
{
	uint32_t sum = 0;
	while (nr > 1)
	{
		sum +=*ptr16;
		nr -= sizeof(uint16_t);
		ptr16++;
		if (sum > UINT16_MAX)
			sum -= UINT16_MAX;
	}

	/* If length is in odd bytes */
	if (nr)
		sum += *((uint8_t*)ptr16);

	sum = ((sum & 0xffff0000) >> 16) + (sum & 0xffff);
	sum &= 0x0ffff;
	return (uint16_t)sum;
}

/*
*
*/
static inline uint16_t
get_ipv4_psd_sum (struct cpc_ipv4_hdr * ip_hdr)
{
	/* Pseudo Header for IPv4/UDP/TCP checksum */
	union ipv4_psd_header {
		struct {
			uint32_t src_addr; /* IP address of source host. */
			uint32_t dst_addr; /* IP address of destination host(s). */
			uint8_t  zero;     /* zero. */
			uint8_t  proto;    /* L4 protocol type. */
			uint16_t len;      /* L4 length. */
		} __attribute__((__packed__));
		uint16_t u16_arr[0];
	} psd_hdr;

	psd_hdr.src_addr = ip_hdr->src;
	psd_hdr.dst_addr = ip_hdr->dst;
	psd_hdr.zero     = 0;
	psd_hdr.proto    = ip_hdr->proto;
	psd_hdr.len      = rte_cpu_to_be_16((uint16_t)(rte_be_to_cpu_16(ip_hdr->length)
				- sizeof(struct cpc_ipv4_hdr)));
	return get_16b_sum(psd_hdr.u16_arr, sizeof(psd_hdr));
}

/*
* Checksums calculator
*/
static inline
void set_checksums_in_hw(datapacket_t* pkt){

/*
   For DPDK v1.8
   http://dpdk.org/doc/api/rte__mbuf_8h.html
   To use hardware L4 checksum offload, the user needs to:

   fill l2_len and l3_len in mbuf
   set the flags PKT_TX_TCP_CKSUM, PKT_TX_SCTP_CKSUM or PKT_TX_UDP_CKSUM
   set the flag PKT_TX_IPV4 or PKT_TX_IPV6
   calculate the pseudo header checksum and set it in the L4 header (only for TCP or UDP). See rte_ipv4_phdr_cksum() and rte_ipv6_phdr_cksum().
   For SCTP, set the crc field to 0.Disable L4 cksum of TX pkt.
*/

	struct rte_mbuf* mbuf_;
	mbuf_ = ((datapacket_dpdk_t*)pkt->platform_state)->mbuf;

	uint8_t l2_len;
	uint8_t l3_len;

	struct cpc_ipv4_hdr  *ipv4_hdr;
//	struct cpc_ipv6_hdr  *ipv6_hdr;
	struct cpc_udp_hdr   *udp_hdr;
	struct cpc_tcp_hdr   *tcp_hdr;
	struct cpc_sctp_hdr  *sctp_hdr;

	void *fipv4 = get_ipv4_hdr( GET_CLAS_STATE_PTR(pkt) , 0);
        void *fipv6 = get_ipv6_hdr( GET_CLAS_STATE_PTR(pkt) , 0);

	datapacket_dpdk_t* dpdk = (datapacket_dpdk_t*)pkt->platform_state;

        //IP Checksum recalculation
	if( is_recalculate_checksum_flag_set( GET_CLAS_STATE_PTR(pkt) , RECALCULATE_IPV4_CHECKSUM_IN_SW) && fipv4 ){
        
		// hw flag
		mbuf_->ol_flags |= PKT_TX_IP_CKSUM;
		//sw flag
		//mbuf->ol_flags |= PKT_TX_IPV4;
	}

	if(fipv4){

		l2_len = (uint8_t*)fipv4 - get_buffer_dpdk(dpdk); 
		/* Do not support ipv4 option field */
		l3_len = sizeof(struct cpc_ipv4_hdr);
		ipv4_hdr = (struct cpc_ipv4_hdr *) (rte_pktmbuf_mtod(mbuf_, unsigned char *) + l2_len);

		/* Do not delete, this is required by HW*/
		ipv4_hdr->checksum = 0;

		/* UDP */
		if ( is_recalculate_checksum_flag_set( GET_CLAS_STATE_PTR(pkt) , RECALCULATE_UDP_CHECKSUM_IN_SW ) && (get_udp_hdr( GET_CLAS_STATE_PTR(pkt) , 0))) {
			
			udp_hdr = (struct cpc_udp_hdr*) (rte_pktmbuf_mtod(mbuf_, unsigned char *) + l2_len + l3_len);
			/* HW Offload */
                        mbuf_->ol_flags |= PKT_TX_UDP_CKSUM;
                        /* Pseudo header sum need be set properly */
                        udp_hdr->checksum = get_ipv4_psd_sum(ipv4_hdr);
		}
		/* TCP */
		else if( is_recalculate_checksum_flag_set( GET_CLAS_STATE_PTR(pkt) , RECALCULATE_TCP_CHECKSUM_IN_SW ) && get_tcp_hdr( GET_CLAS_STATE_PTR(pkt) , 0)){

			tcp_hdr = (struct cpc_tcp_hdr*) (rte_pktmbuf_mtod(mbuf_, unsigned char *) + l2_len + l3_len);
			
			mbuf_->ol_flags |= PKT_TX_TCP_CKSUM;
			tcp_hdr->checksum = get_ipv4_psd_sum(ipv4_hdr);

		}
		/* SCTP */
		else if ( is_recalculate_checksum_flag_set( GET_CLAS_STATE_PTR(pkt) , RECALCULATE_SCTP_CHECKSUM_IN_SW ) && (get_sctp_hdr( GET_CLAS_STATE_PTR(pkt) ,0))) {
			sctp_hdr = (struct cpc_sctp_hdr*) (rte_pktmbuf_mtod(mbuf_,unsigned char *) + l2_len + l3_len);

			mbuf_->ol_flags |= PKT_TX_SCTP_CKSUM;
			sctp_hdr->checksum = 0;

			/* Sanity check, only number of 4 bytes supported */
			if ((rte_be_to_cpu_16(ipv4_hdr->length) % 4) != 0){
				printf("sctp payload must be a multiple "
					"of 4 bytes for checksum offload");
			}
			else {
				sctp_hdr->checksum = 0;
				/* CRC32c sample code available in RFC3309 */
			}
		}
		/* End of L4 Handling*/

	}

	if(fipv6){


      	}

}

#endif //PACKET_OFFLOADS_DPDK_
