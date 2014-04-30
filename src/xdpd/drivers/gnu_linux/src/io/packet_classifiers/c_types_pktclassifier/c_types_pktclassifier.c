#include "c_types_pktclassifier.h"
#include <stdlib.h>
#include <string.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/common/protocol_constants.h>
#include "../packet_operations.h"
#include "../../../config.h"

/// Classify part
classify_state_t* init_classifier(datapacket_t*const  pkt){
	classify_state_t* classifier = malloc(sizeof(classify_state_t));
	memset(classifier,0,sizeof(classify_state_t));
	classifier->matches = &pkt->matches;
	return classifier;
}
void destroy_classifier(classify_state_t* clas_state){
	free(clas_state);
}

void pop_vlan(datapacket_t* pkt, classify_state_t* clas_state){
	//XXX FIXME
}
void pop_mpls(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	//XXX FIXME
}
void pop_pppoe(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	//XXX FIXME
}

void pop_gtp(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	//XXX FIXME
}

void* push_vlan(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	//XXX FIXME
	return NULL;
}

void* push_mpls(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	//XXX FIXME
	return NULL;
}

void* push_pppoe(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	//XXX FIXME
	return NULL;
}

void* push_gtp(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type){
	//XXX FIXME
	return NULL;
}

void dump_pkt_classifier(classify_state_t* clas_state){
	//XXX FIXME
}

size_t get_pkt_len(datapacket_t* pkt, classify_state_t* clas_state, void *from, void *to){
	//XXX FIXME
	return 0;
}
