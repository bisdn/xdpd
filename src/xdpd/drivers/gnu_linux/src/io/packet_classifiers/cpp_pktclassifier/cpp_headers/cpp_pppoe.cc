#include "cpp_pppoe.h"
#include <rofl/common/protocols/fpppoeframe.h>

uint8_t get_pppoe_vers(void *hdr){
	return ((rofl::fpppoeframe*)hdr)->get_pppoe_vers();
}

void set_pppoe_vers(void *hdr, uint8_t version){
	((rofl::fpppoeframe*)hdr)->set_pppoe_vers(version);
}

uint8_t get_pppoe_type(void *hdr){
	return ((rofl::fpppoeframe*)hdr)->get_pppoe_type();
}

void set_pppoe_type(void *hdr, uint8_t type){
	((rofl::fpppoeframe*)hdr)->set_pppoe_type(type);
}

uint8_t get_pppoe_code(void *hdr){
	return ((rofl::fpppoeframe*)hdr)->get_pppoe_code();
}

void set_pppoe_code(void *hdr, uint8_t code){
	((rofl::fpppoeframe*)hdr)->set_pppoe_code(code);
}


uint16_t get_pppoe_sessid(void *hdr){
	return ((rofl::fpppoeframe*)hdr)->get_pppoe_sessid();
}

void set_pppoe_sessid(void *hdr, uint16_t sessid){
	((rofl::fpppoeframe*)hdr)->set_pppoe_sessid(sessid);
}

uint16_t get_pppoe_length(void *hdr){
	return ((rofl::fpppoeframe*)hdr)->get_hdr_length();
}

void set_pppoe_length(void *hdr, uint16_t length){
	((rofl::fpppoeframe*)hdr)->set_hdr_length(length);
}

