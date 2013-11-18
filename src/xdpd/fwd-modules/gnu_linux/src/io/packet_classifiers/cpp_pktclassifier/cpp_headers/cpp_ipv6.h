#ifndef _CPP_IPV6_H_
#define _CPP_IPV6_H_

#include <rofl/common/protocols/fipv6frame.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
void ipv6_calc_checksum(void *hdr, uint16_t length){
	((fipv6frame*)hdr)->ipv6_calc_checksum();
};


inline static
void set_ipv6_version(void *hdr, uint8_t version){
	((fipv6frame*)hdr)->set_version(version);
};

inline static
uint8_t get_ipv6_version(void *hdr){
	return ((fipv6frame*)hdr)->get_version();
};

inline static
void set_ipv6_traffic_class(void *hdr, uint8_t tc){
	((fipv6frame*)hdr)->set_traffic_class(tc);
}

inline static
uint8_t get_ipv6_traffic_class(void *hdr){
	return ((fipv6frame*)hdr)->get_traffic_class();
};

inline static
void set_ipv6_dscp(void *hdr, uint8_t dscp){
	((fipv6frame*)hdr)->set_dscp(dscp);
}

inline static
uint8_t get_ipv6_dscp(void *hdr){
	return ((fipv6frame*)hdr)->get_dscp();
};

inline static
void set_ipv6_ecn(void *hdr, uint8_t ecn){
	((fipv6frame*)hdr)->set_ecn(ecn);
}

inline static
uint8_t get_ipv6_ecn(void *hdr){
	return ((fipv6frame*)hdr)->get_ecn();
};

inline static
void set_flow_label(void *hdr, uint32_t flabel){
	((fipv6frame*)hdr)->set_flow_label(flabel);
}

inline static
uint32_t get_flow_label(void *hdr){
	return ((fipv6frame*)hdr)->get_flow_label();
};

inline static
void set_payload_length(void *hdr, uint16_t len){
	((fipv6frame*)hdr)->set_payload_length(len);
}

inline static
uint16_t get_payload_length(void *hdr){
	return ((fipv6frame*)hdr)->get_payload_length();
};

inline static
void set_next_header(void *hdr, uint8_t nxthdr){
	((fipv6frame*)hdr)->set_next_header(nxthdr);
}

inline static
uint8_t get_next_header(void *hdr){
	return ((fipv6frame*)hdr)->get_next_header();
};

inline static
void set_hop_limit(void *hdr, uint8_t hops){
	((fipv6frame*)hdr)->set_hop_limit(hops);
}

inline static
uint8_t get_hop_limit(void *hdr){
	return ((fipv6frame*)hdr)->get_hop_limit();
};

inline static
void dec_hop_limit(void *hdr){
	((fipv6frame*)hdr)->dec_hop_limit();
};

inline static
void set_ipv6_src(void *hdr, uint128__t src){
	((fipv6frame*)hdr)->set_ipv6_src((uint8_t*)&src.val,16);
};

inline static
uint128__t get_ipv6_src(void *hdr){
	return ((fipv6frame*)hdr)->get_ipv6_src().get_ipv6_addr();
};

inline static
void set_ipv6_dst(void *hdr, uint128__t dst){
	((fipv6frame*)hdr)->set_ipv6_dst((uint8_t*)&dst.val,16);
};

inline static
uint128__t get_ipv6_dst(void *hdr){
	return ((fipv6frame*)hdr)->get_ipv6_dst().get_ipv6_addr();
};


#endif //_CPP_IPV6_H_