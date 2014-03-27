#include "packet_operations.h"
#include "../dpdk_datapacket.h"

// This file is created to let the packet classifiers be independent from the datapacket, which is platform dependent

rofl_result_t pkt_push(datapacket_t* pkt, uint8_t* push_point, unsigned int offset, unsigned int num_of_bytes){
	datapacket_dpdk_t *pkt_state = (datapacket_dpdk_t*)(pkt->platform_state);
	if(push_point)
		return push_datapacket_point(pkt_state,push_point,num_of_bytes);
	else
		return push_datapacket_offset(pkt_state, offset, num_of_bytes);
}

rofl_result_t pkt_pop(datapacket_t* pkt, uint8_t* pop_point, unsigned int offset, unsigned int num_of_bytes){
	datapacket_dpdk_t *pkt_state = (datapacket_dpdk_t*)(pkt->platform_state);
	if(pop_point)
		return pop_datapacket_point(pkt_state, pop_point,num_of_bytes);
	else
		return pop_datapacket_offset(pkt_state, offset, num_of_bytes);
}

size_t get_buffer_length(datapacket_t* pkt){
	return get_buffer_length_dpdk((datapacket_dpdk_t*)pkt->platform_state);
}
