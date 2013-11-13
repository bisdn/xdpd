#ifndef _C_PKTCLASSIFIER_H_
#define _C_PKTCLASSIFIER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#include "./headers/cpc_arpv4.h"
#include "./headers/cpc_ethernet.h"
#include "./headers/cpc_gtpu.h"
#include "./headers/cpc_icmpv4.h"
#include "./headers/cpc_icmpv6.h"
#include "./headers/cpc_ipv4.h"
#include "./headers/cpc_ipv6.h"
#include "./headers/cpc_mpls.h"
#include "./headers/cpc_ppp.h"
#include "./headers/cpc_pppoe.h"
#include "./headers/cpc_tcp.h"
#include "./headers/cpc_udp.h"
#include "./headers/cpc_vlan.h"

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


// Constants
//Maximum header occurrences per type
#define MAX_ETHER_FRAMES 2
#define MAX_VLAN_FRAMES 4
#define MAX_MPLS_FRAMES 16
#define MAX_ARPV4_FRAMES 1
#define MAX_IPV4_FRAMES 2
#define MAX_ICMPV4_FRAMES 2
#define MAX_IPV6_FRAMES 2
#define MAX_ICMPV6_FRAMES 2
#define MAX_UDP_FRAMES 2
#define MAX_TCP_FRAMES 2
#define MAX_SCTP_FRAMES 2
#define MAX_PPPOE_FRAMES 1
#define MAX_PPP_FRAMES 1
#define MAX_GTP_FRAMES 1

//Total maximum header occurrences
#define MAX_HEADERS MAX_ETHER_FRAMES + \
						MAX_VLAN_FRAMES + \
						MAX_MPLS_FRAMES + \
						MAX_ARPV4_FRAMES + \
						MAX_IPV4_FRAMES + \
						MAX_ICMPV4_FRAMES + \
						MAX_IPV6_FRAMES + \
						MAX_ICMPV6_FRAMES + \
						MAX_UDP_FRAMES + \
						MAX_TCP_FRAMES + \
						MAX_SCTP_FRAMES + \
						MAX_PPPOE_FRAMES + \
						MAX_PPP_FRAMES + \
						MAX_GTP_FRAMES


//Relative positions within the array;
//Very first frame always
#define FIRST_ETHER_FRAME_POS 0
#define FIRST_VLAN_FRAME_POS FIRST_ETHER_FRAME_POS+MAX_ETHER_FRAMES
#define FIRST_MPLS_FRAME_POS FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES
#define FIRST_ARPV4_FRAME_POS FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES
#define FIRST_IPV4_FRAME_POS FIRST_ARPV4_FRAME_POS+MAX_ARPV4_FRAMES
#define FIRST_ICMPV4_FRAME_POS FIRST_IPV4_FRAME_POS+MAX_IPV4_FRAMES
#define FIRST_IPV6_FRAME_POS FIRST_ICMPV4_FRAME_POS+MAX_ICMPV4_FRAMES
#define FIRST_ICMPV6_FRAME_POS FIRST_IPV6_FRAME_POS+MAX_IPV6_FRAMES
#define FIRST_UDP_FRAME_POS FIRST_ICMPV6_FRAME_POS+MAX_ICMPV6_FRAMES
#define FIRST_TCP_FRAME_POS FIRST_UDP_FRAME_POS+MAX_UDP_FRAMES
#define FIRST_SCTP_FRAME_POS FIRST_TCP_FRAME_POS+MAX_TCP_FRAMES
#define FIRST_PPPOE_FRAME_POS FIRST_SCTP_FRAME_POS+MAX_SCTP_FRAMES
#define FIRST_PPP_FRAME_POS FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES
#define FIRST_GTP_FRAME_POS FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES

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
	//Real container
	header_container_t headers[MAX_HEADERS];
	
	//Counters
	unsigned int num_of_headers[HEADER_TYPE_MAX];
	
	//Flag to know if it is classified
	bool is_classified;

	//Inner most (last) ethertype
	uint16_t eth_type;

}classify_state_t;


//function declarations
void classify(classify_state_t* clas_state, uint8_t* pkt, size_t len);

cpc_eth_hdr_t* ether(classify_state_t* clas_state, int idx);
cpc_vlan_hdr_t* vlan(classify_state_t* clas_state, int idx);
cpc_mpls_hdr_t* mpls(classify_state_t* clas_state, int idx);
cpc_arpv4_hdr_t* arpv4(classify_state_t* clas_state, int idx);
cpc_ipv4_hdr_t* ipv4(classify_state_t* clas_state, int idx);
cpc_icmpv4_hdr_t* icmpv4(classify_state_t* clas_state, int idx);
cpc_ipv6_hdr_t* ipv6(classify_state_t* clas_state, int idx);
cpc_icmpv6_hdr_t* icmpv6(classify_state_t* clas_state, int idx);
cpc_udp_hdr_t* udp(classify_state_t* clas_state, int idx);
cpc_tcp_hdr_t* tcp(classify_state_t* clas_state, int idx);
cpc_pppoe_hdr_t* pppoe(classify_state_t* clas_state, int idx);
cpc_ppp_hdr_t* ppp(classify_state_t* clas_state, int idx);
cpc_gtphu_t* gtp(classify_state_t* clas_state, int idx);

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






#endif //_C_PKTCLASSIFIER_H_
