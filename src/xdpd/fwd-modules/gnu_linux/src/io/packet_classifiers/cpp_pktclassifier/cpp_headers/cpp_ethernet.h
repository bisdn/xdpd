#ifndef _CPP_ETERNET_H_
#define _CPP_ETERNET_H_

#include <rofl/common/protocols/fetherframe.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
uint64_t get_dl_eth_dst(void *hdr){
	return ((fetherframe *)hdr)->get_dl_dst().get_mac();
};

inline static
void set_dl_eth_dst(void* hdr, uint64_t dl_dst){
	((fetherframe *)hdr)->set_dl_dst(rofl::cmacaddr(dl_dst));
};

inline static
uint64_t get_dl_eth_src(void* hdr){
	return ((fetherframe *)hdr)->get_dl_src().get_mac();
};

inline static
void set_dl_eth_src(void* hdr, uint64_t dl_src){
	((fetherframe *)hdr)->set_dl_src(rofl::cmacaddr(dl_src));
};

inline static
uint16_t get_dl_eth_type(void* hdr){
	return ((fetherframe *)hdr)->get_dl_type();
};

inline static
void set_dl_eth_type(void* hdr, uint16_t dl_type){
	((fetherframe *)hdr)->set_dl_type(dl_type);
};

#endif //_CPP_ETERNET_H_
