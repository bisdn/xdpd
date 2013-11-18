#ifndef _CPP_GTPU_H_
#define _CPP_GTPU_H_

#include <rofl/common/protocols/fgtpuframe.h>

using namespace rofl;
using namespace xdpd::gnu_linux;


inline static
uint8_t get_gtp_version(void *hdr){
	return ((fgtpuframe *)hdr)->get_version();
};

inline static
void set_gtp_version(void *hdr, uint8_t version){
	((fgtpuframe *)hdr)->set_version(version);
};

inline static
bool get_pt_flag(void *hdr){
	return ((fgtpuframe *)hdr)->get_pt_flag(); 
};

inline static
void set_pt_flag(void *hdr, bool pt){
	((fgtpuframe *)hdr)->set_pt_flag(pt);
};

inline static
bool get_e_flag(void *hdr){
	return ((fgtpuframe *)hdr)->get_e_flag(); 
};

inline static
void set_e_flag(void *hdr, bool e){
	((fgtpuframe *)hdr)->set_e_flag(e);
};

inline static
bool get_s_flag(void *hdr){
	return ((fgtpuframe *)hdr)->get_s_flag();
};

inline static
void set_s_flag(void *hdr, bool s){
	((fgtpuframe *)hdr)->set_s_flag(s);
};

inline static
bool get_pn_flag(void *hdr){
	return ((fgtpuframe *)hdr)->get_pn_flag();
};

inline static
void set_pn_flag(void *hdr, bool pn){
	((fgtpuframe *)hdr)->set_pn_flag(pn);
};

inline static
uint8_t get_msg_type(void *hdr){
	return ((fgtpuframe *)hdr)->get_msg_type();
};

inline static
void set_msg_type(void *hdr, uint8_t msgtype){
	((fgtpuframe *)hdr)->set_msg_type(msgtype);
};

inline static
uint16_t get_gtp_length(void *hdr){
	return ((fgtpuframe *)hdr)->get_length();
};

inline static
void set_gtp_length(void *hdr, uint16_t length){
	((fgtpuframe *)hdr)->set_length(length);
};

inline static
uint32_t get_teid(void *hdr){
	return ((fgtpuframe *)hdr)->get_teid();
};

inline static
void set_teid(void *hdr, uint32_t teid){
	((fgtpuframe *)hdr)->set_teid(teid);
};

inline static
uint16_t get_seq_no(void *hdr){
	return ((fgtpuframe *)hdr)->get_seq_no();
};

inline static
void set_seq_no(void *hdr, uint16_t seqno){
	((fgtpuframe *)hdr)->set_seq_no(seqno);
};

inline static
uint8_t get_npdu_no(void *hdr){
	return ((fgtpuframe *)hdr)->get_npdu_no();
};

inline static
void set_npdu_no(void *hdr, uint8_t npdu){
	((fgtpuframe *)hdr)->set_npdu_no(npdu);
};

inline static
uint8_t get_ext_type(void *hdr){
	return ((fgtpuframe *)hdr)->get_ext_type();
};

inline static
void set_ext_type(void *hdr, uint8_t exthdr){
	((fgtpuframe *)hdr)->set_ext_type(exthdr);
};

#endif //_CPP_GTPU_H_
