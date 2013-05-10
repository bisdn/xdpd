#include "static_pktclassifier.h"
#include <rofl/common/utils/c_logger.h>
#include "../datapacketx86.h"


//Constructor&destructor
static_pktclassifier::static_pktclassifier(datapacketx86* pkt_ref) :
	packetclassifier(pkt_ref),
{
	unsigned int i;

	//Initialize frames
	for(i=0;i<MAX_ETHER_FRAMES;i++){
		headers[FIRST_ETHER_FRAME_POS+i].frame = rofl::fetherframe() ;		
	}
	
	//Add more here

	classify_reset();	
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

rofl::fetherframe* rofl_pktclassifier::ether(unsigned int idx) const{

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

rofl::fvlanframe* rofl_pktclassifier::vlan(unsigned int idx) const
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

rofl::fmplsframe* rofl_pktclassifier::mpls(unsigned int idx) const
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

rofl::farpv4frame* rofl_pktclassifier::arpv4(unsigned int idx) const
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

rofl::fipv4frame* rofl_pktclassifier::ipv4(unsigned int idx) const
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

rofl::ficmpv4frame* rofl_pktclassifier::icmpv4(unsigned int idx) const
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

rofl::fudpframe* rofl_pktclassifier::udp(unsigned int idx) const
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

rofl::ftcpframe* rofl_pktclassifier::tcp(unsigned int idx) const
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

rofl::fsctpframe* rofl_pktclassifier::sctp(unsigned int idx) const
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

rofl::fpppoeframe* rofl_pktclassifier::pppoe(unsigned int idx) const
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

rofl::fpppframe* rofl_pktclassifier::ppp(unsigned int idx) const
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
