#include "cpp_arpv4.h"
#include <rofl/common/endian_conversion.h>
#include <rofl/common/protocols/farpv4frame.h>


uint16_t get_ar_hrd(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_hw_addr_type();
};

void set_ar_hdr(void *hdr, uint16_t ar_hdr){
	((rofl::farpv4frame *)hdr)->set_hw_addr_type(ar_hdr);
};

uint16_t get_ar_pro(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_prot_addr_type();
};

void set_ar_pro(void *hdr, uint16_t ar_pro){
	((rofl::farpv4frame *)hdr)->set_prot_addr_type(ar_pro);
};

uint8_t get_ar_hln(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_hw_addr_size();
};

void set_ar_hln(void *hdr, uint8_t ar_hln){
	((rofl::farpv4frame *)hdr)->set_hw_addr_size(ar_hln);
	
};

uint8_t get_ar_pln(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_prot_hw_addr_size();
};

void set_ar_pln(void *hdr, uint8_t ar_pln){
	((rofl::farpv4frame *)hdr)->set_prot_hw_addr_size(ar_pln);
};

uint16_t get_ar_op(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_opcode();
};

void set_ar_op(void *hdr, uint16_t ar_op){
	((rofl::farpv4frame *)hdr)->set_opcode(ar_op);
};

uint64_t get_dl_arpv4_dst(void *hdr){
	return ((rofl::farpv4frame *)hdr)->get_dl_dst().get_mac();
};

void set_dl_arpv4_dst(void* hdr, uint64_t dl_dst){
	((rofl::farpv4frame *)hdr)->set_dl_dst(rofl::cmacaddr(dl_dst));
};

uint64_t get_dl_arpv4_src(void* hdr){
	return ((rofl::farpv4frame *)hdr)->get_dl_src().get_mac();
};

void set_dl_arpv4_src(void* hdr, uint64_t dl_src){
	((rofl::farpv4frame *)hdr)->set_dl_src(rofl::cmacaddr(dl_src));
};

uint32_t get_ip_arpv4_src(void *hdr){
	return be32toh(((rofl::farpv4frame *)hdr)->get_nw_src().ca_s4addr->sin_addr.s_addr);
};

void set_ip_arpv4_src(void *hdr, uint16_t ip_src){
	((rofl::farpv4frame *)hdr)->set_nw_src(ip_src);
};

uint16_t get_ip_arpv4_dst(void *hdr){
	return be32toh(((rofl::farpv4frame *)hdr)->get_nw_dst().ca_s4addr->sin_addr.s_addr);
};

void set_ip_arpv4_dst(void *hdr, uint16_t ip_dst){
	((rofl::farpv4frame *)hdr)->set_nw_dst(ip_dst);
};
