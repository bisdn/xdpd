#ifndef _PKTCLASSIFIER_INTERFACE_H_
#define _PKTCLASSIFIER_INTERFACE_H_

#include <stddef.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "packetclassifier.h"

#include "./cpp_headers/cpp_ethernet.h"
#include "./cpp_headers/cpp_arpv4.h"
#include "./cpp_headers/cpp_gtpu.h"
#include "./cpp_headers/cpp_icmpv4.h"
#include "./cpp_headers/cpp_icmpv6.h"
#include "./cpp_headers/cpp_ipv4.h"
#include "./cpp_headers/cpp_ipv6.h"
#include "./cpp_headers/cpp_mpls.h"
#include "./cpp_headers/cpp_ppp.h"
#include "./cpp_headers/cpp_pppoe.h"
#include "./cpp_headers/cpp_tcp.h"
#include "./cpp_headers/cpp_udp.h"
#include "./cpp_headers/cpp_vlan.h"

struct classify_state;

//function declarations
struct classify_state* init_classifier();
void destroy_classifier(struct classify_state* clas_state);
void classify_packet(struct classify_state* clas_state, uint8_t* pkt, size_t len);
void reset_classifier(struct classify_state* clas_state);

void* ether(struct classify_state* clas_state, int idx);
void* vlan(struct classify_state* clas_state, int idx);
void* mpls(struct classify_state* clas_state, int idx);
void* arpv4(struct classify_state* clas_state, int idx);
void* ipv4(struct classify_state* clas_state, int idx);
void* icmpv4(struct classify_state* clas_state, int idx);
void* ipv6(struct classify_state* clas_state, int idx);
void* icmpv6(struct classify_state* clas_state, int idx);
void* udp(struct classify_state* clas_state, int idx);
void* tcp(struct classify_state* clas_state, int idx);
void* pppoe(struct classify_state* clas_state, int idx);
void* ppp(struct classify_state* clas_state, int idx);
void* gtp(struct classify_state* clas_state, int idx);

//push & pop
void pop_vlan(datapacket_t* pkt, struct classify_state* clas_state);
void pop_mpls(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void pop_pppoe(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void pop_gtp(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);

void* push_vlan(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void* push_mpls(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void* push_pppoe(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void* push_gtp(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);

//void pkt_push();
//void pkt_pop();

void dump_pkt_classifier(struct classify_state* clas_state);
size_t get_pkt_len(datapacket_t* pkt, struct classify_state* clas_state, void *from, void *to);

#endif //_C_PKTCLASSIFIER_H_
