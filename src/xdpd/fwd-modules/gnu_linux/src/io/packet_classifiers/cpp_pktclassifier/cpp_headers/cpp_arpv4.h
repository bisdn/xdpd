#ifndef _CPP_ARPV4_H_
#define _CPP_ARPV4_H_

#include <rofl/common/endian_conversion.h>
#include <rofl/common/protocols/farpv4frame.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
uint16_t get_ar_hrd(void *hdr){
	return ((farpv4frame *)hdr)->get_hw_addr_type();
};

inline static
void set_ar_hdr(void *hdr, uint16_t ar_hdr){
	((farpv4frame *)hdr)->set_hw_addr_type(ar_hdr);
};

inline static
uint16_t get_ar_pro(void *hdr){
	return ((farpv4frame *)hdr)->get_prot_addr_type();
};

inline static
void set_ar_pro(void *hdr, uint16_t ar_pro){
	((farpv4frame *)hdr)->set_prot_addr_type(ar_pro);
};

inline static
uint8_t get_ar_hln(void *hdr){
	return ((farpv4frame *)hdr)->get_hw_addr_size();
};

inline static
void set_ar_hln(void *hdr, uint8_t ar_hln){
	((farpv4frame *)hdr)->set_hw_addr_size(ar_hln);
	
};

inline static
uint8_t get_ar_pln(void *hdr){
	return ((farpv4frame *)hdr)->get_prot_hw_addr_size();
};

inline static
void set_ar_pln(void *hdr, uint8_t ar_pln){
	((farpv4frame *)hdr)->set_prot_hw_addr_size(ar_pln);
};

inline static
uint16_t get_ar_op(void *hdr){
	return ((farpv4frame *)hdr)->get_opcode();
};

inline static
void set_ar_op(void *hdr, uint16_t ar_op){
	((farpv4frame *)hdr)->set_opcode(ar_op);
};

inline static
uint64_t get_dl_arpv4_dst(void *hdr){
	return ((farpv4frame *)hdr)->get_dl_dst().get_mac();
};

inline static
void set_dl_arpv4_dst(void* hdr, uint64_t dl_dst){
	((farpv4frame *)hdr)->set_dl_dst(rofl::cmacaddr(dl_dst));
};

inline static
uint64_t get_dl_arpv4_src(void* hdr){
	return ((farpv4frame *)hdr)->get_dl_src().get_mac();
};

inline static
void set_dl_arpv4_src(void* hdr, uint64_t dl_src){
	((farpv4frame *)hdr)->set_dl_src(rofl::cmacaddr(dl_src));
};

inline static
uint32_t get_ip_arpv4_src(void *hdr){
	return be32toh(((farpv4frame *)hdr)->get_nw_src().ca_s4addr->sin_addr.s_addr);
};

inline static
void set_ip_arpv4_src(void *hdr, uint16_t ip_src){
	((farpv4frame *)hdr)->set_nw_src(ip_src);
};

inline static
uint16_t get_ip_arpv4_dst(void *hdr){
	return be32toh(((farpv4frame *)hdr)->get_nw_dst().ca_s4addr->sin_addr.s_addr);
};

inline static
void set_ip_arpv4_dst(void *hdr, uint16_t ip_dst){
	((farpv4frame *)hdr)->set_nw_dst(ip_dst);
};
#endif //_CPP_ARPV4_H_
