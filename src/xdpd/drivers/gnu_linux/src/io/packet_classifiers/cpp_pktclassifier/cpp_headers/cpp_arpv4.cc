#include "cpp_arpv4.h"
#include <rofl/common/endian_conversion.h>
#include <rofl/common/protocols/farpv4frame.h>


uint16_t get_arpv4_htype(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_hw_addr_type();
};

void set_arpv4_hdr(void *hdr, uint16_t htype){
	((rofl::farpv4frame *)hdr)->set_hw_addr_type(htype);
};

uint16_t get_arpv4_ptype(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_prot_addr_type();
};

void set_arpv4_ptype(void *hdr, uint16_t ptype){
	((rofl::farpv4frame *)hdr)->set_prot_addr_type(ptype);
};

uint8_t get_arpv4_hlen(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_hw_addr_size();
};

void set_arpv4_hlen(void *hdr, uint8_t hlen){
	((rofl::farpv4frame *)hdr)->set_hw_addr_size(hlen);
	
};

uint8_t get_arpv4_plen(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_prot_hw_addr_size();
};

void set_arpv4_plen(void *hdr, uint8_t plen){
	((rofl::farpv4frame *)hdr)->set_prot_hw_addr_size(plen);
};

uint16_t get_arpv4_opcode(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_opcode();
};

void set_arpv4_opcode(void *hdr, uint16_t opcode){
	((rofl::farpv4frame *)hdr)->set_opcode(opcode);
};

uint64_t get_arpv4_dl_dst(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_dl_dst().get_mac();
};

void set_arpv4_dl_dst(void* hdr, uint64_t dl_dst){
	((rofl::farpv4frame *)hdr)->set_dl_dst(rofl::cmacaddr(dl_dst));
};

uint64_t get_arpv4_dl_src(void* hdr){
	return ((rofl::farpv4frame *)hdr)->get_dl_src().get_mac();
};

void set_arpv4_dl_src(void* hdr, uint64_t dl_src){
	((rofl::farpv4frame *)hdr)->set_dl_src(rofl::cmacaddr(dl_src));
};

uint32_t get_arpv4_ip_src(void *hdr){
	return be32toh(((rofl::farpv4frame *)hdr)->get_nw_src().ca_s4addr->sin_addr.s_addr);
};

void set_arpv4_ip_src(void *hdr, uint16_t ip_src){
	((rofl::farpv4frame *)hdr)->set_nw_src(ip_src);
};

uint16_t get_arpv4_ip_dst(void *hdr){
	return be32toh(((rofl::farpv4frame *)hdr)->get_nw_dst().ca_s4addr->sin_addr.s_addr);
};

void set_arpv4_ip_dst(void *hdr, uint16_t ip_dst){
	((rofl::farpv4frame *)hdr)->set_nw_dst(ip_dst);
};
