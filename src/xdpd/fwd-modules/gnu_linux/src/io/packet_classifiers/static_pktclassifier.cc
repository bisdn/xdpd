#include "static_pktclassifier.h"
#include <rofl/common/utils/c_logger.h>
#include "../datapacketx86.h"


//Constructor&destructor
static_pktclassifier::static_pktclassifier(datapacketx86* pkt_ref) :
	packetclassifier(pkt_ref),
{
	unsigned int i;

	classify_reset();	
	
	/*
	* Initialize fframes
	*/
	
	//Ether
	for(i=0;i<MAX_ETHER_FRAMES;i++){
		headers[FIRST_ETHER_FRAME_POS+i].frame = rofl::fetherframe(NULL, 0);		
		headers[FIRST_ETHER_FRAME_POS+i].type = HEADER_TYPE_ETHER;
	}
	//vlan
	for(i=0;i<MAX_VLAN_FRAMES;i++){
		headers[FIRST_VLAN_FRAME_POS+i].frame = rofl::fvlanframe(NULL, 0);		
		headers[FIRST_VLAN_FRAME_POS+i].type = HEADER_TYPE_VLAN;
	}
	//mpls
	for(i=0;i<MAX_MPLS_FRAMES;i++){
		headers[FIRST_MPLS_FRAME_POS+i].frame = rofl::fmplsframe(NULL, 0);		
		headers[FIRST_MPLS_FRAME_POS+i].type = HEADER_TYPE_MPLS;
	}
	//arpv4
	for(i=0;i<MAX_ARPV4_FRAMES;i++){
		headers[FIRST_ARPV4_FRAME_POS+i].frame = rofl::farpv4frame(NULL, 0);		
		headers[FIRST_ARPV4_FRAME_POS+i].type = HEADER_TYPE_ARPV4;
	}
	//ipv4
	for(i=0;i<MAX_IPV4_FRAMES;i++){
		headers[FIRST_IPV4_FRAME_POS+i].frame = rofl::fipv4frame(NULL, 0);		
		headers[FIRST_IPV4_FRAME_POS+i].type = HEADER_TYPE_IPV4;
	}
	//icmpv4
	for(i=0;i<MAX_ICMPV4_FRAMES;i++){
		headers[FIRST_ICMPV4_FRAME_POS+i].frame = rofl::ficmpv4frame(NULL, 0);		
		headers[FIRST_ICMPV4_FRAME_POS+i].type = HEADER_TYPE_ICMPV4;
	}
	//udp
	for(i=0;i<MAX_UDP_FRAMES;i++){
		headers[FIRST_UDP_FRAME_POS+i].frame = rofl::fudpframe(NULL, 0);		
		headers[FIRST_UDP_FRAME_POS+i].type = HEADER_TYPE_UDP;
	}
	//tcp
	for(i=0;i<MAX_TCP_FRAMES;i++){
		headers[FIRST_TCP_FRAME_POS+i].frame = rofl::ftcpframe(NULL, 0);		
		headers[FIRST_TCP_FRAME_POS+i].type = HEADER_TYPE_TCP;
	}
	//sctp
	for(i=0;i<MAX_SCTP_FRAMES;i++){
		headers[FIRST_SCTP_FRAME_POS+i].frame = rofl::fsctpframe(NULL, 0);		
		headers[FIRST_SCTP_FRAME_POS+i].type = HEADER_TYPE_SCTP;
	}
	//pppoe
	for(i=0;i<MAX_PPPOE_FRAMES;i++){
		headers[FIRST_PPPOE_FRAME_POS+i].frame = rofl::fpppoeframe(NULL, 0);		
		headers[FIRST_PPPOE_FRAME_POS+i].type = HEADER_TYPE_PPPOE;
	}

	//ppp
	for(i=0;i<MAX_PPP_FRAMES;i++){
		headers[FIRST_PPP_FRAME_POS+i].frame = rofl::fpppframe(NULL, 0);		
		headers[FIRST_PPP_FRAME_POS+i].type = HEADER_TYPE_PPP;
	}

	//Add more here...
}

static_pktclassifier::~static_pktclassifier(){


}



void static_pktclassifier::classify(void){
	classify_reset();
	parse_ether(pkt->get_buffer(), pkt->get_buffer_length());
	is_classified = true;
}


/*
* Getters
*/

rofl::fetherframe* static_pktclassifier::ether(unsigned int idx) const{

	unsigned int pos;	

	if(idx > MAX_ETHER_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ETHER_FRAME_POS + num_of_headers[HEADER_TYPE_ETHER];
	else
		pos = FIRST_ETHER_FRAME_POS + idx;	

	//Return the index
	return (rofl::fetherframe*) headers[pos]->frame	
}

rofl::fvlanframe* static_pktclassifier::vlan(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_VLAN_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_VLAN_FRAME_POS + num_of_headers[HEADER_TYPE_VLAN];
	else
		pos = FIRST_VLAN_FRAME_POS + idx;	

	//Return the index
	return (rofl::fvlanframe*) headers[pos]->frame;
}

rofl::fmplsframe* static_pktclassifier::mpls(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_MPLS_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_MPLS_FRAME_POS + num_of_headers[HEADER_TYPE_MPLS];
	else
		pos = FIRST_MPLS_FRAME_POS + idx;	

	//Return the index
	return (rofl::fmplsframe*) headers[pos]->frame;
}

rofl::farpv4frame* static_pktclassifier::arpv4(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_ARPV4_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ARPV4_FRAME_POS + num_of_headers[HEADER_TYPE_ARPV4];
	else
		pos = FIRST_ARPV4_FRAME_POS + idx;	

	//Return the index
	return (rofl::farpv4frame*) headers[pos]->frame;
}

rofl::fipv4frame* static_pktclassifier::ipv4(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_IPV4_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_IPV4_FRAME_POS + num_of_headers[HEADER_TYPE_IPV4];
	else
		pos = FIRST_IPV4_FRAME_POS + idx;	

	//Return the index
	return (rofl::fipv4frame*) headers[pos]->frame;
}

rofl::ficmpv4frame* static_pktclassifier::icmpv4(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_ICMPV4_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ICMPV4_FRAME_POS + num_of_headers[HEADER_TYPE_ICMPV4];
	else
		pos = FIRST_ICMPV4_FRAME_POS + idx;	

	//Return the index
	return (rofl::ficmpv4frame*) headers[pos]->frame;

}

rofl::fudpframe* static_pktclassifier::udp(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_UDP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_UDP_FRAME_POS + num_of_headers[HEADER_TYPE_UDP];
	else
		pos = FIRST_UDP_FRAME_POS + idx;	

	//Return the index
	return (rofl::fudpframe*) headers[pos]->frame;

}

rofl::ftcpframe* static_pktclassifier::tcp(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_TCP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_TCP_FRAME_POS + num_of_headers[HEADER_TYPE_TCP];
	else
		pos = FIRST_TCP_FRAME_POS + idx;	

	//Return the index
	return (rofl::ftcpframe*) headers[pos]->frame;

}

rofl::fsctpframe* static_pktclassifier::sctp(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_SCTP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_SCTP_FRAME_POS + num_of_headers[HEADER_TYPE_SCTP];
	else
		pos = FIRST_SCTP_FRAME_POS + idx;	

	//Return the index
	return (rofl::fsctpframe*) headers[pos]->frame;
}

rofl::fpppoeframe* static_pktclassifier::pppoe(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_PPPOE_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_PPPOE_FRAME_POS + num_of_headers[HEADER_TYPE_PPPOE];
	else
		pos = FIRST_PPPOE_FRAME_POS + idx;	

	//Return the index
	return (rofl::fpppoeframe*) headers[pos]->frame;

}

rofl::fpppframe* static_pktclassifier::ppp(unsigned int idx) const
{
	unsigned int pos;	

	if(idx > MAX_PPP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_PPP_FRAME_POS + num_of_headers[HEADER_TYPE_PPP];
	else
		pos = FIRST_PPP_FRAME_POS + idx;	

	//Return the index
	return (rofl::fpppframe*) headers[pos]->frame;
}

/*
* Classify code...
*/
void static_pktclassifier::parse_ether(uint8_t *data, size_t datalen){

	uint8_t *p_ptr 	= data;
	size_t 	 p_len 	= datalen;

	if (p_len < sizeof(struct rofl::fetherframe::eth_hdr_t)){return;}

	//Set frame
	unsigned int num_of_ether = num_of_headers[HEADER_TYPE_ETHER];
	headers[FIRST_ETHER_FRAME_POS + num_of_ether].frame.reset(data, sizeof(struct rofl::fetherframe::eth_hdr_t));
	headers[FIRST_ETHER_FRAME_POS + num_of_ether].present = true;
	num_of_headers[HEADER_TYPE_ETHER] = num_of_ether+1;

	//Increment pointers and decrement remaining payload size
	p_ptr += sizeof(struct rofl::fetherframe::eth_hdr_t);
	p_len -= sizeof(struct rofl::fetherframe::eth_hdr_t);

	//Set pointer to header
	rofl::fetherframe* ether = &headers[FIRST_ETHER_FRAME_POS + num_of_ether].frame;
	
	eth_type = ether->get_dl_type();

	switch (eth_type) {
		case rofl::fvlanframe::VLAN_CTAG_ETHER:
		case rofl::fvlanframe::VLAN_STAG_ETHER:
		case rofl::fvlanframe::VLAN_ITAG_ETHER:
			{
				parse_vlan(p_ptr, p_len);
			}
			break;
		case rofl::fmplsframe::MPLS_ETHER:
		case rofl::fmplsframe::MPLS_ETHER_UPSTREAM:
			{
				parse_mpls(p_ptr, p_len);
			}
			break;
		case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
		case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
			{
				parse_pppoe(p_ptr, p_len);
			}
			break;
		case rofl::farpv4frame::ARPV4_ETHER:
			{
				parse_arpv4(p_ptr, p_len);
			}
			break;
		case rofl::fipv4frame::IPV4_ETHER:
			{
				parse_ipv4(p_ptr, p_len);
			}
			break;
		default:
			{
				
			}
			break;
	}
}

void static_pktclassifier::parse_vlan( uint8_t *data, size_t datalen){

	uint8_t *p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::fvlanframe::vlan_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_vlan = num_of_headers[HEADER_TYPE_VLAN];
	headers[FIRST_VLAN_FRAME_POS + num_of_vlan].frame.reset(data, sizeof(struct rofl::fvlanframe::vlan_hdr_t));
	headers[FIRST_VLAN_FRAME_POS + num_of_vlan].present = true;
	num_of_headers[HEADER_TYPE_VLAN] = num_of_vlan+1;

	//Increment pointers and decrement remaining payload size
	p_ptr += sizeof(struct rofl::fvlanframe::vlan_hdr_t);
	p_len -= sizeof(struct rofl::fvlanframe::vlan_hdr_t);

	//Set pointer to header
	rofl::fvlanframe* vlan = &headers[FIRST_VLAN_FRAME_POS + num_of_vlan].frame;
	
	eth_type = vlan->get_dl_type();

	switch (eth_type) {
		case rofl::fvlanframe::VLAN_CTAG_ETHER:
		case rofl::fvlanframe::VLAN_STAG_ETHER:
		case rofl::fvlanframe::VLAN_ITAG_ETHER:
			{
				parse_vlan(p_ptr, p_len);
			}
			break;
		case rofl::fmplsframe::MPLS_ETHER:
		case rofl::fmplsframe::MPLS_ETHER_UPSTREAM:
			{
				parse_mpls(p_ptr, p_len);
			}
			break;
		case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
		case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
			{
				parse_pppoe(p_ptr, p_len);
			}
			break;
		case rofl::farpv4frame::ARPV4_ETHER:
			{
				parse_arpv4(p_ptr, p_len);
			}
			break;
		case rofl::fipv4frame::IPV4_ETHER:
			{
				parse_ipv4(p_ptr, p_len);
			}
			break;
		default:
			{

			}
			break;
	}
}

void static_pktclassifier::parse_mpls(uint8_t *data, size_t datalen){

	
	uint8_t *p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::fmplsframe::mpls_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_mpls = num_of_headers[HEADER_TYPE_MPLS];
	headers[FIRST_MPLS_FRAME_POS + num_of_mpls].frame.reset(data, sizeof(struct rofl::fmplsframe::mpls_hdr_t));
	headers[FIRST_MPLS_FRAME_POS + num_of_mpls].present = true;
	num_of_headers[HEADER_TYPE_MPLS] = num_of_mpls+1;

	//Increment pointers and decrement remaining payload size
	p_ptr += sizeof(struct rofl::fmplsframe::mpls_hdr_t);
	p_len -= sizeof(struct rofl::fmplsframe::mpls_hdr_t);

	//Set pointer to header
	rofl::fmplsframe* mpls = &headers[FIRST_MPLS_FRAME_POS + num_of_mpls].frame;
	
	if (not mpls->get_mpls_bos()){

		parse_mpls(p_ptr, p_len);

	}else{
		
		//TODO: We could be trying to guess if payload is IPv4/v6 and continue parsing...
	}
}



void static_pktclassifier::parse_pppoe(uint8_t *data, size_t datalen){

	uint8_t *p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::fpppoeframe::pppoe_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_pppoe = num_of_headers[HEADER_TYPE_PPPOE];
	headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].frame.reset(data, sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
	headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].present = true;
	num_of_headers[HEADER_TYPE_PPPOE] = num_of_pppoe+1;
	
	rofl::fpppoeframe* pppoe = &headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].frame;

	switch (eth_type) {
		case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
			{
				p_len -= sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);

				uint16_t pppoe_len = pppoe->get_hdr_length() > p_len ? p_len : pppoe->get_hdr_length();

				/*
				 * parse any pppoe service tags
				 */
				pppoe->unpack(p_ptr, sizeof(struct rofl::fpppoeframe::pppoe_hdr_t) + pppoe_len);


				/*
				 * any remaining bytes after the pppoe tags => padding?
				 */
				if (p_len > pppoe->tags.length())
				{
					//TODO: Continue parsing??	
				}
			}
			break;
		case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
			{
				//Increment pointers and decrement remaining payload size
				p_ptr += sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);
				p_len -= sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);

				parse_ppp(p_ptr, p_len);
			}
			break;
		default:
			{
				// log error?
			}
			break;
	}


}



void static_pktclassifier::parse_ppp(uint8_t *data, size_t datalen){

	uint8_t *p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::fpppframe::ppp_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_ppp = num_of_headers[HEADER_TYPE_PPP];
	headers[FIRST_PPP_FRAME_POS + num_of_mpls].frame.reset(data, sizeof(struct rofl::fpppframe::ppp_hdr_t));
	headers[FIRST_PPP_FRAME_POS + num_of_ppp].present = true;
	num_of_headers[HEADER_TYPE_PPP] = num_of_ppp+1;

	//Set reference
	rofl::fpppframe* ppp = &headers[FIRST_PPP_FRAME_POS + num_of_ppp].frame; 

	//Increment pointers and decrement remaining payload size
	switch (ppp->get_ppp_prot()) {
		case rofl::fpppframe::PPP_PROT_IPV4:
			{
				//Increment pointers and decrement remaining payload size
				p_ptr += sizeof(struct rofl::fpppframe::ppp_hdr_t);
				p_len -= sizeof(struct rofl::fpppframe::ppp_hdr_t);

				parse_ipv4(p_ptr, p_len);
			}
			break;
		default:
			{
				ppp->unpack(p_ptr, p_len);
			}
			break;
	}
}



void static_pktclassifier::parse_arpv4(uint8_t *data, size_t datalen){

	uint8_t *p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::farpv4frame::arpv4_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_arpv4 = num_of_headers[HEADER_TYPE_ARPV4];
	headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].frame.reset(data, sizeof(struct rofl::farpv4frame::arpv4_hdr_t));
	headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].present = true;
	num_of_headers[HEADER_TYPE_ARPV4] = num_of_arpv4+1;

	//Set reference
	//rofl::farpv4frame *arpv4 = &headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].frame; 

	//Increment pointers and decrement remaining payload size
	p_ptr += sizeof(struct rofl::farpv4frame::arpv4_hdr_t);
	p_len -= sizeof(struct rofl::farpv4frame::arpv4_hdr_t);

	if (p_len > 0){
		//TODO: something?
	}
}



void static_pktclassifier::parse_ipv4(uint8_t *data, size_t datalen){

	uint8_t *p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::fipv4frame::ipv4_hdr_t)) { return; }
	
	//Set frame
	unsigned int num_of_ipv4 = num_of_headers[HEADER_TYPE_IPV4];
	headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].frame.reset(data, sizeof(struct rofl::fipv4frame::ipv4_hdr_t));
	headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].present = true;
	num_of_headers[HEADER_TYPE_IPV4] = num_of_ipv4+1;

	//Set reference
	rofl::fipv4frame *ipv4 = &headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].frame; 

	//Increment pointers and decrement remaining payload size
	p_ptr += sizeof(struct rofl::fipv4frame::ipv4_hdr_t);
	p_len -= sizeof(struct rofl::fipv4frame::ipv4_hdr_t);

	if (ipv4->has_MF_bit_set){
		// TODO: fragment handling

		return;
	}

	// FIXME: IP header with options


	switch (ipv4->get_ipv4_proto()) {
		case rofl::fipv4frame::IPV4_IP_PROTO:
			{
				parse_ipv4(p_ptr, p_len);
			}
			break;
		case rofl::ficmpv4frame::ICMPV4_IP_PROTO:
			{
				parse_icmpv4(p_ptr, p_len);
			}
			break;
		case rofl::fudpframe::UDP_IP_PROTO:
			{
				parse_udp(p_ptr, p_len);
			}
			break;
		case rofl::ftcpframe::TCP_IP_PROTO:
			{
				parse_tcp(p_ptr, p_len);
			}
			break;
		case rofl::fsctpframe::SCTP_IP_PROTO:
			{
				parse_sctp(p_ptr, p_len);
			}
			break;
		default:
			{
			
			}
			break;
	}
}



void static_pktclassifier::parse_icmpv4( uint8_t *data, size_t datalen){

	uint8_t	*p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_icmpv4 = num_of_headers[HEADER_TYPE_ICMPV4];
	headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].frame.reset(data, sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t));
	headers[FIRST_IPV4_FRAME_POS + num_of_icmpv4].present = true;
	num_of_headers[HEADER_TYPE_ICMPV4] = num_of_icmpv4+1;

	//Set reference
	rofl::ficmpv4frame *icmpv4 = &headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].frame; 

	//Increment pointers and decrement remaining payload size
	p_ptr += sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t);
	p_len -= sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t);


	if (p_len > 0){
		//TODO: something?	
	}
}



void static_pktclassifier::parse_udp(uint8_t *data, size_t datalen){

	uint8_t *p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::fudpframe::udp_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_udp = num_of_headers[HEADER_TYPE_UDP];
	headers[FIRST_UDP_FRAME_POS + num_of_udp].frame.reset(data, sizeof(struct rofl::fudpframe::udp_hdr_t));
	headers[FIRST_UDP_FRAME_POS + num_of_udp].present = true;
	num_of_headers[HEADER_TYPE_UDP] = num_of_udp+1;

	//Set reference
	rofl::fudpframe *udp = &headers[FIRST_UDP_FRAME_POS + num_of_udp].frame; 
	//Increment pointers and decrement remaining payload size
	p_ptr += sizeof(struct rofl::fudpframe::udp_hdr_t);
	p_len -= sizeof(struct rofl::fudpframe::udp_hdr_t);


	if (p_len > 0){
		//TODO: something 
	}
}



void static_pktclassifier::parse_tcp(uint8_t *data, size_t datalen){

	uint8_t *p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::ftcpframe::tcp_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_tcp = num_of_headers[HEADER_TYPE_TCP];
	headers[FIRST_TCP_FRAME_POS + num_of_tcp].frame.reset(data, sizeof(struct rofl::ftcpframe::tcp_hdr_t));
	headers[FIRST_TCP_FRAME_POS + num_of_tcp].present = true;
	num_of_headers[HEADER_TYPE_TCP] = num_of_tcp+1;

	//Set reference
	rofl::ftcpframe *tcp = &headers[FIRST_TCP_FRAME_POS + num_of_tcp].frame; 

	//Increment pointers and decrement remaining payload size
	p_ptr += sizeof(struct rofl::ftcpframe::tcp_hdr_t);
	p_len -= sizeof(struct rofl::ftcpframe::tcp_hdr_t);

	if (p_len > 0){
		//TODO: something 
	}

}



void static_pktclassifier::parse_sctp( uint8_t *data, size_t datalen){

	uint8_t *p_ptr = data;
	size_t p_len = datalen;

	if (p_len < sizeof(struct rofl::fsctpframe::sctp_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_sctcp = num_of_headers[HEADER_TYPE_SCTCP];
	headers[FIRST_SCTCP_FRAME_POS + num_of_sctcp].frame.reset(data, sizeof(struct rofl::fsctcpframe::sctcp_hdr_t));
	headers[FIRST_SCTCP_FRAME_POS + num_of_sctcp].present = true;
	num_of_headers[HEADER_TYPE_SCTCP] = num_of_sctcp+1;

	//Set reference
	rofl::fsctcpframe *sctcp = &headers[FIRST_SCTCP_FRAME_POS + num_of_sctcp].frame; 

	//Increment pointers and decrement remaining payload size
	p_ptr += sizeof(struct rofl::fsctpframe::sctp_hdr_t);
	p_len -= sizeof(struct rofl::fsctpframe::sctp_hdr_t);

	if (p_len > 0){
		//TODO: something 
	}

}

//Simply moves the first element(start) to the end-1 (rotates), and resets it
void static_pktclassifier::pop_header(enum header_type type, unsigned int start, unsigned int end){

	unsigned int i;

	if(num_of_headers[type] == 0){
		//Do nothing
		assert(0);
		return;
	}else if(num_of_headers[type] == 1)
		//Easy, just mark as 
		headers[start].present = false;
		headers[start].prev = headers[start].next = NULL;
		return;
	}else{
		//Move stuff around

		header_container_t copy = headers[start];

		//Rotate
		for(i=start;i<end-1;i++){
			headers[i] = headers[i+1];
		}
		
		//Reset copy
		copy.present = false;
		copy.prev = copy.next = NULL;
		
		//Set last item
		headers[end-1] = copy;		
	}
	//Decrement header type counter	
	num_of_headers[type]--;	
}

//Pushes frame in the header array. The function assumes there is enough space
void static_pktclassifier::push_header(enum header_type type, unsigned int start, unsigned int end){ 

	unsigned int i;

	if(){
		headers[]
	}

	header_container_t copy = headers[start];

	//Rotate
	for(i=start;i<end-1;i++){
		headers[i] = headers[i+1];
	}
	
	//Reset copy
	copy.present = false;
	copy.prev = copy.next = NULL;
	
	//Set last item
	headers[end-1] = copy;		

	//Decrement header type counter	
	num_of_headers[type]--;	
}

void static_pktclassifier::pop_vlan(){
	// outermost vlan tag, if any, following immediately the initial ethernet header
	if(num_of_headers[HEADER_TYPE_VLAN] == 0 || !headers[FIRST_VLAN_FRAME_POS].present)
		return;

	rofl::fvlanframe* vlan = dynamic_cast<rofl::fvlanframe*> headers[FIRST_VLAN_FRAME_POS].frame;

	if (!vlan)
		return;

	uint16_t ether_type = vlan->get_dl_type();

	pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(rofl::fvlanframe::vlan_hdr_t));

	//Take header out
	pop_header(HEADER_TYPE_VLAN, FIRST_VLAN_FRAME_POS, FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES);

	//Set ether_type of new frame
	ether(0)->shift_right(sizeof(struct rofl::fvlanframe::vlan_hdr_t));
	ether(0)->set_dl_type(ether_type);
}

void static_pktclassifier::pop_mpls(uint16_t ether_type){
	// outermost mpls tag, if any, following immediately the initial ethernet header

	if (num_of_headers[HEADER_TYPE_MPLS] == 0 || !headers[FIRST_MPLS_FRAME_POS].present)
		return;
	
	rofl::fmplsframe* mpls = dynamic_cast<rofl::fmplsframe*> headers[FIRST_MPLS_FRAME_POS].frame;
	
	if (!mpls)
		return;
	pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(rofl::fmplsframe::mpls_hdr_t));

	//Take header out
	pop_header(HEADER_TYPE_MPLS, FIRST_MPLS_FRAME_POS, FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES);

	ether(0)->shift_right(sizeof(rofl::fmplsframe::mpls_hdr_t));
	ether(0)->set_dl_type(ether_type);
}

void
static_pktclassifier::pop_pppoe(uint16_t ether_type){
	// outermost mpls tag, if any, following immediately the initial ethernet header
	if(num_of_headers[HEADER_TYPE_PPPOE] == 0 || !headers[FIRST_PPPOE_FRAME_POS].present)
		return;

	switch (ether(0)->get_dl_type()) {
		case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
		{
			pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(rofl::fpppoeframe::pppoe_hdr_t));
			if (this->pppoe(0)) {
				//Take header out
				pop_header(HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);

				frame_pop(pppoe(0));
			}
			ether(0)->shift_right(sizeof(rofl::fpppoeframe::pppoe_hdr_t));
		}
			break;
		case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
		{
			pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t),
							sizeof(rofl::fpppoeframe::pppoe_hdr_t) + sizeof(rofl::fpppframe::ppp_hdr_t));
			if (this->pppoe(0)) {
				pop_header(HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);
			}
			if (ppp(0)) {
				pop_header(HEADER_TYPE_PPP, FIRST_PPP_FRAME_POS, FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES);
			}
			ether(0)->shift_right(sizeof(rofl::fpppoeframe::pppoe_hdr_t) + sizeof(rofl::fpppframe::ppp_hdr_t));
		}
		break;
	}

	ether(0)->set_dl_type(ether_type);
}


/* PUSH operations
 *
 * we want to avoid a total re-classification of the packet, so we make
 * some assumptions:
 * - the packet is stored in one consecutive buffer
 * - a push operation always occurs behind the initial ethernet header (ether(0))
 * - byte insertion moves the affected first ethernet header to the left
 *   re-classification is necessary, but ether(0) must be readjusted
 */

rofl::fvlanframe* static_pktclassifier::push_vlan(uint16_t ether_type){

	if ((NULL == ether(0)) || num_of_headers[HEADER_TYPE_VLAN] == MAX_VLAN_FRAMES ){
		return NULL;
	}
	
	if(!is_classified)
		classify(); // this ensures that ether(0) exists

	uint16_t inner_ether_type = ether(0)->get_dl_type();

	/*
	 * this invalidates ether(0), as it shifts ether(0) to the left
	 */
	if (pkt_push(sizeof(rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fvlanframe::vlan_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}

	/*
	 * adjust ether(0): move one vlan tag to the left
	 */
	ether(0)->shift_left(sizeof(struct rofl::fvlanframe::vlan_hdr_t));

	/*
	 * create new vlan tag
	 */
	rofl::fvlanframe *vlan = new rofl::fvlanframe(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fvlanframe::vlan_hdr_t));

	/*
	 * append the new fvlanframe instance to ether(0)
	 */
	frame_insert(ether(0), vlan);

	/*
	 * set default values in vlan tag
	 */
	if (this->vlan(1)){
		vlan->set_dl_vlan_id(this->vlan(1)->get_dl_vlan_id());
		vlan->set_dl_vlan_pcp(this->vlan(1)->get_dl_vlan_pcp());
	} else {
		vlan->set_dl_vlan_id(0x0000);
		vlan->set_dl_vlan_pcp(0x00);
	}
	vlan->set_dl_type(inner_ether_type);
	ether(0)->set_dl_type(ether_type);

	return vlan;
}

rofl::fmplsframe* static_pktclassifier::push_mpls(uint16_t ether_type){

	if(!is_classified)
		classify();

	/*
	 * this invalidates ether(0), as it shifts ether(0) to the left
	 */
	if (pkt_push(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fmplsframe::mpls_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}

	/*
	 * adjust ether(0): move one mpls tag to the left
	 */
	ether(0)->shift_left(sizeof(struct rofl::fmplsframe::mpls_hdr_t));

	ether(0)->set_dl_type(ether_type);

	/*
	 * create new mpls tag
	 */
	rofl::fmplsframe *mpls = new rofl::fmplsframe(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fmplsframe::mpls_hdr_t));

	/*
	 * append the new fmplsframe instance to ether(0)
	 */
	frame_insert(ether(0), mpls);

	/*
	 * set default values in mpls tag
	 */
	if (this->mpls(1) != this->mpls(0)){
		mpls->set_mpls_bos(false);
		mpls->set_mpls_label(this->mpls(1)->get_mpls_label());
		mpls->set_mpls_tc(this->mpls(1)->get_mpls_tc());
		mpls->set_mpls_ttl(this->mpls(1)->get_mpls_ttl());
	} else {
		mpls->set_mpls_bos(true);
		mpls->set_mpls_label(0x0000);
		mpls->set_mpls_tc(0x00);
		mpls->set_mpls_ttl(0x00);
	}

	return mpls;
}

rofl::fpppoeframe*
static_pktclassifier::push_pppoe(uint16_t ether_type){

	if(!is_classified)
		classify();
	
	if (pppoe(0)){
		// TODO: log error => pppoe tag already exists
		return NULL;
	}

	rofl::fpppoeframe *n_pppoe = NULL; 
	rofl::fpppframe *n_ppp = NULL; 

	switch (ether_type) {
	case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
	{
		unsigned int bytes_to_insert = sizeof(struct rofl::fpppoeframe::pppoe_hdr_t) +
										 sizeof(struct rofl::fpppframe::ppp_hdr_t);

		/*
		 * this invalidates ether(0), as it shifts ether(0) to the left
		 */
		if (pkt_push(sizeof(struct rofl::fetherframe::eth_hdr_t), bytes_to_insert) == ROFL_FAILURE){
			// TODO: log error
			return NULL;
		}

		/*
		 * adjust ether(0): move one pppoe tag to the left
		 */
		ether(0)->shift_left(bytes_to_insert);

		ether(0)->set_dl_type(rofl::fpppoeframe::PPPOE_ETHER_SESSION);

		/*
		 * create new pppoe tag
		 */
		n_pppoe = new rofl::fpppoeframe(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));

		n_ppp = new rofl::fpppframe(n_pppoe->soframe() + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t), sizeof(struct rofl::fpppframe::ppp_hdr_t));

		/*
		 * append the new fpppoeframe instance to ether(0)
		 */
		frame_insert(ether(0), n_pppoe);

		frame_insert(n_pppoe, n_ppp);

		/*
		 * TODO: check if this is an appropiate fix 
		 */
		n_pppoe->set_hdr_length(pkt->get_buffer_length() - sizeof(struct rofl::fetherframe::eth_hdr_t) - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));

		n_ppp->set_ppp_prot(0x0000);
	}
		break;

	case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
	{
		unsigned int bytes_to_insert = sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);

		/*
		 * this invalidates ether(0), as it shifts ether(0) to the left
		 */
		if (pkt_push(ether(0)->payload(), bytes_to_insert) == ROFL_FAILURE){
			// TODO: log error
			return NULL;
		}

		/*
		 * adjust ether(0): move one pppoe tag to the left
		 */
		ether(0)->shift_left(bytes_to_insert);

		ether(0)->set_dl_type(rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY);

		/*
		 * create new pppoe tag
		 */
		n_pppoe = new rofl::fpppoeframe(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));

		/*
		 * append the new fpppoeframe instance to ether(0)
		 */
		frame_insert(ether(0), n_pppoe);

	}
		break;
	}

	/*
	 * set default values in pppoe tag
	 */
	n_pppoe->set_pppoe_code(0x00);
	n_pppoe->set_pppoe_sessid(0x0000);
	n_pppoe->set_pppoe_type(rofl::fpppoeframe::PPPOE_TYPE);
	n_pppoe->set_pppoe_vers(rofl::fpppoeframe::PPPOE_VERSION);

	return NULL;
}


void static_pktclassifier::dump(){

	ROFL_DEBUG("datapacketx86(%p) soframe: %p framelen: %zu\n", this, pkt->get_buffer(), pkt->get_buffer_length());
	rofl::fframe *frame = fhead;

//	while (NULL != frame) {
//		ROFL_ERR("%s\n", frame->c_str());
//		frame = frame->next;
//	}
	

	rofl::fframe content(pkt->get_buffer(), pkt->get_buffer_length());
	ROFL_DEBUG("content: %s\n", content.c_str());

}


size_t static_pktclassifier::get_pkt_len(rofl::fframe *from, rofl::fframe *to){

	rofl::fframe *curr = (from != 0) ? from : fhead;
	//rofl::fframe *last =   (to != 0) ?   to : ftail;

	size_t len = 0;

	while ((curr != 0)){// && (curr->next != last)) {
		len += curr->framelen();
		curr = curr->next;
	}
	return len;

}
