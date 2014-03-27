#include "packet_operations.h"
#include "../datapacketx86.h"

// This file is created to let the packet classifiers be independent from the datapacket, which is platform dependent

rofl_result_t pkt_push(datapacket_t* pkt, uint8_t* push_point, unsigned int offset, unsigned int num_of_bytes){
	xdpd::gnu_linux::datapacketx86 *pkt_state = (xdpd::gnu_linux::datapacketx86*)pkt->platform_state;
	if(push_point)
		return pkt_state->push(push_point,num_of_bytes);
	else
		return pkt_state->push(offset, num_of_bytes);
}

rofl_result_t pkt_pop(datapacket_t* pkt, uint8_t* pop_point, unsigned int offset, unsigned int num_of_bytes){
	xdpd::gnu_linux::datapacketx86 *pkt_state = (xdpd::gnu_linux::datapacketx86*)pkt->platform_state;
	if(pop_point)
		return pkt_state->pop(pop_point,num_of_bytes);
	else
		return pkt_state->pop(offset, num_of_bytes);
}

size_t get_buffer_length(datapacket_t* pkt){
	return ((xdpd::gnu_linux::datapacketx86*)pkt->platform_state)->get_buffer_length();
}
