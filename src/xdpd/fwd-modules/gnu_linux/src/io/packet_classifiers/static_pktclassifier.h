/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STATIC_PKTCLASSIFIER_H
#define STATIC_PKTCLASSIFIER_H 

#include <assert.h> 
#include <rofl/common/protocols/fetherframe.h>
#include <rofl/common/protocols/fvlanframe.h>
#include <rofl/common/protocols/fmplsframe.h>
#include <rofl/common/protocols/farpv4frame.h>
#include <rofl/common/protocols/fipv4frame.h>
#include <rofl/common/protocols/ficmpv4frame.h>
#include <rofl/common/protocols/fipv6frame.h>
#include <rofl/common/protocols/ficmpv6frame.h>
#include <rofl/common/protocols/fudpframe.h>
#include <rofl/common/protocols/ftcpframe.h>
#include <rofl/common/protocols/fsctpframe.h>
#include <rofl/common/protocols/fpppoeframe.h>
#include <rofl/common/protocols/fpppframe.h>
#include <rofl/common/protocols/fgtpuframe.h>

#include "packetclassifier.h"

/**
* @file static_pktclassifier.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Static memory data packet header classifier 
*/

namespace xdpd {
namespace gnu_linux {

class static_pktclassifier: public packetclassifier{

public:

	//Constructor&destructor
	static_pktclassifier(datapacketx86* pkt_ref);
	virtual ~static_pktclassifier();

	/*
	* Main classification methods. 
	*/
	virtual void classify(void);
	virtual void classify_reset(void){
	
		for(unsigned int i=0;i<MAX_HEADERS;i++){
			headers[i].present = false;
			headers[i].next = headers[i].prev = NULL;
		}

		memset(num_of_headers,0,sizeof(num_of_headers));
		is_classified=false; 
	};

	
	/*
	* header access
	*/	
	virtual rofl::fetherframe* 	ether(int idx = 0)	const;
	virtual rofl::fvlanframe* 	vlan(int idx = 0) 	const;
	virtual rofl::fmplsframe* 	mpls(int idx = 0) 	const;
	virtual rofl::farpv4frame* 	arpv4(int idx = 0)	const;
	virtual rofl::fipv4frame* 	ipv4(int idx = 0) 	const;
	virtual rofl::ficmpv4frame* 	icmpv4(int idx = 0)	const;
	virtual rofl::fipv6frame* 	ipv6(int idx = 0) 	const;
	virtual rofl::ficmpv6frame* 	icmpv6(int idx = 0)	const;
	virtual rofl::fudpframe* 	udp(int idx = 0) 	const;
	virtual rofl::ftcpframe* 	tcp(int idx = 0) 	const;
	virtual rofl::fsctpframe* 	sctp(int idx = 0) 	const;
	virtual rofl::fpppoeframe* 	pppoe(int idx = 0)	const;
	virtual rofl::fpppframe* 	ppp(int idx = 0) 	const;
	virtual rofl::fgtpuframe*	gtp(int idx = 0)	const;

	/*
	 * pop operations
	 */
	virtual void pop_vlan(void);
	virtual void pop_mpls(uint16_t ether_type);
	virtual void pop_pppoe(uint16_t ether_type);
	virtual void pop_gtp(uint16_t ether_type);

	/*
	 * push operations
	 */
	virtual rofl::fvlanframe* 	push_vlan(uint16_t ether_type);
	virtual rofl::fmplsframe* 	push_mpls(uint16_t ether_type);
	virtual rofl::fpppoeframe* 	push_pppoe(uint16_t ether_type);
	virtual rofl::fgtpuframe*	push_gtp(uint16_t ether_type);

	/*
	* dump
	*/
	virtual void dump(void);

	/** returns length of parsed packet (may be shortened during Packet-In)
	 *
	 */
	virtual size_t
	get_pkt_len(
			rofl::fframe *from = (rofl::fframe*)0,
			rofl::fframe   *to = (rofl::fframe*)0);


protected:

	//Flag to know if it is classified
	bool is_classified;

	//Inner most (last) ethertype
	uint16_t eth_type;
	
	//Header type
	enum header_type{
		HEADER_TYPE_ETHER = 0,	
		HEADER_TYPE_VLAN = 1,	
		HEADER_TYPE_MPLS = 2,	
		HEADER_TYPE_ARPV4 = 3,	
		HEADER_TYPE_IPV4 = 4,	
		HEADER_TYPE_ICMPV4 = 5,
		HEADER_TYPE_IPV6 = 6,	
		HEADER_TYPE_ICMPV6 = 7,	
		HEADER_TYPE_UDP = 8,	
		HEADER_TYPE_TCP = 9,	
		HEADER_TYPE_SCTP = 10,	
		HEADER_TYPE_PPPOE = 11,	
		HEADER_TYPE_PPP = 12,	
		HEADER_TYPE_GTP = 13,

		//Must be the last one
		HEADER_TYPE_MAX
	};

	//Maximum header occurrences per type
	static const unsigned int MAX_ETHER_FRAMES = 2;
	static const unsigned int MAX_VLAN_FRAMES = 4;
	static const unsigned int MAX_MPLS_FRAMES = 16;
	static const unsigned int MAX_ARPV4_FRAMES = 1;
	static const unsigned int MAX_IPV4_FRAMES = 2;
	static const unsigned int MAX_ICMPV4_FRAMES = 2;
	static const unsigned int MAX_IPV6_FRAMES = 2;
	static const unsigned int MAX_ICMPV6_FRAMES = 2;
	static const unsigned int MAX_UDP_FRAMES = 2;
	static const unsigned int MAX_TCP_FRAMES = 2;
	static const unsigned int MAX_SCTP_FRAMES = 2;
	static const unsigned int MAX_PPPOE_FRAMES = 1;
	static const unsigned int MAX_PPP_FRAMES = 1;
	static const unsigned int MAX_GTP_FRAMES = 1;

	//Total maximum header occurrences
	static const unsigned int MAX_HEADERS = MAX_ETHER_FRAMES +
							MAX_VLAN_FRAMES +
							MAX_MPLS_FRAMES +
							MAX_ARPV4_FRAMES +
							MAX_IPV4_FRAMES +
							MAX_ICMPV4_FRAMES +
							MAX_IPV6_FRAMES +
							MAX_ICMPV6_FRAMES +
							MAX_UDP_FRAMES +
							MAX_TCP_FRAMES +
							MAX_SCTP_FRAMES +
							MAX_PPPOE_FRAMES + 
							MAX_PPP_FRAMES +
							MAX_GTP_FRAMES;


	//Relative positions within the array;
	static const unsigned int FIRST_ETHER_FRAME_POS = 0; //Very first frame always
	static const unsigned int FIRST_VLAN_FRAME_POS = FIRST_ETHER_FRAME_POS+MAX_ETHER_FRAMES;
	static const unsigned int FIRST_MPLS_FRAME_POS = FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES;
	static const unsigned int FIRST_ARPV4_FRAME_POS = FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES;
	static const unsigned int FIRST_IPV4_FRAME_POS = FIRST_ARPV4_FRAME_POS+MAX_ARPV4_FRAMES;
	static const unsigned int FIRST_ICMPV4_FRAME_POS = FIRST_IPV4_FRAME_POS+MAX_IPV4_FRAMES;
	static const unsigned int FIRST_IPV6_FRAME_POS = FIRST_ICMPV4_FRAME_POS+MAX_ICMPV4_FRAMES;
	static const unsigned int FIRST_ICMPV6_FRAME_POS = FIRST_IPV6_FRAME_POS+MAX_IPV6_FRAMES;
	static const unsigned int FIRST_UDP_FRAME_POS = FIRST_ICMPV6_FRAME_POS+MAX_ICMPV6_FRAMES;
	static const unsigned int FIRST_TCP_FRAME_POS = FIRST_UDP_FRAME_POS+MAX_UDP_FRAMES;
	static const unsigned int FIRST_SCTP_FRAME_POS = FIRST_TCP_FRAME_POS+MAX_TCP_FRAMES;
	static const unsigned int FIRST_PPPOE_FRAME_POS = FIRST_SCTP_FRAME_POS+MAX_SCTP_FRAMES;
	static const unsigned int FIRST_PPP_FRAME_POS = FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES;
	static const unsigned int FIRST_GTP_FRAME_POS = FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES;

	//Just to be on the safe side of life
	//assert( (FIRST_PPP_FRAME_POS + MAX_PPP_FRAMES) == MAX_HEADERS);

	//Counters
	unsigned int num_of_headers[HEADER_TYPE_MAX];

 
	//Header container
	typedef struct header_container{

		//Presence of header
		bool present;
		
		//ROFL header 
		rofl::fframe* frame;	
		enum header_type type; 

		//Pseudo-linked list pointers (short-cuts)
		struct header_container* prev;
		struct header_container* next;
	}header_container_t;

	//Real container
	header_container_t headers[MAX_HEADERS];	
	// ethernet type of payload (=innermost ethernet type)
	rofl::fframe* frame_insert(rofl::fframe *append_to, rofl::fframe *frame);
	void frame_append(rofl::fframe *frame);
	void frame_push(rofl::fframe *frame);
	void frame_pop(rofl::fframe *frame);

	void parse_ether	(uint8_t *data, size_t datalen);
	void parse_vlan(uint8_t *data, size_t datalen);
	void parse_mpls(uint8_t *data, size_t datalen);
	void parse_pppoe(uint8_t *data, size_t datalen);
	void parse_ppp(uint8_t *data, size_t datalen);
	void parse_arpv4(uint8_t *data, size_t datalen);
	void parse_ipv4	(uint8_t *data, size_t datalen);
	void parse_icmpv4(uint8_t *data, size_t datalen);
	void parse_ipv6	(uint8_t *data, size_t datalen);
	void parse_icmpv6(uint8_t *data, size_t datalen);
	void parse_udp(uint8_t *data, size_t datalen);
	void parse_tcp(uint8_t *data, size_t datalen);
	void parse_sctp	(uint8_t *data, size_t datalen);
	void parse_gtp(uint8_t *data, size_t datalen);

	//Insert/pop frame
	void pop_header(enum header_type type, unsigned int start, unsigned int end);
	void push_header(enum header_type type, unsigned int start, unsigned int end);
};

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* STATIC_PKTCLASSIFIER_H_ */
