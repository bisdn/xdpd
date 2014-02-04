#include "cpp_gtpu.h"
#include <rofl/common/protocols/fgtpuframe.h>

uint8_t get_gtpu_version(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_version();
};

void set_gtpu_version(void *hdr, uint8_t version){
	((rofl::fgtpuframe *)hdr)->set_version(version);
};

bool get_gtpu_pt_flag(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_pt_flag(); 
};

void set_gtpu_pt_flag(void *hdr, bool pt){
	((rofl::fgtpuframe *)hdr)->set_pt_flag(pt);
};

bool get_gtpu_e_flag(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_e_flag(); 
};

void set_gtpu_e_flag(void *hdr, bool e){
	((rofl::fgtpuframe *)hdr)->set_e_flag(e);
};

bool get_gtpu_s_flag(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_s_flag();
};

void set_gtpu_s_flag(void *hdr, bool s){
	((rofl::fgtpuframe *)hdr)->set_s_flag(s);
};

bool get_gtpu_pn_flag(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_pn_flag();
};

void set_gtpu_pn_flag(void *hdr, bool pn){
	((rofl::fgtpuframe *)hdr)->set_pn_flag(pn);
};

uint8_t get_gtpu_msg_type(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_msg_type();
};

void set_gtpu_msg_type(void *hdr, uint8_t msgtype){
	((rofl::fgtpuframe *)hdr)->set_msg_type(msgtype);
};

uint16_t get_gtpu_length(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_length();
};

void set_gtpu_length(void *hdr, uint16_t length){
	((rofl::fgtpuframe *)hdr)->set_length(length);
};

uint32_t get_gtpu_teid(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_teid();
};

void set_gtpu_teid(void *hdr, uint32_t teid){
	((rofl::fgtpuframe *)hdr)->set_teid(teid);
};

uint16_t get_gtpu_seq_no(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_seq_no();
};

void set_gtpu_seq_no(void *hdr, uint16_t seqno){
	((rofl::fgtpuframe *)hdr)->set_seq_no(seqno);
};

uint8_t get_gtpu_npdu_no(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_npdu_no();
};

void set_gtpu_npdu_no(void *hdr, uint8_t npdu){
	((rofl::fgtpuframe *)hdr)->set_npdu_no(npdu);
};

uint8_t get_gtpu_ext_type(void *hdr){
	return ((rofl::fgtpuframe *)hdr)->get_ext_type();
};

void set_gtpu_ext_type(void *hdr, uint8_t exthdr){
	((rofl::fgtpuframe *)hdr)->set_ext_type(exthdr);
};

