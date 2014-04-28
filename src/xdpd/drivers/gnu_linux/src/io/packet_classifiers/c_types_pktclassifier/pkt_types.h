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
	PROTO_ETH,
	PROTO_MPLS,
	PROTO_VLAN,
	PROTO_PPPOE,
	PROTO_PPP,
	PROTO_ARPV4,
	PROTO_IPV4,
	PROTO_ICMPV4,
	PROTO_ICMPV6,
	PROTO_ICMPV6_OPTS,
	PROTO_IPV6,
	PROTO_TCP,
	PROTO_UDP,
	PROTO_SCTP,
	PROTO_GTPU,
	
	//Never delete this
	PROTO_MAX__,
}protocol_types_t;

//Protocol offsets by type [PKT_TYPE][HEADER]
const int protocol_offsets_bt[PT_MAX__][PROTO_MAX__] =
{
		{0,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
		{0,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

#endif //PKT_TYPES_H
