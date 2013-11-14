#include "push_pop_operations.h"
#include "../datapacketx86.h"
inline static
rofl_result_t pkt_push(datapacket_t* pkt, uint8_t* push_point, unsigned int num_of_bytes, unsigned int offset){
	datapacketx86 *pkt_state = (datapacketx86*)pkt->platform_state;
	if(push_point)
		return pkt_state->push(push_point,num_of_bytes);
	else
		return pkt_state->push(num_of_bytes,offset);
};

inline static
rofl_result_t pkt_pop(datapacket_t* pkt, uint8_t* pop_point, unsigned int num_of_bytes, unsigned int offset){
	datapacketx86 *pkt_state = (datapacketx86*)pkt->platform_state;
	if(pop_point)
		return pkt_state->pop(pop_point,num_of_bytes);
	else
		return pkt_state->pop(num_of_bytes,offset);
};