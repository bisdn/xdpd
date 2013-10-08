#include "static_pktclassifier.h"
#include <rofl/common/utils/c_logger.h>
#include "../datapacketx86.h"

using namespace xdpd::gnu_linux;

//Constructor&destructor
static_pktclassifier::static_pktclassifier(datapacketx86* pkt_ref) :
	packetclassifier(pkt_ref)
{
	unsigned int i;

	classify_reset();	
	
	/*
	* Initialize fframes
	*/
	
	//Ether
	for(i=0;i<MAX_ETHER_FRAMES;i++){
		headers[FIRST_ETHER_FRAME_POS+i].frame = new rofl::fetherframe(NULL, 0);		
		headers[FIRST_ETHER_FRAME_POS+i].type = HEADER_TYPE_ETHER;
	}
	//vlan
	for(i=0;i<MAX_VLAN_FRAMES;i++){
		headers[FIRST_VLAN_FRAME_POS+i].frame = new rofl::fvlanframe(NULL, 0);		
		headers[FIRST_VLAN_FRAME_POS+i].type = HEADER_TYPE_VLAN;
	}
	//mpls
	for(i=0;i<MAX_MPLS_FRAMES;i++){
		headers[FIRST_MPLS_FRAME_POS+i].frame = new rofl::fmplsframe(NULL, 0);		
		headers[FIRST_MPLS_FRAME_POS+i].type = HEADER_TYPE_MPLS;
	}
	//arpv4
	for(i=0;i<MAX_ARPV4_FRAMES;i++){
		headers[FIRST_ARPV4_FRAME_POS+i].frame = new rofl::farpv4frame(NULL, 0);		
		headers[FIRST_ARPV4_FRAME_POS+i].type = HEADER_TYPE_ARPV4;
	}
	//ipv4
	for(i=0;i<MAX_IPV4_FRAMES;i++){
		headers[FIRST_IPV4_FRAME_POS+i].frame = new rofl::fipv4frame(NULL, 0);		
		headers[FIRST_IPV4_FRAME_POS+i].type = HEADER_TYPE_IPV4;
	}
	//icmpv4
	for(i=0;i<MAX_ICMPV4_FRAMES;i++){
		headers[FIRST_ICMPV4_FRAME_POS+i].frame = new rofl::ficmpv4frame(NULL, 0);		
		headers[FIRST_ICMPV4_FRAME_POS+i].type = HEADER_TYPE_ICMPV4;
	}
	//ipv6
	for(i=0;i<MAX_IPV6_FRAMES;i++){
		headers[FIRST_IPV6_FRAME_POS+i].frame = new rofl::fipv6frame(NULL, 0);		
		headers[FIRST_IPV6_FRAME_POS+i].type = HEADER_TYPE_IPV6;
	}
	//icmpv6
	for(i=0;i<MAX_ICMPV6_FRAMES;i++){
		headers[FIRST_ICMPV6_FRAME_POS+i].frame = new rofl::ficmpv6frame(NULL, 0);
		headers[FIRST_ICMPV6_FRAME_POS+i].type = HEADER_TYPE_ICMPV6;
	}
	//udp
	for(i=0;i<MAX_UDP_FRAMES;i++){
		headers[FIRST_UDP_FRAME_POS+i].frame = new rofl::fudpframe(NULL, 0);		
		headers[FIRST_UDP_FRAME_POS+i].type = HEADER_TYPE_UDP;
	}
	//tcp
	for(i=0;i<MAX_TCP_FRAMES;i++){
		headers[FIRST_TCP_FRAME_POS+i].frame = new rofl::ftcpframe(NULL, 0);		
		headers[FIRST_TCP_FRAME_POS+i].type = HEADER_TYPE_TCP;
	}
	//sctp
	for(i=0;i<MAX_SCTP_FRAMES;i++){
		headers[FIRST_SCTP_FRAME_POS+i].frame = new rofl::fsctpframe(NULL, 0);		
		headers[FIRST_SCTP_FRAME_POS+i].type = HEADER_TYPE_SCTP;
	}
	//pppoe
	for(i=0;i<MAX_PPPOE_FRAMES;i++){
		headers[FIRST_PPPOE_FRAME_POS+i].frame = new rofl::fpppoeframe(NULL, 0);		
		headers[FIRST_PPPOE_FRAME_POS+i].type = HEADER_TYPE_PPPOE;
	}

	//ppp
	for(i=0;i<MAX_PPP_FRAMES;i++){
		headers[FIRST_PPP_FRAME_POS+i].frame = new rofl::fpppframe(NULL, 0);		
		headers[FIRST_PPP_FRAME_POS+i].type = HEADER_TYPE_PPP;
	}
	//gtp
	for (i=0;i<MAX_GTP_FRAMES;i++){
		headers[FIRST_GTP_FRAME_POS+i].frame = new rofl::fgtpuframe(NULL, 0);
		headers[FIRST_GTP_FRAME_POS+i].type = HEADER_TYPE_GTP;
	}

	//Add more here...
}

static_pktclassifier::~static_pktclassifier(){
	
	for(unsigned i=0; i<MAX_HEADERS; i++){
		if(headers[i].frame)
			delete headers[i].frame;		
	}

}



void static_pktclassifier::classify(void){
	if(is_classified)
		classify_reset();
	parse_ether(pkt->get_buffer(), pkt->get_buffer_length());
	is_classified = true;
}


/*
* Getters
*/

rofl::fetherframe* static_pktclassifier::ether(int idx) const{

	unsigned int pos;	

	if(idx > (int)MAX_ETHER_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ETHER_FRAME_POS + num_of_headers[HEADER_TYPE_ETHER] - 1;
	else
		pos = FIRST_ETHER_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::fetherframe*) headers[pos].frame;	
	return NULL;
}

rofl::fvlanframe* static_pktclassifier::vlan(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_VLAN_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_VLAN_FRAME_POS + num_of_headers[HEADER_TYPE_VLAN] - 1;
	else
		pos = FIRST_VLAN_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::fvlanframe*) headers[pos].frame;
	return NULL;
}

rofl::fmplsframe* static_pktclassifier::mpls(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_MPLS_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_MPLS_FRAME_POS + num_of_headers[HEADER_TYPE_MPLS] - 1;
	else
		pos = FIRST_MPLS_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::fmplsframe*) headers[pos].frame;
	return NULL;
}

rofl::farpv4frame* static_pktclassifier::arpv4(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_ARPV4_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ARPV4_FRAME_POS + num_of_headers[HEADER_TYPE_ARPV4] - 1;
	else
		pos = FIRST_ARPV4_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::farpv4frame*) headers[pos].frame;
	return NULL;
}

rofl::fipv4frame* static_pktclassifier::ipv4(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_IPV4_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_IPV4_FRAME_POS + num_of_headers[HEADER_TYPE_IPV4] - 1;
	else
		pos = FIRST_IPV4_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::fipv4frame*) headers[pos].frame;
	return NULL;
}

rofl::ficmpv4frame* static_pktclassifier::icmpv4(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_ICMPV4_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ICMPV4_FRAME_POS + num_of_headers[HEADER_TYPE_ICMPV4] - 1;
	else
		pos = FIRST_ICMPV4_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::ficmpv4frame*) headers[pos].frame;
	return NULL;

}

rofl::fipv6frame* static_pktclassifier::ipv6(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_IPV6_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_IPV6_FRAME_POS + num_of_headers[HEADER_TYPE_IPV6] - 1;
	else
		pos = FIRST_IPV6_FRAME_POS + idx;

	//Return the index
	if(headers[pos].present)
		return (rofl::fipv6frame*) headers[pos].frame;
	return NULL;
}

rofl::ficmpv6frame* static_pktclassifier::icmpv6(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_ICMPV6_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ICMPV6_FRAME_POS + num_of_headers[HEADER_TYPE_ICMPV6] - 1;
	else
		pos = FIRST_ICMPV6_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::ficmpv6frame*) headers[pos].frame;
	return NULL;

}

rofl::fudpframe* static_pktclassifier::udp(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_UDP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_UDP_FRAME_POS + num_of_headers[HEADER_TYPE_UDP] - 1;
	else
		pos = FIRST_UDP_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::fudpframe*) headers[pos].frame;
	return NULL;
}

rofl::ftcpframe* static_pktclassifier::tcp(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_TCP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_TCP_FRAME_POS + num_of_headers[HEADER_TYPE_TCP] - 1;
	else
		pos = FIRST_TCP_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::ftcpframe*) headers[pos].frame;
	return NULL;
}

rofl::fsctpframe* static_pktclassifier::sctp(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_SCTP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_SCTP_FRAME_POS + num_of_headers[HEADER_TYPE_SCTP] - 1;
	else
		pos = FIRST_SCTP_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::fsctpframe*) headers[pos].frame;
	return NULL;
}

rofl::fpppoeframe* static_pktclassifier::pppoe(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_PPPOE_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_PPPOE_FRAME_POS + num_of_headers[HEADER_TYPE_PPPOE] - 1;
	else
		pos = FIRST_PPPOE_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::fpppoeframe*) headers[pos].frame;
	return NULL;
}

rofl::fpppframe* static_pktclassifier::ppp(int idx) const
{
	unsigned int pos;	

	if(idx > (int)MAX_PPP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_PPP_FRAME_POS + num_of_headers[HEADER_TYPE_PPP] - 1;
	else
		pos = FIRST_PPP_FRAME_POS + idx;	

	//Return the index
	if(headers[pos].present)
		return (rofl::fpppframe*) headers[pos].frame;
	return NULL;
}

rofl::fgtpuframe* static_pktclassifier::gtp(int idx) const
{
	unsigned int pos;

	if(idx > (int)MAX_GTP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_GTP_FRAME_POS + num_of_headers[HEADER_TYPE_GTP] - 1;
	else
		pos = FIRST_GTP_FRAME_POS + idx;

	//Return the index
	if(headers[pos].present)
		return (rofl::fgtpuframe*) headers[pos].frame;
	return NULL;
}

/*
* Classify code...
*/
void static_pktclassifier::parse_ether(uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::fetherframe::eth_hdr_t)){return;}

	//Set frame
	unsigned int num_of_ether = num_of_headers[HEADER_TYPE_ETHER];
	headers[FIRST_ETHER_FRAME_POS + num_of_ether].frame->reset(data, datalen/*sizeof(struct rofl::fetherframe::eth_hdr_t)*/);
	headers[FIRST_ETHER_FRAME_POS + num_of_ether].present = true;
	num_of_headers[HEADER_TYPE_ETHER] = num_of_ether+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::fetherframe::eth_hdr_t);
	datalen -= sizeof(struct rofl::fetherframe::eth_hdr_t);

	//Set pointer to header
	rofl::fetherframe* ether_header = (rofl::fetherframe*) headers[FIRST_ETHER_FRAME_POS + num_of_ether].frame;
	
	eth_type = ether_header->get_dl_type();

	switch (eth_type) {
		case rofl::fvlanframe::VLAN_CTAG_ETHER:
		case rofl::fvlanframe::VLAN_STAG_ETHER:
		case rofl::fvlanframe::VLAN_ITAG_ETHER:
			{
				parse_vlan(data, datalen);
			}
			break;
		case rofl::fmplsframe::MPLS_ETHER:
		case rofl::fmplsframe::MPLS_ETHER_UPSTREAM:
			{
				parse_mpls(data, datalen);
			}
			break;
		case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
		case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
			{
				parse_pppoe(data, datalen);
			}
			break;
		case rofl::farpv4frame::ARPV4_ETHER:
			{
				parse_arpv4(data, datalen);
			}
			break;
		case rofl::fipv4frame::IPV4_ETHER:
			{
				parse_ipv4(data, datalen);
			}
			break;
		case rofl::fipv6frame::IPV6_ETHER:
			{
				parse_ipv6(data,datalen);
			}
			break;
		default:
			{
				
			}
			break;
	}
}

void static_pktclassifier::parse_vlan( uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::fvlanframe::vlan_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_vlan = num_of_headers[HEADER_TYPE_VLAN];
	headers[FIRST_VLAN_FRAME_POS + num_of_vlan].frame->reset(data, datalen);
	headers[FIRST_VLAN_FRAME_POS + num_of_vlan].present = true;
	num_of_headers[HEADER_TYPE_VLAN] = num_of_vlan+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::fvlanframe::vlan_hdr_t);
	datalen -= sizeof(struct rofl::fvlanframe::vlan_hdr_t);

	//Set pointer to header
	rofl::fvlanframe* vlan = (rofl::fvlanframe*) headers[FIRST_VLAN_FRAME_POS + num_of_vlan].frame;
	
	eth_type = vlan->get_dl_type();

	switch (eth_type) {
		case rofl::fvlanframe::VLAN_CTAG_ETHER:
		case rofl::fvlanframe::VLAN_STAG_ETHER:
		case rofl::fvlanframe::VLAN_ITAG_ETHER:
			{
				parse_vlan(data, datalen);
			}
			break;
		case rofl::fmplsframe::MPLS_ETHER:
		case rofl::fmplsframe::MPLS_ETHER_UPSTREAM:
			{
				parse_mpls(data, datalen);
			}
			break;
		case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
		case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
			{
				parse_pppoe(data, datalen);
			}
			break;
		case rofl::farpv4frame::ARPV4_ETHER:
			{
				parse_arpv4(data, datalen);
			}
			break;
		case rofl::fipv4frame::IPV4_ETHER:
			{
				parse_ipv4(data, datalen);
			}
			break;
		default:
			{

			}
			break;
	}
}

void static_pktclassifier::parse_mpls(uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::fmplsframe::mpls_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_mpls = num_of_headers[HEADER_TYPE_MPLS];
	headers[FIRST_MPLS_FRAME_POS + num_of_mpls].frame->reset(data, datalen);
	headers[FIRST_MPLS_FRAME_POS + num_of_mpls].present = true;
	num_of_headers[HEADER_TYPE_MPLS] = num_of_mpls+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::fmplsframe::mpls_hdr_t);
	datalen -= sizeof(struct rofl::fmplsframe::mpls_hdr_t);

	//Set pointer to header
	rofl::fmplsframe* mpls = (rofl::fmplsframe*) headers[FIRST_MPLS_FRAME_POS + num_of_mpls].frame;
	
	if (not mpls->get_mpls_bos()){

		parse_mpls(data, datalen);

	}else{
		
		//TODO: We could be trying to guess if payload is IPv4/v6 and continue parsing...
	}
}



void static_pktclassifier::parse_pppoe(uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::fpppoeframe::pppoe_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_pppoe = num_of_headers[HEADER_TYPE_PPPOE];
	headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].frame->reset(data, datalen);
	headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].present = true;
	num_of_headers[HEADER_TYPE_PPPOE] = num_of_pppoe+1;
	
	rofl::fpppoeframe* pppoe = (rofl::fpppoeframe*) headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].frame;

	switch (eth_type) {
		case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
			{
				datalen -= sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);

				uint16_t pppoe_len = pppoe->get_hdr_length() > datalen ? datalen : pppoe->get_hdr_length();

				/*
				 * parse any pppoe service tags
				 */
				pppoe->unpack(data, sizeof(struct rofl::fpppoeframe::pppoe_hdr_t) + pppoe_len);


				/*
				 * any remaining bytes after the pppoe tags => padding?
				 */
				if (datalen > pppoe->tags.length())
				{
					//TODO: Continue parsing??	
				}
			}
			break;
		case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
			{
				//Increment pointers and decrement remaining payload size
				data += sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);
				datalen -= sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);

				parse_ppp(data, datalen);
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

	if (datalen < sizeof(struct rofl::fpppframe::ppp_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_ppp = num_of_headers[HEADER_TYPE_PPP];
	headers[FIRST_PPP_FRAME_POS + num_of_ppp].frame->reset(data, datalen);
	headers[FIRST_PPP_FRAME_POS + num_of_ppp].present = true;
	num_of_headers[HEADER_TYPE_PPP] = num_of_ppp+1;

	//Set reference
	rofl::fpppframe* ppp = (rofl::fpppframe*) headers[FIRST_PPP_FRAME_POS + num_of_ppp].frame; 

	//Increment pointers and decrement remaining payload size
	switch (ppp->get_ppp_prot()) {
		case rofl::fpppframe::PPP_PROT_IPV4:
			{
				//Increment pointers and decrement remaining payload size
				data += sizeof(struct rofl::fpppframe::ppp_hdr_t);
				datalen -= sizeof(struct rofl::fpppframe::ppp_hdr_t);

				parse_ipv4(data, datalen);
			}
			break;
		default:
			{
				ppp->unpack(data, datalen);
			}
			break;
	}
}



void static_pktclassifier::parse_arpv4(uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::farpv4frame::arpv4_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_arpv4 = num_of_headers[HEADER_TYPE_ARPV4];
	headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].frame->reset(data, datalen);
	headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].present = true;
	num_of_headers[HEADER_TYPE_ARPV4] = num_of_arpv4+1;

	//Set reference
	//rofl::farpv4frame *arpv4 = headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].frame; 

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::farpv4frame::arpv4_hdr_t);
	datalen -= sizeof(struct rofl::farpv4frame::arpv4_hdr_t);

	if (datalen > 0){
		//TODO: something?
	}
}



void static_pktclassifier::parse_ipv4(uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::fipv4frame::ipv4_hdr_t)) { return; }
	
	//Set frame
	unsigned int num_of_ipv4 = num_of_headers[HEADER_TYPE_IPV4];
	headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].frame->reset(data, datalen);
	headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].present = true;
	num_of_headers[HEADER_TYPE_IPV4] = num_of_ipv4+1;

	//Set reference
	rofl::fipv4frame *ipv4 = (rofl::fipv4frame*) headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].frame; 

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::fipv4frame::ipv4_hdr_t);
	datalen -= sizeof(struct rofl::fipv4frame::ipv4_hdr_t);

	if (ipv4->has_MF_bit_set()){
		// TODO: fragment handling

		return;
	}

	// FIXME: IP header with options


	switch (ipv4->get_ipv4_proto()) {
		case rofl::fipv4frame::IPV4_IP_PROTO:
			{
				parse_ipv4(data, datalen);
			}
			break;
		case rofl::ficmpv4frame::ICMPV4_IP_PROTO:
			{
				parse_icmpv4(data, datalen);
			}
			break;
		case rofl::fudpframe::UDP_IP_PROTO:
			{
				parse_udp(data, datalen);
			}
			break;
		case rofl::ftcpframe::TCP_IP_PROTO:
			{
				parse_tcp(data, datalen);
			}
			break;
		case rofl::fsctpframe::SCTP_IP_PROTO:
			{
				parse_sctp(data, datalen);
			}
			break;
		default:
			{
			
			}
			break;
	}
}



void static_pktclassifier::parse_icmpv4( uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_icmpv4 = num_of_headers[HEADER_TYPE_ICMPV4];
	headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].frame->reset(data, datalen);
	headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].present = true;
	num_of_headers[HEADER_TYPE_ICMPV4] = num_of_icmpv4+1;

	//Set reference
	//rofl::ficmpv4frame *icmpv4 = (rofl::ficmpv4frame*) headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].frame; 

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t);
	datalen -= sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t);


	if (datalen > 0){
		//TODO: something?	
	}
}

//IPv6 & ICMPv6
void static_pktclassifier::parse_ipv6(uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::fipv6frame::ipv6_hdr_t)) { return; }
	
	//Set frame
	unsigned int num_of_ipv6 = num_of_headers[HEADER_TYPE_IPV6];
	headers[FIRST_IPV6_FRAME_POS + num_of_ipv6].frame->reset(data, datalen);
	headers[FIRST_IPV6_FRAME_POS + num_of_ipv6].present = true;
	num_of_headers[HEADER_TYPE_IPV6] = num_of_ipv6+1;

	//Set reference
	rofl::fipv6frame *ipv6 = (rofl::fipv6frame*) headers[FIRST_IPV6_FRAME_POS + num_of_ipv6].frame; 

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::fipv6frame::ipv6_hdr_t);
	datalen -= sizeof(struct rofl::fipv6frame::ipv6_hdr_t);

	// FIXME: IP header with options

	switch (ipv6->get_next_header()) {
		case rofl::fipv4frame::IPV4_IP_PROTO:
			{
				parse_ipv4(data, datalen);
			}
			break;
		case rofl::ficmpv4frame::ICMPV4_IP_PROTO:
			{
				parse_icmpv4(data, datalen);
			}
			break;
		case rofl::fipv6frame::IPV6_IP_PROTO:
			{
				parse_ipv6(data, datalen);
			}
			break;
		case rofl::ficmpv6frame::ICMPV6_IP_PROTO:
			{
				parse_icmpv6(data, datalen);
			}
			break;
		case rofl::fudpframe::UDP_IP_PROTO:
			{
				parse_udp(data, datalen);
			}
			break;
		case rofl::ftcpframe::TCP_IP_PROTO:
			{
				parse_tcp(data, datalen);
			}
			break;
		case rofl::fsctpframe::SCTP_IP_PROTO:
			{
				parse_sctp(data, datalen);
			}
			break;
		default:
			{
			
			}
			break;
	}
}



void static_pktclassifier::parse_icmpv6( uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::ficmpv6frame::icmpv6_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_icmpv6 = num_of_headers[HEADER_TYPE_ICMPV6];
	headers[FIRST_ICMPV6_FRAME_POS + num_of_icmpv6].frame->reset(data, datalen);
	headers[FIRST_ICMPV6_FRAME_POS + num_of_icmpv6].present = true;
	num_of_headers[HEADER_TYPE_ICMPV6] = num_of_icmpv6+1;

	//Set reference
	//rofl::ficmpv6frame *icmpv6 = (rofl::ficmpv6frame*) headers[FIRST_ICMPV6_FRAME_POS + num_of_icmpv6].frame; 

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::ficmpv6frame::icmpv6_hdr_t);
	datalen -= sizeof(struct rofl::ficmpv6frame::icmpv6_hdr_t);


	if (datalen > 0){
		//TODO: something?	
	}
}

void static_pktclassifier::parse_udp(uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::fudpframe::udp_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_udp = num_of_headers[HEADER_TYPE_UDP];
	headers[FIRST_UDP_FRAME_POS + num_of_udp].frame->reset(data, datalen);
	headers[FIRST_UDP_FRAME_POS + num_of_udp].present = true;
	num_of_headers[HEADER_TYPE_UDP] = num_of_udp+1;

	//Set reference
	rofl::fudpframe *udp = (rofl::fudpframe*) headers[FIRST_UDP_FRAME_POS + num_of_udp].frame;
	
	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::fudpframe::udp_hdr_t);
	datalen -= sizeof(struct rofl::fudpframe::udp_hdr_t);

	if (datalen > 0){
		switch (udp->get_dport()) {
		case rofl::fgtpuframe::GTPU_UDP_PORT: {
			parse_gtp(data, datalen);
		} break;
		default: {
			//TODO: something
		} break;
		}
	}
}



void static_pktclassifier::parse_tcp(uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::ftcpframe::tcp_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_tcp = num_of_headers[HEADER_TYPE_TCP];
	headers[FIRST_TCP_FRAME_POS + num_of_tcp].frame->reset(data, datalen);
	headers[FIRST_TCP_FRAME_POS + num_of_tcp].present = true;
	num_of_headers[HEADER_TYPE_TCP] = num_of_tcp+1;

	//Set reference
	//rofl::ftcpframe *tcp = (rofl::ftcpframe*) headers[FIRST_TCP_FRAME_POS + num_of_tcp].frame; 

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::ftcpframe::tcp_hdr_t);
	datalen -= sizeof(struct rofl::ftcpframe::tcp_hdr_t);

	if (datalen > 0){
		//TODO: something 
	}

}



void static_pktclassifier::parse_sctp( uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::fsctpframe::sctp_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_sctp = num_of_headers[HEADER_TYPE_SCTP];
	headers[FIRST_SCTP_FRAME_POS + num_of_sctp].frame->reset(data, datalen);
	headers[FIRST_SCTP_FRAME_POS + num_of_sctp].present = true;
	num_of_headers[HEADER_TYPE_SCTP] = num_of_sctp+1;

	//Set reference
	//rofl::fsctpframe *sctp = (rofl::fsctpframe*) headers[FIRST_SCTP_FRAME_POS + num_of_sctp].frame; 

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::fsctpframe::sctp_hdr_t);
	datalen -= sizeof(struct rofl::fsctpframe::sctp_hdr_t);

	if (datalen > 0){
		//TODO: something 
	}

}


void static_pktclassifier::parse_gtp( uint8_t *data, size_t datalen){

	if (datalen < sizeof(struct rofl::fgtpuframe::gtpu_base_hdr_t)) { return; }

	//Set frame
	unsigned int num_of_gtp = num_of_headers[HEADER_TYPE_GTP];
	headers[FIRST_GTP_FRAME_POS + num_of_gtp].frame->reset(data, datalen);
	headers[FIRST_GTP_FRAME_POS + num_of_gtp].present = true;
	num_of_headers[HEADER_TYPE_GTP] = num_of_gtp+1;

	//Set reference
	//rofl::fgtpuframe *gtp = (rofl::fgtpuframe*) headers[FIRST_GTP_FRAME_POS + num_of_gtp].frame;

	//Increment pointers and decrement remaining payload size
	data += sizeof(struct rofl::fgtpuframe::gtpu_base_hdr_t);
	datalen -= sizeof(struct rofl::fgtpuframe::gtpu_base_hdr_t);

	if (datalen > 0){
		//TODO: something
	}

}



//Simply moves the first element(start) to the end-1 (rotates), and resets it
void static_pktclassifier::pop_header(enum header_type type, unsigned int start, unsigned int end){

	if(num_of_headers[type] == 0){
		//Do nothing
		assert(0);
		return;
	}else if(num_of_headers[type] == 1){
		//Easy, just mark as 
		headers[start].present = false;
		//headers[start].prev = headers[start].next = NULL;
		return;
	}else{
		//Move stuff around

		header_container_t copy = headers[start];

		//Rotate
		for(unsigned i=start;i<end-1;i++){
			headers[i] = headers[i+1];
		}
		
		//Reset copy
		copy.present = false;
		//copy.prev = copy.next = NULL;
		
		//Set last item
		headers[end-1] = copy;		
	}
	//Decrement header type counter	
	num_of_headers[type]--;	
}

//Pushes frame in the header array. The function assumes there is enough space
void static_pktclassifier::push_header(enum header_type type, unsigned int start, unsigned int end){ 

	unsigned int i;
	header_container_t *header;

	if(headers[start].present){
		header_container_t copy = headers[end-1];
		
		//Rotate frames (push them)
		for(i=start;i<end-1;i++){
			headers[i] = headers[i+1];
		}

		headers[start] = copy;

	}

	//Get the copy	
	header = &headers[start];
	
	//Set presence
	header->present = true;
	//header->copy.prev = NULL; 
	
	//Increment header type counter	
	num_of_headers[type]++;	
}

void static_pktclassifier::pop_vlan(){
	
	rofl::fetherframe* ether_header;
	
	// outermost vlan tag, if any, following immediately the initial ethernet header
	if(num_of_headers[HEADER_TYPE_VLAN] == 0 || !headers[FIRST_VLAN_FRAME_POS].present)
		return;

	rofl::fvlanframe* vlan = (rofl::fvlanframe*) headers[FIRST_VLAN_FRAME_POS].frame;

	if (!vlan)
		return;

	uint16_t ether_type = vlan->get_dl_type();

	pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(rofl::fvlanframe::vlan_hdr_t));

	//Take header out
	pop_header(HEADER_TYPE_VLAN, FIRST_VLAN_FRAME_POS, FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES);

	//Recover the ether(0)
	ether_header = ether(0);
	
	//Set ether_type of new frame
	ether_header->shift_right(sizeof(struct rofl::fvlanframe::vlan_hdr_t));
	ether_header->set_dl_type(ether_type);
	ether_header->reset(ether_header->soframe(), ether_header->framelen() - sizeof(struct rofl::fvlanframe::vlan_hdr_t));
}

void static_pktclassifier::pop_mpls(uint16_t ether_type){
	// outermost mpls tag, if any, following immediately the initial ethernet header
	
	rofl::fetherframe* ether_header;
	unsigned int current_length;	

	if (num_of_headers[HEADER_TYPE_MPLS] == 0 || !headers[FIRST_MPLS_FRAME_POS].present)
		return;
	
	rofl::fmplsframe* mpls = (rofl::fmplsframe*) headers[FIRST_MPLS_FRAME_POS].frame;
	
	if (!mpls)
		return;

	//Recover the ether(0)
	ether_header = ether(0);
	current_length = ether_header->framelen(); 
	
	pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(rofl::fmplsframe::mpls_hdr_t));

	//Take header out
	pop_header(HEADER_TYPE_MPLS, FIRST_MPLS_FRAME_POS, FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES);

	ether_header->shift_right(sizeof(rofl::fmplsframe::mpls_hdr_t));
	ether_header->set_dl_type(ether_type);
	ether_header->reset(ether_header->soframe(), current_length - sizeof(struct rofl::fmplsframe::mpls_hdr_t));
}

void static_pktclassifier::pop_pppoe(uint16_t ether_type){
	
	rofl::fetherframe* ether_header;
	
	// outermost mpls tag, if any, following immediately the initial ethernet header
	if(num_of_headers[HEADER_TYPE_PPPOE] == 0 || !headers[FIRST_PPPOE_FRAME_POS].present)
		return;

	//Recover the ether(0)
	ether_header = ether(0);

	switch (ether_header->get_dl_type()) {
		case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
		{
			pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(rofl::fpppoeframe::pppoe_hdr_t));
			if (this->pppoe(0)) {
				//Take header out
				pop_header(HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);

				pop_header(HEADER_TYPE_PPP, FIRST_PPP_FRAME_POS, FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES);
			}
			ether_header->shift_right(sizeof(rofl::fpppoeframe::pppoe_hdr_t));
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
			ether_header->shift_right(sizeof(rofl::fpppoeframe::pppoe_hdr_t) + sizeof(rofl::fpppframe::ppp_hdr_t));
		}
		break;
	}

	ether_header->set_dl_type(ether_type);
	ether_header->reset(ether_header->soframe(), ether_header->framelen() - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));

}

void static_pktclassifier::pop_gtp(uint16_t ether_type){

	// assumption: UDP -> GTP

	// an ip header must be present
	if((num_of_headers[HEADER_TYPE_IPV4] == 0) || (!headers[FIRST_IPV4_FRAME_POS].present) || (num_of_headers[HEADER_TYPE_IPV4] > 1))
		return;

	// a udp header must be present
	if((num_of_headers[HEADER_TYPE_UDP] == 0) || (!headers[FIRST_UDP_FRAME_POS].present) || (num_of_headers[HEADER_TYPE_UDP] > 1))
		return;

	// a gtp header must be present
	if(num_of_headers[HEADER_TYPE_GTP] == 0 || !headers[FIRST_GTP_FRAME_POS].present)
		return;


	// determine effective length of GTP header
	size_t pop_length = sizeof(struct rofl::fipv4frame::ipv4_hdr_t) +
							sizeof(struct rofl::fudpframe::udp_hdr_t);
								gtp(0)->get_hdr_length(); // based on flags set to 1 in GTP header

	//Remove bytes from packet
	pkt_pop(ipv4(0)->soframe(), pop_length);

	//Take headers out
	pop_header(HEADER_TYPE_GTP, FIRST_GTP_FRAME_POS, FIRST_GTP_FRAME_POS+MAX_GTP_FRAMES);
	pop_header(HEADER_TYPE_UDP, FIRST_UDP_FRAME_POS, FIRST_UDP_FRAME_POS+MAX_UDP_FRAMES);
	pop_header(HEADER_TYPE_IPV4, FIRST_IPV4_FRAME_POS, FIRST_IPV4_FRAME_POS+MAX_IPV4_FRAMES);


	for (int i = MAX_VLAN_FRAMES-1; i >= 0; ++i) {
		vlan(i)->shift_right(pop_length);
		vlan(i)->reset(vlan(i)->soframe(), vlan(i)->framelen() - pop_length);
	}
	for (int i = MAX_ETHER_FRAMES-1; i >= 0; ++i) {
		ether(i)->shift_right(pop_length);
		ether(i)->reset(ether(i)->soframe(), ether(i)->framelen() - pop_length);
	}

	if (vlan(-1)) {
		vlan(-1)->set_dl_type(ether_type);
	} else {
		ether(-1)->set_dl_type(ether_type);
	}
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
	
	rofl::fetherframe* ether_header;
	unsigned int current_length;

	if ((NULL == ether(0)) || num_of_headers[HEADER_TYPE_VLAN] == MAX_VLAN_FRAMES ){
		return NULL;
	}
	
	//Recover the ether(0)
	ether_header = ether(0);
	current_length = ether_header->framelen(); 
	
	if(!is_classified)
		classify(); // this ensures that ether(0) exists

	uint16_t inner_ether_type = ether_header->get_dl_type();

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
	ether_header->shift_left(sizeof(struct rofl::fvlanframe::vlan_hdr_t));

	/*
	 * append the new fvlanframe 
	 */
	push_header(HEADER_TYPE_VLAN, FIRST_VLAN_FRAME_POS, FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES);
	
	//Now reset frame 
	ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fvlanframe::vlan_hdr_t));
	
	headers[FIRST_VLAN_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fvlanframe::vlan_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));


	/*
	 * set default values in vlan tag
	 */
	rofl::fvlanframe* vlan_header = this->vlan(0);
	if ( this->vlan(1) ) {
		vlan_header->set_dl_vlan_id(this->vlan(1)->get_dl_vlan_id());
		vlan_header->set_dl_vlan_pcp(this->vlan(1)->get_dl_vlan_pcp());
	} else {
		vlan_header->set_dl_vlan_id(0x0000);
		vlan_header->set_dl_vlan_pcp(0x00);
	}

	vlan_header->set_dl_type(inner_ether_type);
	ether_header->set_dl_type(ether_type);
	
	return vlan_header;
}

rofl::fmplsframe* static_pktclassifier::push_mpls(uint16_t ether_type){
	
	rofl::fetherframe* ether_header;
	rofl::fmplsframe* mpls_header;
	unsigned int current_length;

	if(!is_classified)
		classify();

	//Recover the ether(0)
	ether_header = ether(0);
	current_length = ether_header->framelen(); 
	
	/*
	 * this invalidates ether(0), as it shifts ether(0) to the left
	 */
	if (pkt_push(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fmplsframe::mpls_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}


	/*
	 * adjust ether(0): move one mpls tag to the left
	 */
	ether_header->shift_left(sizeof(struct rofl::fmplsframe::mpls_hdr_t));
	ether_header->set_dl_type(ether_type);

	/*
	 * append the new fmplsframe instance to ether(0)
	 */
	push_header(HEADER_TYPE_MPLS, FIRST_MPLS_FRAME_POS, FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES);
	
	//Now reset frame 
	//Size of ethernet needs to be extended with + MPLS size 
	//MPLS size needs to be ether_header->size + MPLS - ether_header
	ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fmplsframe::mpls_hdr_t));
	headers[FIRST_MPLS_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fmplsframe::mpls_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));


	/*
	 * set default values in mpls tag
	 */
	mpls_header = this->mpls(0);

	if (this->mpls(1)){
		mpls_header->set_mpls_bos(false);
		mpls_header->set_mpls_label(this->mpls(1)->get_mpls_label());
		mpls_header->set_mpls_tc(this->mpls(1)->get_mpls_tc());
		mpls_header->set_mpls_ttl(this->mpls(1)->get_mpls_ttl());
	} else {
		mpls_header->set_mpls_bos(true);
		mpls_header->set_mpls_label(0x0000);
		mpls_header->set_mpls_tc(0x00);
		mpls_header->set_mpls_ttl(0x00);
	}

	return mpls_header;
}

rofl::fpppoeframe* static_pktclassifier::push_pppoe(uint16_t ether_type){
	
	rofl::fetherframe* ether_header;
	unsigned int current_length;

	if(!is_classified)
		classify();
	
	if (pppoe(0)){
		// TODO: log error => pppoe tag already exists
		return NULL;
	}

	//Recover the ether(0)
	ether_header = ether(0);
	current_length = ether_header->framelen(); 
	
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
			ether_header->shift_left(bytes_to_insert);

			ether_header->set_dl_type(rofl::fpppoeframe::PPPOE_ETHER_SESSION);

			/*
			 * append the new fpppoeframe instance to ether(0)
			 */
			push_header(HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);
			push_header(HEADER_TYPE_PPP, FIRST_PPP_FRAME_POS, FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES);
	
			n_pppoe = (rofl::fpppoeframe*)headers[FIRST_PPPOE_FRAME_POS].frame;
			n_ppp = (rofl::fpppframe*)headers[FIRST_PPP_FRAME_POS].frame;

			//Now reset frames 
			ether_header->reset(ether_header->soframe(), current_length + bytes_to_insert);
			n_pppoe->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), ether_header->framelen() - sizeof(struct rofl::fetherframe::eth_hdr_t) );
			n_ppp->reset(n_pppoe->soframe() + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t), n_pppoe->framelen() - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
	
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
			if (pkt_push(ether_header->payload(), bytes_to_insert) == ROFL_FAILURE){
				// TODO: log error
				return NULL;
			}

			/*
			 * adjust ether(0): move one pppoe tag to the left
			 */
			ether_header->shift_left(bytes_to_insert);

			ether_header->set_dl_type(rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY);

			/*
			 * append the new fpppoeframe instance to ether(0)
			 */
			push_header(HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);
			
			//Now reset frame
			//Size of ethernet needs to be extended with +PPPOE size 
			//PPPOE size needs to be ether_header->size + PPPOE - ether_header
			ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
			headers[FIRST_PPPOE_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));

			n_pppoe = (rofl::fpppoeframe*)headers[FIRST_PPPOE_FRAME_POS].frame;


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


rofl::fgtpuframe* static_pktclassifier::push_gtp(uint16_t ether_type){
#if 0
	rofl::fetherframe* ether_header;
	rofl::fmplsframe* mpls_header;
	unsigned int current_length;

	if(!is_classified)
		classify();

	//Recover the ether(0)
	ether_header = ether(0);
	current_length = ether_header->framelen();

	/*
	 * this invalidates ether(0), as it shifts ether(0) to the left
	 */
	if (pkt_push(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fmplsframe::mpls_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}


	/*
	 * adjust ether(0): move one mpls tag to the left
	 */
	ether_header->shift_left(sizeof(struct rofl::fmplsframe::mpls_hdr_t));
	ether_header->set_dl_type(ether_type);

	/*
	 * append the new fmplsframe instance to ether(0)
	 */
	push_header(HEADER_TYPE_MPLS, FIRST_MPLS_FRAME_POS, FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES);

	//Now reset frame
	//Size of ethernet needs to be extended with + MPLS size
	//MPLS size needs to be ether_header->size + MPLS - ether_header
	ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fmplsframe::mpls_hdr_t));
	headers[FIRST_MPLS_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fmplsframe::mpls_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));


	/*
	 * set default values in mpls tag
	 */
	mpls_header = this->mpls(0);

	if (this->mpls(1)){
		mpls_header->set_mpls_bos(false);
		mpls_header->set_mpls_label(this->mpls(1)->get_mpls_label());
		mpls_header->set_mpls_tc(this->mpls(1)->get_mpls_tc());
		mpls_header->set_mpls_ttl(this->mpls(1)->get_mpls_ttl());
	} else {
		mpls_header->set_mpls_bos(true);
		mpls_header->set_mpls_label(0x0000);
		mpls_header->set_mpls_tc(0x00);
		mpls_header->set_mpls_ttl(0x00);
	}

	return mpls_header;
#endif
	return NULL;
#if 0
	rofl::fetherframe* ether_header;
	unsigned int current_length;

	if(!is_classified)
		classify();

	if (pppoe(0)){
		// TODO: log error => pppoe tag already exists
		return NULL;
	}

	//Recover the ether(0)
	ether_header = ether(0);
	current_length = ether_header->framelen();

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
			ether_header->shift_left(bytes_to_insert);

			ether_header->set_dl_type(rofl::fpppoeframe::PPPOE_ETHER_SESSION);

			/*
			 * append the new fpppoeframe instance to ether(0)
			 */
			push_header(HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);
			push_header(HEADER_TYPE_PPP, FIRST_PPP_FRAME_POS, FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES);

			n_pppoe = (rofl::fpppoeframe*)headers[FIRST_PPPOE_FRAME_POS].frame;
			n_ppp = (rofl::fpppframe*)headers[FIRST_PPP_FRAME_POS].frame;

			//Now reset frames
			ether_header->reset(ether_header->soframe(), current_length + bytes_to_insert);
			n_pppoe->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), ether_header->framelen() - sizeof(struct rofl::fetherframe::eth_hdr_t) );
			n_ppp->reset(n_pppoe->soframe() + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t), n_pppoe->framelen() - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));

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
			if (pkt_push(ether_header->payload(), bytes_to_insert) == ROFL_FAILURE){
				// TODO: log error
				return NULL;
			}

			/*
			 * adjust ether(0): move one pppoe tag to the left
			 */
			ether_header->shift_left(bytes_to_insert);

			ether_header->set_dl_type(rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY);

			/*
			 * append the new fpppoeframe instance to ether(0)
			 */
			push_header(HEADER_TYPE_PPPOE, FIRST_PPPOE_FRAME_POS, FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES);

			//Now reset frame
			//Size of ethernet needs to be extended with +PPPOE size
			//PPPOE size needs to be ether_header->size + PPPOE - ether_header
			ether_header->reset(ether_header->soframe(), current_length + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));
			headers[FIRST_PPPOE_FRAME_POS].frame->reset(ether_header->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), current_length + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t) - sizeof(struct rofl::fetherframe::eth_hdr_t));

			n_pppoe = (rofl::fpppoeframe*)headers[FIRST_PPPOE_FRAME_POS].frame;


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
#endif
	return NULL;
}


void static_pktclassifier::dump(){

	ROFL_DEBUG("datapacketx86(%p) soframe: %p framelen: %zu\n", this, pkt->get_buffer(), pkt->get_buffer_length());


	//while (NULL != frame) {
	//	ROFL_ERR("%s\n", frame->c_str());
	//	frame = frame->next;
	//}
	

	rofl::fframe content(pkt->get_buffer(), pkt->get_buffer_length());
	ROFL_DEBUG("content: %s\n", content.c_str());

}


size_t static_pktclassifier::get_pkt_len(rofl::fframe *from, rofl::fframe *to){

	unsigned int total_length = pkt->get_buffer_length();

	if(!from)
		return total_length;

	if(!to)
		return from->framelen();

	return from->framelen() - to->framelen();
}
