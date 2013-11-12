#ifndef _C_PKTCLASSIFIER_H_
#define _C_PKTCLASSIFIER_H_


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
static const unsigned int MAX_ETHER_FRAMES = 2;
static const unsigned int MAX_VLAN_FRAMES = 4;
static const unsigned int MAX_MPLS_FRAMES = 16;
static const unsigned int MAX_ARPV4_FRAMES = 1;
static const unsigned int MAX_IPV4_FRAMES = 2;
static const unsigned int MAX_ICMPV4_FRAMES = 2;
static const unsigned int MAX_IPV6_FRAMES = 2;
static const unsigned int MAX_ICMPV6_FRAMES = 2;
static const unsigned int MAX_UDP_FRAMES = 2;
static const unsigned int MAX_TCP_FRAMES = 2;
static const unsigned int MAX_SCTP_FRAMES = 2;
static const unsigned int MAX_PPPOE_FRAMES = 1;
static const unsigned int MAX_PPP_FRAMES = 1;
static const unsigned int MAX_GTP_FRAMES = 1;

//Total maximum header occurrences
static const unsigned int MAX_HEADERS = MAX_ETHER_FRAMES +
						MAX_VLAN_FRAMES +
						MAX_MPLS_FRAMES +
						MAX_ARPV4_FRAMES +
						MAX_IPV4_FRAMES +
						MAX_ICMPV4_FRAMES +
						MAX_IPV6_FRAMES +
						MAX_ICMPV6_FRAMES +
						MAX_UDP_FRAMES +
						MAX_TCP_FRAMES +
						MAX_SCTP_FRAMES +
						MAX_PPPOE_FRAMES + 
						MAX_PPP_FRAMES +
						MAX_GTP_FRAMES;


//Relative positions within the array;
static const unsigned int FIRST_ETHER_FRAME_POS = 0; //Very first frame always
static const unsigned int FIRST_VLAN_FRAME_POS = FIRST_ETHER_FRAME_POS+MAX_ETHER_FRAMES;
static const unsigned int FIRST_MPLS_FRAME_POS = FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES;
static const unsigned int FIRST_ARPV4_FRAME_POS = FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES;
static const unsigned int FIRST_IPV4_FRAME_POS = FIRST_ARPV4_FRAME_POS+MAX_ARPV4_FRAMES;
static const unsigned int FIRST_ICMPV4_FRAME_POS = FIRST_IPV4_FRAME_POS+MAX_IPV4_FRAMES;
static const unsigned int FIRST_IPV6_FRAME_POS = FIRST_ICMPV4_FRAME_POS+MAX_ICMPV4_FRAMES;
static const unsigned int FIRST_ICMPV6_FRAME_POS = FIRST_IPV6_FRAME_POS+MAX_IPV6_FRAMES;
static const unsigned int FIRST_UDP_FRAME_POS = FIRST_ICMPV6_FRAME_POS+MAX_ICMPV6_FRAMES;
static const unsigned int FIRST_TCP_FRAME_POS = FIRST_UDP_FRAME_POS+MAX_UDP_FRAMES;
static const unsigned int FIRST_SCTP_FRAME_POS = FIRST_TCP_FRAME_POS+MAX_TCP_FRAMES;
static const unsigned int FIRST_PPPOE_FRAME_POS = FIRST_SCTP_FRAME_POS+MAX_SCTP_FRAMES;
static const unsigned int FIRST_PPP_FRAME_POS = FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES;
static const unsigned int FIRST_GTP_FRAME_POS = FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES;

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



typedef struct classify_state{
	///?
	//Real container
	header_container_t headers[MAX_HEADERS];
	
	//Counters
	unsigned int num_of_headers[HEADER_TYPE_MAX];
	
	//Flag to know if it is classified
	bool is_classified;

	//Inner most (last) ethertype
	uint16_t eth_type;

}classify_state_t;



#endif //_C_PKTCLASSIFIER_H_
