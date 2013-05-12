/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STATIC_PKTCLASSIFIER_H
#define STATIC_PKTCLASSIFIER_H 

#include <assert.h> 
#include "packetclassifier.h"

/**
* @file static_pktclassifier.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Static memory data packet header classifier 
*/

class static_pktclassifier: public packetclassifier{

public:

	//Constructor&destructor
	static_pktclassifier(datapacketx86* pkt_ref) :
		packetclassifier(pkt_ref),
	{
		classify_reset();	
	}; 

	virtual ~static_pktclassifier(){};

	/*
	* Main classification methods. 
	*/
	virtual void classify(void);
	virtual void classify_reset(void){
	
		for(int i=0;i<MAX_HEADERS;i++){
			headers[i].present = false;
			headers[i].next = headers[i].prev = NULL;
		}

		memset(num_of_headers,0,sizeof(num_of_headers));
		is_classified=false; 
	};

	
	/*
	* header access
	*/	
	virtual rofl::fetherframe* 	ether(unsigned int idx = 0)	const;
	virtual rofl::fvlanframe* 	vlan(unsigned int idx = 0) 	const;
	virtual rofl::fmplsframe* 	mpls(unsigned int idx = 0) 	const;
	virtual rofl::farpv4frame* 	arpv4(unsigned int idx = 0)	const;
	virtual rofl::fipv4frame* 	ipv4(unsigned int idx = 0) 	const;
	virtual rofl::ficmpv4frame* 	icmpv4(unsigned int idx = 0)	const;
	virtual rofl::fudpframe* 	udp(unsigned int idx = 0) 	const;
	virtual rofl::ftcpframe* 	tcp(unsigned int idx = 0) 	const;
	virtual rofl::fsctpframe* 	sctp(unsigned int idx = 0) 	const;
	virtual rofl::fpppoeframe* 	pppoe(unsigned int idx = 0)	const;
	virtual rofl::fpppframe* 	ppp(unsigned int idx = 0) 	const;

	/*
	 * pop operations
	 */
	virtual void pop_vlan(void);
	virtual void pop_mpls(uint16_t ether_type);
	virtual void pop_pppoe(uint16_t ether_type);

	/*
	 * push operations
	 */
	virtual rofl::fvlanframe* 	push_vlan(uint16_t ether_type);
	virtual rofl::fmplsframe* 	push_mpls(uint16_t ether_type);
	virtual rofl::fpppoeframe* 	push_pppoe(uint16_t ether_type);

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
		HEADER_TYPE_UDP = 6,	
		HEADER_TYPE_TCP = 7,	
		HEADER_TYPE_SCTCP = 8,	
		HEADER_TYPE_PPPOE = 9,	
		HEADER_TYPE_PPP = 10,	

		//Must be the last one
		HEADER_TYPE_MAX = 11	
	};

	//Maximum header occurrences per type
	static const unsigned int MAX_ETHER_FRAMES = 2;
	static const unsigned int MAX_VLAN_FRAMES = 4;
	static const unsigned int MAX_MPLS_FRAMES = 16;
	static const unsigned int MAX_ARPV4_FRAMES = 1;
	static const unsigned int MAX_IPV4_FRAMES = 2;
	static const unsigned int MAX_ICMPV4_FRAMES = 2;
	static const unsigned int MAX_UDP_FRAMES = 2;
	static const unsigned int MAX_TCP_FRAMES = 2;
	static const unsigned int MAX_SCTP_FRAMES = 2;
	static const unsigned int MAX_PPPOE_FRAMES = 1;
	static const unsigned int MAX_PPP_FRAMES = 1;

	//Total maximum header occurrences
	static const unsigned int MAX_HEADERS = MAX_ETHER_FRAMES +
							MAX_VLAN_FRAMES +
							MAX_MPLS_FRAMES +
							MAX_ARPV4_FRAMES +
							MAX_IPV4_FRAMES +
							MAX_ICMPV4_FRAMES +
							MAX_UDP_FRAMES +
							MAX_TCP_FRAMES +
							MAX_SCTP_FRAMES +
							MAX_PPPOE_FRAMES + 
							MAX_PPP_FRAMES; 


	//Relative positions within the array;
	static const unsigned int FIRST_ETHER_FRAME_POS = 0; //Very first frame always
	static const unsigned int FIRST_VLAN_FRAME_POS = FIRST_ETHER_FRAME_POS+MAX_ETHER_FRAMES;
	static const unsigned int FIRST_MPLS_FRAME_POS = FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES;
	static const unsigned int FIRST_ARPV4_FRAME_POS = FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES;
	static const unsigned int FIRST_IPV4_FRAME_POS = FIRST_ARPV4_FRAME_POS+MAX_ARPV4_FRAMES;
	static const unsigned int FIRST_ICMPV4_FRAME_POS = FIRST_IPV4_FRAME_POS+MAX_IPV4_FRAMES;
	static const unsigned int FIRST_UDP_FRAME_POS = FIRST_ICMPV4_FRAME_POS+MAX_ICMPV4_FRAMES;
	static const unsigned int FIRST_TCP_FRAME_POS = FIRST_UDP_FRAME_POS+MAX_UDP_FRAMES;
	static const unsigned int FIRST_SCTP_FRAME_POS = FIRST_TCP_FRAME_POS+MAX_TCP_FRAMES;
	static const unsigned int FIRST_PPPOE_FRAME_POS = FIRST_SCTP_FRAME_POS+MAX_SCTP_FRAMES;
	static const unsigned int FIRST_PPP_FRAME_POS = FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES;

	//Just to be on the safe side of life
	assert(FIRST_PPP_FRAME_POS + MAX_PPP_FRAMES == MAX_HEADERS);

	//Counters
	unsigned int num_of_headers[HEADER_TYPE_MAX];

 
	//Header container
	typedef struct header_container{

		//Presence of header
		bool present;
		
		//ROFL header 
		rofl::fframe frame;	
		enum header_type type; 

		//Pseudo-linked list pointers (short-cuts)
		struct header_container* prev;
		struct header_container* next;
	}header_container_t;

	//Real container
	header_container_t headers[MAX_HEADERS];	
};


#endif /* STATIC_PKTCLASSIFIER_H_ */
