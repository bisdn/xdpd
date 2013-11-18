#ifndef _CPP_VLAN_H_
#define _CPP_VLAN_H_

#include <rofl/common/protocols/fvlanframe.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
void set_dl_vlan_id(void* hdr, uint16_t vid){
	((fvlanframe*)hdr)->set_dl_vlan_id(vid);
}

inline static
uint16_t get_dl_vlan_id(void* hdr){
	return ((fvlanframe*)hdr)->get_dl_vlan_id();
}

inline static
void set_dl_vlan_pcp(void* hdr, uint8_t pcp){
	((fvlanframe*)hdr)->set_dl_vlan_pcp(pcp);
}

inline static
uint16_t get_dl_vlan_pcp(void* hdr){
	return ((fvlanframe*)hdr)->get_dl_vlan_pcp();
}

inline static
void set_dl_vlan_cfi(void* hdr, bool cfi){
	((fvlanframe*)hdr)->set_dl_vlan_cfi(cfi);
}

inline static
bool get_dl_vlan_cfi(void* hdr){
	return ((fvlanframe*)hdr)->get_dl_vlan_cfi();
}

inline static
void set_dl_vlan_type(void* hdr, uint16_t dl_type){
	((fvlanframe*)hdr)->set_dl_type(dl_type);
}

inline static
uint16_t get_dl_vlan_type(void* hdr){
	return ((fvlanframe*)hdr)->get_dl_type();
}


#endif //_CPP_VLAN_H_
