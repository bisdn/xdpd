#ifndef _CPP_ICMPV6_H_
#define _CPP_ICMPV6_H_

#include <rofl/common/protocols/ficmpv6frame.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
void icmpv6_calc_checksum(void *hdr, uint16_t length){
	((ficmpv6frame *)hdr)->icmpv6_calc_checksum();
};

inline static
uint8_t icmpv6_get_option(void *hdr, uint8_t type){
	//((ficmpv6frame *)hdr)->get_option(type);
	return 0;
};

inline static
uint8_t get_icmpv6_code(void *hdr){
	return ((ficmpv6frame *)hdr)->get_icmpv6_code();
};

inline static
void set_icmpv6_code(void *hdr, uint8_t code){
	((ficmpv6frame *)hdr)->set_icmpv6_code(code);
};

inline static
uint8_t get_icmpv6_type(void *hdr){
	return ((ficmpv6frame *)hdr)->get_icmpv6_type();
};

inline static
void set_icmpv6_type(void *hdr, uint8_t type){
	((ficmpv6frame *)hdr)->set_icmpv6_type(type);
};

inline static
uint128__t get_icmpv6_neighbor_taddr(void *hdr){
	return ((ficmpv6frame *)hdr)->get_icmpv6_neighbor_taddr().get_ipv6_addr();
};

inline static
void set_icmpv6_neighbor_taddr(void *hdr, uint128__t taddr){
	caddress addr(AF_INET6,"0:0:0:0:0");
	addr.set_ipv6_addr(taddr);
	((ficmpv6frame *)hdr)->set_icmpv6_neighbor_taddr(addr);
};

//ndp_rtr_flag
//ndp_solicited_flag
//ndp_override_flag
//neighbor_taddr

inline static
uint8_t get_icmpv6_opt_type(void *hdr){
	return ((ficmpv6opt *)hdr)->get_opt_type();
};

inline static
void set_icmpv6_opt_type(void *hdr, uint8_t type){
	((ficmpv6opt *)hdr)->set_opt_type(type);
};

inline static
uint64_t get_ll_taddr(void *hdr){
	return ((ficmpv6opt *)hdr)->get_ll_taddr().get_mac();
};

inline static
void set_ll_taddr(void *hdr, uint64_t taddr){
	((ficmpv6opt *)hdr)->set_ll_taddr(rofl::cmacaddr(taddr));
};

inline static
uint64_t get_ll_saddr(void *hdr){
	return ((ficmpv6opt *)hdr)->get_ll_saddr().get_mac();
};

inline static
void set_ll_saddr(void *hdr, uint64_t saddr){
	((ficmpv6opt *)hdr)->set_ll_saddr(rofl::cmacaddr(saddr));
};

inline static
uint8_t get_pfx_on_link_flag(void *hdr){
	return ((ficmpv6opt *)hdr)->get_pfx_on_link_flag();
};

inline static
void set_pfx_on_link_flag(void *hdr, uint8_t flag){
	((ficmpv6opt *)hdr)->set_pfx_on_link_flag(flag);
};

inline static
uint8_t get_pfx_aac_flag(void *hdr){
	return ((ficmpv6opt *)hdr)->get_pfx_aac_flag();
};

inline static
void set_pfx_aac_flag(void *hdr, uint8_t flag){
	((ficmpv6opt *)hdr)->set_pfx_aac_flag(flag);
};

#endif //_CPP_ICMPV6_H_
