#include "dpdk_datapacket.h"

datapacket_dpdk_t* create_datapacket_dpdk(datapacket_t* pkt){
	datapacket_dpdk_t* dpkt = (datapacket_dpdk_t*) malloc(sizeof(datapacket_dpdk_t));
	dpkt->headers = init_classifier(pkt);
	dpkt->icmpv4_recalc_checksum = false;
	dpkt->ipv4_recalc_checksum = false;
	dpkt->tcp_recalc_checksum = false;
	dpkt->udp_recalc_checksum = false;
	dpkt->packet_in_bufferpool = false;
	return dpkt;
}

void destroy_datapacket_dpdk(datapacket_dpdk_t* dpkt){
	destroy_classifier(dpkt->headers);
	free(dpkt);
}

/*
 * Push&pop operations
 */
rofl_result_t push_datapacket_offset(datapacket_dpdk_t *dpkt, unsigned int offset, unsigned int num_of_bytes){
	
	uint8_t *src_ptr = get_buffer_dpdk(dpkt);
	uint8_t *dst_ptr = (uint8_t*)rte_pktmbuf_prepend(dpkt->mbuf, num_of_bytes);
	//NOTE dst_ptr = src_ptr - num_of_bytes
	
	if( NULL==dst_ptr )
		return ROFL_FAILURE;
	
	if(false==rte_pktmbuf_is_contiguous(dpkt->mbuf)){
		assert(0);
		return ROFL_FAILURE;
	}

	// move header num_of_bytes backward
	memmove(dst_ptr, src_ptr, offset);
	
#ifndef NDEBUG
	// initialize new pushed memory area with 0x00
	memset(dst_ptr + offset, 0x00, num_of_bytes);
#endif

	return ROFL_SUCCESS;
}


rofl_result_t pop_datapacket_offset(datapacket_dpdk_t *dpkt, unsigned int offset, unsigned int num_of_bytes){
	
	uint8_t *src_ptr = get_buffer_dpdk(dpkt);
	uint8_t *dst_ptr = (uint8_t*)rte_pktmbuf_adj(dpkt->mbuf, num_of_bytes);
	//NOTE dst_ptr = src_ptr + num_of_bytes
	
	if( NULL==dst_ptr )
		return ROFL_FAILURE;

	if(false==rte_pktmbuf_is_contiguous(dpkt->mbuf)){
		assert(0);
		return ROFL_FAILURE;
	}
	
	// move first bytes backward
	memmove(dst_ptr, src_ptr, offset);
	
#ifndef NDEBUG
	// set now unused bytes to 0x00 for easier debugging
	memset(src_ptr, 0x00, num_of_bytes);
#endif

	return ROFL_SUCCESS;
}


rofl_result_t push_datapacket_point(datapacket_dpdk_t *dpkt, uint8_t* push_point, unsigned int num_of_bytes){
	
	uint8_t *src_ptr = get_buffer_dpdk(dpkt);
	
	if(false==rte_pktmbuf_is_contiguous(dpkt->mbuf)){
		assert(0);
		return ROFL_FAILURE;
	}
	
	if (push_point < src_ptr){
		return ROFL_FAILURE;
	}

	if (((uint8_t*)push_point + num_of_bytes) > (src_ptr + get_buffer_length_dpdk(dpkt))){
		return ROFL_FAILURE;
	}

	size_t offset = (push_point - src_ptr);

	return push_datapacket_offset(dpkt, offset, num_of_bytes);
}



rofl_result_t pop_datapacket_point(datapacket_dpdk_t *dpkt, uint8_t* pop_point, unsigned int num_of_bytes){
	
	uint8_t *src_ptr = get_buffer_dpdk(dpkt);
	
	if(false==rte_pktmbuf_is_contiguous(dpkt->mbuf)){
		assert(0);
		return ROFL_FAILURE;
	}
	
	if (pop_point < src_ptr){
		return ROFL_FAILURE;
	}

	if (((uint8_t*)pop_point + num_of_bytes) > (src_ptr + get_buffer_length_dpdk(dpkt))){
		return ROFL_FAILURE;
	}

	size_t offset = (src_ptr - pop_point);

	return pop_datapacket_offset(dpkt, offset, num_of_bytes);
}
