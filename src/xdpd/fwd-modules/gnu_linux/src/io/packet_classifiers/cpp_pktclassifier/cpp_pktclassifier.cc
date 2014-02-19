#include "cpp_pktclassifier.h"

#include "static_pktclassifier.h"


using xdpd::gnu_linux::static_pktclassifier;

//typedef struct classify_state pktclassifier;
typedef static_pktclassifier pktclassifier;

struct classify_state* init_classifier(datapacket_t*const pkt){
	return (struct classify_state*) new pktclassifier(pkt);
}

void destroy_classifier(struct classify_state* clas_state){
	delete ((pktclassifier*)clas_state);
}

void classify_packet(struct classify_state* clas_state, uint8_t* pkt, size_t len,  uint32_t port_in, uint32_t phy_port_in){
	((pktclassifier*)clas_state)->classify(pkt, len, port_in, phy_port_in);
}

void reset_classifier(struct classify_state* clas_state){
	((pktclassifier*)clas_state)->classify_reset();
}


/* Header getters */
void* get_ether_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->ether(idx);
}

void* get_vlan_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->vlan(idx);
}

void* get_mpls_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->mpls(idx);
}

void* get_arpv4_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->arpv4(idx);
}

void* get_ipv4_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->ipv4(idx);
}

void* get_icmpv4_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->icmpv4(idx);
}

void* get_ipv6_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->ipv6(idx);
}

void* get_icmpv6_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->icmpv6(idx);
}

void* get_udp_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->udp(idx);
}

void* get_tcp_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->tcp(idx);
}

void* get_pppoe_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->pppoe(idx);
}

void* get_ppp_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->ppp(idx);
}

void* get_gtpu_hdr(struct classify_state* clas_state, int idx){
	return ((pktclassifier*)clas_state)->gtp(idx);
}


/* push & pop */
void pop_vlan(datapacket_t* pkt, struct classify_state* clas_state){
	((pktclassifier*)clas_state)->pop_vlan(pkt);
}

void pop_mpls(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type){
	((pktclassifier*)clas_state)->pop_mpls(pkt, ether_type);
}

void pop_pppoe(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type){
	((pktclassifier*)clas_state)->pop_pppoe(pkt, ether_type);
}

void pop_gtp(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type){
	//TODO ((pktclassifier*)clas_state)->pop_gtp();
}


void* push_vlan(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type){
	return ((pktclassifier*)clas_state)->push_vlan(pkt, ether_type);
}

void* push_mpls(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type){
	return ((pktclassifier*)clas_state)->push_mpls(pkt, ether_type);
}

void* push_pppoe(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type){
	return ((pktclassifier*)clas_state)->push_pppoe(pkt, ether_type);
}

void* push_gtp(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type){
	//TODO ((pktclassifier*)clas_state)->push_gtp();
	return NULL;
}

size_t get_pkt_len(datapacket_t* pkt, struct classify_state* clas_state, void *from, void *to){
	return ((pktclassifier*)clas_state)->get_pkt_len(pkt, from, to);
}
