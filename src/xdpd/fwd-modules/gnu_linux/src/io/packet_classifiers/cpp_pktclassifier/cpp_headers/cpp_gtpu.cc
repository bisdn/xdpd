#include "cpp_gtpu.h"
#include <rofl/common/protocols/fgtpuframe.h>

uint8_t get_gtp_version(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_version();
};

void set_gtp_version(void *hdr, uint8_t version){
	((rofl::fgtpuframe *)hdr)->set_version(version);
};

bool get_pt_flag(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_pt_flag(); 
};

void set_pt_flag(void *hdr, bool pt){
	((rofl::fgtpuframe *)hdr)->set_pt_flag(pt);
};

bool get_e_flag(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_e_flag(); 
};

void set_e_flag(void *hdr, bool e){
	((rofl::fgtpuframe *)hdr)->set_e_flag(e);
};

bool get_s_flag(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_s_flag();
};

void set_s_flag(void *hdr, bool s){
	((rofl::fgtpuframe *)hdr)->set_s_flag(s);
};

bool get_pn_flag(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_pn_flag();
};

void set_pn_flag(void *hdr, bool pn){
	((rofl::fgtpuframe *)hdr)->set_pn_flag(pn);
};

uint8_t get_msg_type(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_msg_type();
};

void set_msg_type(void *hdr, uint8_t msgtype){
	((rofl::fgtpuframe *)hdr)->set_msg_type(msgtype);
};

uint16_t get_gtp_length(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_length();
};

void set_gtp_length(void *hdr, uint16_t length){
	((rofl::fgtpuframe *)hdr)->set_length(length);
};

uint32_t get_teid(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_teid();
};

void set_teid(void *hdr, uint32_t teid){
	((rofl::fgtpuframe *)hdr)->set_teid(teid);
};

uint16_t get_seq_no(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_seq_no();
};

void set_seq_no(void *hdr, uint16_t seqno){
	((rofl::fgtpuframe *)hdr)->set_seq_no(seqno);
};

uint8_t get_npdu_no(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_npdu_no();
};

void set_npdu_no(void *hdr, uint8_t npdu){
	((rofl::fgtpuframe *)hdr)->set_npdu_no(npdu);
};

uint8_t get_ext_type(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_ext_type();
};

void set_ext_type(void *hdr, uint8_t exthdr){
	((rofl::fgtpuframe *)hdr)->set_ext_type(exthdr);
};

