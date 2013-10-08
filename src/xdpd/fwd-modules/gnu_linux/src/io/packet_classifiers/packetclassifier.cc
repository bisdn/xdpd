#include "packetclassifier.h"
#include "../datapacketx86.h"

/*
* Implementation of the wrappers
*/
using namespace xdpd::gnu_linux;

rofl_result_t packetclassifier::pkt_push(unsigned int num_of_bytes, unsigned int offset){
	return pkt->push(num_of_bytes,offset);
}
rofl_result_t packetclassifier::pkt_pop(unsigned int num_of_bytes, unsigned int offset){
	return pkt->pop(num_of_bytes,offset);
}
rofl_result_t packetclassifier::pkt_push(uint8_t* push_point, unsigned int num_of_bytes){
	return pkt->push(push_point,num_of_bytes);
}
rofl_result_t packetclassifier::pkt_pop(uint8_t* pop_point, unsigned int num_of_bytes){ 
	return pkt->pop(pop_point,num_of_bytes);
}


