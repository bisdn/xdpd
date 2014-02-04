#include "cpp_vlan.h"
#include <rofl/common/protocols/fvlanframe.h>

void set_vlan_id(void* hdr, uint16_t vid){
	((rofl::fvlanframe*)hdr)->set_dl_vlan_id(vid);
}

uint16_t get_vlan_id(void* hdr){
	return ((rofl::fvlanframe*)hdr)->get_dl_vlan_id();
}

void set_vlan_pcp(void* hdr, uint8_t pcp){
	((rofl::fvlanframe*)hdr)->set_dl_vlan_pcp(pcp);
}

uint16_t get_vlan_pcp(void* hdr){
	return ((rofl::fvlanframe*)hdr)->get_dl_vlan_pcp();
}

void set_vlan_cfi(void* hdr, bool cfi){
	((rofl::fvlanframe*)hdr)->set_dl_vlan_cfi(cfi);
}

bool get_vlan_cfi(void* hdr){
	return ((rofl::fvlanframe*)hdr)->get_dl_vlan_cfi();
}

void set_vlan_type(void* hdr, uint16_t dl_type){
	((rofl::fvlanframe*)hdr)->set_dl_type(dl_type);
}

uint16_t get_vlan_type(void* hdr){
	return ((rofl::fvlanframe*)hdr)->get_dl_type();
}
