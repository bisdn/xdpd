#ifndef PKT_TYPES_H
#define PKT_TYPES_H

//Auto-generated structures

/**
* Packet types
*/
typedef enum pkt_types{

	//Plain ethernet frame
	PT_ETH,

	//VLAN
	//TODO
	
	//MPLS
	//TODO

	//PPPoE PPP
	//TODO

	//IPv4
	PT_ETH_ARPV4,
	PT_ETH_ICMPV4,
	PT_ETH_IPV4,
	
	//IPv4+TP
	PT_ETH_IPV4_TCP,
	PT_ETH_IPV4_UDP,
	PT_ETH_IPV4_SCTP,

	//IPv6
	//TODO
	
	PT_MAX__
}pkt_types_t;

//Protocol types
typedef enum protocol_types{
	PT_PROTO_ETH,
	PT_PROTO_MPLS,
	PT_PROTO_VLAN,
	PT_PROTO_PPPOE,
	PT_PROTO_PPP,
	PT_PROTO_ARPV4,
	PT_PROTO_IPV4,
	PT_PROTO_ICMPV4,
	PT_PROTO_ICMPV6,
	PT_PROTO_ICMPV6_OPTS,
	PT_PROTO_ICMPV6_OPTS_LLADR_SRC,
	PT_PROTO_ICMPV6_OPTS_LLADR_TGT,
	PT_PROTO_ICMPV6_OPTS_PREFIX_INFO,
	PT_PROTO_IPV6,
	PT_PROTO_TCP,
	PT_PROTO_UDP,
	PT_PROTO_SCTP,
	PT_PROTO_GTPU,
	
	//Never delete this
	PT_PROTO_MAX__,
}protocol_types_t;

//Protocol offsets by type [PKT_TYPE][HEADER]
const int protocol_offsets_bt[PT_MAX__][PT_PROTO_MAX__] =
{
		{0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

#define PKT_TYPES_GET_HDR(tmp, state, proto)\
	do{\
		tmp = state->base + protocol_offsets_bt[ state->type ][ proto ];\
		if(tmp <= state->base )\
			tmp = NULL;\
	}while(0)


#endif //PKT_TYPES_H
