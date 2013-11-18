#ifndef _CPP_PPPOE_H_
#define _CPP_PPPOE_H_

#include <rofl/common/protocols/fpppoeframe.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
uint8_t get_pppoe_vers(void *hdr){
	return ((fpppoeframe*)hdr)->get_pppoe_vers();
}

inline static
void set_pppoe_vers(void *hdr, uint8_t version){
	((fpppoeframe*)hdr)->set_pppoe_vers(version);
}

inline static
uint8_t get_pppoe_type(void *hdr){
	return ((fpppoeframe*)hdr)->get_pppoe_type();
}

inline static
void set_pppoe_type(void *hdr, uint8_t type){
	((fpppoeframe*)hdr)->set_pppoe_type(type);
}

inline static
uint8_t get_pppoe_code(void *hdr){
	return ((fpppoeframe*)hdr)->get_pppoe_code();
}

inline static
void set_pppoe_code(void *hdr, uint8_t code){
	((fpppoeframe*)hdr)->set_pppoe_code(code);
}


inline static
uint16_t get_pppoe_sessid(void *hdr){
	return ((fpppoeframe*)hdr)->get_pppoe_sessid();
}

inline static
void set_pppoe_sessid(void *hdr, uint16_t sessid){
	((fpppoeframe*)hdr)->set_pppoe_sessid(sessid);
}

inline static
uint16_t get_pppoe_length(void *hdr){
	return ((fpppoeframe*)hdr)->get_hdr_length();
}

inline static
void set_pppoe_length(void *hdr, uint16_t length){
	((fpppoeframe*)hdr)->set_hdr_length(length);
}

#endif //_CPP_PPPOE_H_
