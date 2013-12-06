#include "cpp_ethernet.h"
#include <rofl/common/protocols/fetherframe.h>


uint64_t get_ether_dl_dst(void *hdr){
	return ((rofl::fetherframe *)hdr)->get_dl_dst().get_mac();
};

void set_ether_dl_dst(void* hdr, uint64_t dl_dst){
	((rofl::fetherframe *)hdr)->set_dl_dst(rofl::cmacaddr(dl_dst));
};

uint64_t get_ether_dl_src(void* hdr){
	return ((rofl::fetherframe *)hdr)->get_dl_src().get_mac();
};

void set_ether_dl_src(void* hdr, uint64_t dl_src){
	((rofl::fetherframe *)hdr)->set_dl_src(rofl::cmacaddr(dl_src));
};

uint16_t get_ether_type(void* hdr){
	return ((rofl::fetherframe *)hdr)->get_dl_type();
};

void set_ether_type(void* hdr, uint16_t dl_type){
	((rofl::fetherframe *)hdr)->set_dl_type(dl_type);
};

