#include "cpp_mpls.h"
#include <rofl/common/protocols/fmplsframe.h>

void set_mpls_label(void *hdr, uint32_t label){
	((rofl::fmplsframe*)hdr)->set_mpls_label(label);
};

uint32_t get_mpls_label(void *hdr){
	return ((rofl::fmplsframe*)hdr)->get_mpls_label();
};

void set_mpls_tc(void *hdr, uint8_t tc){
	((rofl::fmplsframe*)hdr)->set_mpls_tc(tc);
}

uint8_t get_mpls_tc(void *hdr){
	return ((rofl::fmplsframe*)hdr)->get_mpls_tc();
}

void dec_mpls_ttl(void *hdr){
	((rofl::fmplsframe*)hdr)->dec_mpls_ttl();
}

void set_mpls_ttl(void *hdr, uint8_t ttl){
	((rofl::fmplsframe*)hdr)->set_mpls_ttl(ttl);
}

uint8_t get_mpls_ttl(void *hdr){
	return ((rofl::fmplsframe*)hdr)->get_mpls_ttl();
}

void set_mpls_bos(void *hdr, bool flag){
	((rofl::fmplsframe*)hdr)->set_mpls_bos();
}

bool get_mpls_bos(void *hdr){
	return ((rofl::fmplsframe*)hdr)->get_mpls_bos();
}
