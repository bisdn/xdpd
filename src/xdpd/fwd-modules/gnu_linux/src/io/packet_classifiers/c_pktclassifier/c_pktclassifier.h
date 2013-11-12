#ifndef _C_PKTCLASSIFIER_H_
#define _C_PKTCLASSIFIER_H_

#include <stdbool.h>

//Header type
enum header_type{
	HEADER_TYPE_ETHER = 0,	
	HEADER_TYPE_VLAN = 1,	
	HEADER_TYPE_MPLS = 2,	
	HEADER_TYPE_ARPV4 = 3,	
	HEADER_TYPE_IPV4 = 4,	
	HEADER_TYPE_ICMPV4 = 5,
	HEADER_TYPE_IPV6 = 6,	
	HEADER_TYPE_ICMPV6 = 7,	
	HEADER_TYPE_UDP = 8,	
	HEADER_TYPE_TCP = 9,	
	HEADER_TYPE_SCTP = 10,	
	HEADER_TYPE_PPPOE = 11,	
	HEADER_TYPE_PPP = 12,	
	HEADER_TYPE_GTP = 13,

	//Must be the last one
	HEADER_TYPE_MAX
};

//Maximum header occurrences per type
const unsigned int MAX_ETHER_FRAMES;
const unsigned int MAX_VLAN_FRAMES;
const unsigned int MAX_MPLS_FRAMES;
const unsigned int MAX_ARPV4_FRAMES;
const unsigned int MAX_IPV4_FRAMES;
const unsigned int MAX_ICMPV4_FRAMES;
const unsigned int MAX_IPV6_FRAMES;
const unsigned int MAX_ICMPV6_FRAMES;
const unsigned int MAX_UDP_FRAMES;
const unsigned int MAX_TCP_FRAMES;
const unsigned int MAX_SCTP_FRAMES;
const unsigned int MAX_PPPOE_FRAMES;
const unsigned int MAX_PPP_FRAMES;
const unsigned int MAX_GTP_FRAMES;

//Total maximum header occurrences
const unsigned int MAX_HEADERS;

//Relative positions within the array;
const unsigned int FIRST_ETHER_FRAME_POS; //Very first frame always
const unsigned int FIRST_VLAN_FRAME_POS;
const unsigned int FIRST_MPLS_FRAME_POS;
const unsigned int FIRST_ARPV4_FRAME_POS;
const unsigned int FIRST_IPV4_FRAME_POS;
const unsigned int FIRST_ICMPV4_FRAME_POS;
const unsigned int FIRST_IPV6_FRAME_POS;
const unsigned int FIRST_ICMPV6_FRAME_POS;
const unsigned int FIRST_UDP_FRAME_POS;
const unsigned int FIRST_TCP_FRAME_POS;
const unsigned int FIRST_SCTP_FRAME_POS;
const unsigned int FIRST_PPPOE_FRAME_POS;
const unsigned int FIRST_PPP_FRAME_POS;
const unsigned int FIRST_GTP_FRAME_POS;

//Just to be on the safe side of life
//assert( (FIRST_PPP_FRAME_POS + MAX_PPP_FRAMES) == MAX_HEADERS);

//Header container
typedef struct header_container{

	//Presence of header
	bool present;
	
	//Header pointer
	void* frame;
	enum header_type type; 

	//Pseudo-linked list pointers (short-cuts)
	struct header_container* prev;
	struct header_container* next;
}header_container_t;


#endif //_C_PKTCLASSIFIER_H_
