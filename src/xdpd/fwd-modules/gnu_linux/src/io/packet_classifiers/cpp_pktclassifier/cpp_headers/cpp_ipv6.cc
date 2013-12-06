#include "cpp_ipv6.h"
#include <rofl/common/protocols/fipv6frame.h>

void ipv6_calc_checksum(void *hdr, uint16_t length){
	((rofl::fipv6frame*)hdr)->ipv6_calc_checksum();
};

void set_ipv6_version(void *hdr, uint8_t version){
	((rofl::fipv6frame*)hdr)->set_version(version);
};

uint8_t get_ipv6_version(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_version();
};

void set_ipv6_traffic_class(void *hdr, uint8_t tc){
	((rofl::fipv6frame*)hdr)->set_traffic_class(tc);
}

uint8_t get_ipv6_traffic_class(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_traffic_class();
};

void set_ipv6_dscp(void *hdr, uint8_t dscp){
	((rofl::fipv6frame*)hdr)->set_dscp(dscp);
}

uint8_t get_ipv6_dscp(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_dscp();
};

void set_ipv6_ecn(void *hdr, uint8_t ecn){
	((rofl::fipv6frame*)hdr)->set_ecn(ecn);
}

uint8_t get_ipv6_ecn(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_ecn();
};

void set_ipv6_flow_label(void *hdr, uint32_t flabel){
	((rofl::fipv6frame*)hdr)->set_flow_label(flabel);
}

uint32_t get_ipv6_flow_label(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_flow_label();
};

void set_ipv6_payload_length(void *hdr, uint16_t len){
	((rofl::fipv6frame*)hdr)->set_payload_length(len);
}

uint16_t get_ipv6_payload_length(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_payload_length();
};

void set_ipv6_next_header(void *hdr, uint8_t nxthdr){
	((rofl::fipv6frame*)hdr)->set_next_header(nxthdr);
}

uint8_t get_ipv6_next_header(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_next_header();
};

void set_ipv6_hop_limit(void *hdr, uint8_t hops){
	((rofl::fipv6frame*)hdr)->set_hop_limit(hops);
}

uint8_t get_ipv6_hop_limit(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_hop_limit();
};

void dec_ipv6_hop_limit(void *hdr){
	((rofl::fipv6frame*)hdr)->dec_hop_limit();
};

void set_ipv6_src(void *hdr, uint128__t src){
	((rofl::fipv6frame*)hdr)->set_ipv6_src((uint8_t*)&src.val,16);
};

uint128__t get_ipv6_src(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_ipv6_src().get_ipv6_addr();
};

void set_ipv6_dst(void *hdr, uint128__t dst){
	((rofl::fipv6frame*)hdr)->set_ipv6_dst((uint8_t*)&dst.val,16);
};

uint128__t get_ipv6_dst(void *hdr){
	return ((rofl::fipv6frame*)hdr)->get_ipv6_dst().get_ipv6_addr();
};
