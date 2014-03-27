#include "packetclassifier.h"
#include "../../datapacketx86.h"

/*
* Implementation of the wrappers
*/
using namespace xdpd::gnu_linux;

rofl_result_t packetclassifier::pkt_push(datapacket_t* pkt, unsigned int offset, unsigned int num_of_bytes){
	return ((datapacketx86 *)pkt->platform_state)->push(offset,num_of_bytes);
}
rofl_result_t packetclassifier::pkt_pop(datapacket_t* pkt, unsigned int offset, unsigned int num_of_bytes){
	return ((datapacketx86 *)pkt->platform_state)->pop(offset,num_of_bytes);
}
rofl_result_t packetclassifier::pkt_push(datapacket_t* pkt, uint8_t* push_point, unsigned int num_of_bytes){
	return ((datapacketx86 *)pkt->platform_state)->push(push_point,num_of_bytes);
}
rofl_result_t packetclassifier::pkt_pop(datapacket_t *pkt, uint8_t* pop_point, unsigned int num_of_bytes){ 
	return ((datapacketx86 *)pkt->platform_state)->pop(pop_point,num_of_bytes);
}

size_t packetclassifier::get_buffer_length(datapacket_t* pkt){
	return ((datapacketx86 *)pkt->platform_state)->get_buffer_length();
}

uint8_t* packetclassifier::get_buffer(datapacket_t* pkt){
	return ((datapacketx86 *)pkt->platform_state)->get_buffer();
}