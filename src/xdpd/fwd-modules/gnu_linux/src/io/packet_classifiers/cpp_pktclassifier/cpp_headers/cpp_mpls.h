#ifndef _CPP_MPLS_H_
#define _CPP_MPLS_H_

#include <rofl/common/protocols/fmplsframe.h>

using namespace rofl;
using namespace xdpd::gnu_linux;


inline static
void set_mpls_label(void *hdr, uint32_t label){
	((fmplsframe*)hdr)->set_mpls_label(label);
};

inline static
uint32_t get_mpls_label(void *hdr){
	return ((fmplsframe*)hdr)->get_mpls_label();
};

inline static
void set_mpls_tc(void *hdr, uint8_t tc){
	((fmplsframe*)hdr)->set_mpls_tc(tc);
}

inline static
uint8_t get_mpls_tc(void *hdr){
	return ((fmplsframe*)hdr)->get_mpls_tc();
}

inline static
void dec_mpls_ttl(void *hdr){
	((fmplsframe*)hdr)->dec_mpls_ttl();
}

inline static
void set_mpls_ttl(void *hdr, uint8_t ttl){
	((fmplsframe*)hdr)->set_mpls_ttl(ttl);
}

inline static
uint8_t get_mpls_ttl(void *hdr){
	return ((fmplsframe*)hdr)->get_mpls_ttl();
}

inline static
void set_mpls_bos(void *hdr, bool flag){
	((fmplsframe*)hdr)->set_mpls_bos();
}

inline static
bool get_mpls_bos(void *hdr){
	return ((fmplsframe*)hdr)->get_mpls_bos();
}

#endif //_CPP_MPLS_H_