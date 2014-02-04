#include "cpp_icmpv6.h"
#include <rofl/common/protocols/ficmpv6frame.h>

void icmpv6_calc_checksum(void *hdr, uint16_t length){
	((rofl::ficmpv6frame *)hdr)->icmpv6_calc_checksum();
};

void *get_icmpv6_option(void *hdr, uint8_t type){
	return (void*) &((rofl::ficmpv6frame *)hdr)->get_option((rofl::ficmpv6opt::icmpv6_option_type_t)type);
};

uint8_t get_icmpv6_code(void *hdr){
	return ((rofl::ficmpv6frame *)hdr)->get_icmpv6_code();
};

void set_icmpv6_code(void *hdr, uint8_t code){
	((rofl::ficmpv6frame *)hdr)->set_icmpv6_code(code);
};

uint8_t get_icmpv6_type(void *hdr){
	return ((rofl::ficmpv6frame *)hdr)->get_icmpv6_type();
};

void set_icmpv6_type(void *hdr, uint8_t type){
	((rofl::ficmpv6frame *)hdr)->set_icmpv6_type(type);
};

uint128__t get_icmpv6_neighbor_taddr(void *hdr){
	return ((rofl::ficmpv6frame *)hdr)->get_icmpv6_neighbor_taddr().get_ipv6_addr();
};

void set_icmpv6_neighbor_taddr(void *hdr, uint128__t taddr){
	rofl::caddress addr(AF_INET6,"0:0:0:0:0");
	addr.set_ipv6_addr(taddr);
	((rofl::ficmpv6frame *)hdr)->set_icmpv6_neighbor_taddr(addr);
};

//ndp_rtr_flag
//ndp_solicited_flag
//ndp_override_flag
//neighbor_taddr

uint8_t get_icmpv6_opt_type(void *hdr){
	return ((rofl::ficmpv6opt *)hdr)->get_opt_type();
};

void set_icmpv6_opt_type(void *hdr, uint8_t type){
	((rofl::ficmpv6opt *)hdr)->set_opt_type(type);
};

uint64_t get_icmpv6_ll_taddr(void *hdr){
	return ((rofl::ficmpv6frame *)hdr)->get_option(rofl::ficmpv6opt::ICMPV6_OPT_LLADDR_TARGET).get_ll_taddr().get_mac();
};

void set_icmpv6_ll_taddr(void *hdr, uint64_t taddr){
	((rofl::ficmpv6frame *)hdr)->get_option(rofl::ficmpv6opt::ICMPV6_OPT_LLADDR_TARGET).set_ll_taddr(rofl::cmacaddr(taddr));
};

uint64_t get_icmpv6_ll_saddr(void *hdr){
	return ((rofl::ficmpv6frame *)hdr)->get_option(rofl::ficmpv6opt::ICMPV6_OPT_LLADDR_SOURCE).get_ll_saddr().get_mac();
};

void set_icmpv6_ll_saddr(void *hdr, uint64_t saddr){
	((rofl::ficmpv6frame *)hdr)->get_option(rofl::ficmpv6opt::ICMPV6_OPT_LLADDR_SOURCE).set_ll_saddr(rofl::cmacaddr(saddr));
};

uint8_t get_icmpv6_pfx_on_link_flag(void *hdr){
	return ((rofl::ficmpv6frame *)hdr)->get_option(rofl::ficmpv6opt::ICMPV6_OPT_PREFIX_INFO).get_pfx_on_link_flag();
};

void set_icmpv6_pfx_on_link_flag(void *hdr, uint8_t flag){
	((rofl::ficmpv6frame *)hdr)->get_option(rofl::ficmpv6opt::ICMPV6_OPT_PREFIX_INFO).set_pfx_on_link_flag(flag);
};

uint8_t get_icmpv6_pfx_aac_flag(void *hdr){
	return ((rofl::ficmpv6frame *)hdr)->get_option(rofl::ficmpv6opt::ICMPV6_OPT_PREFIX_INFO).get_pfx_aac_flag();
};

void set_icmpv6_pfx_aac_flag(void *hdr, uint8_t flag){
	((rofl::ficmpv6frame *)hdr)->get_option(rofl::ficmpv6opt::ICMPV6_OPT_PREFIX_INFO).set_pfx_aac_flag(flag);
};
