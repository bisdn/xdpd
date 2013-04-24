/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ROFL_PKTCLASSIFIER_H
#define ROFL_PKTCLASSIFIER_H 

#include "packetclassifier.h"

/**
* @file rofl_pktclassifier.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief ROFL-based (C++) data packet classifier 
*
*/

class rofl_pktclassifier: public packetclassifier{

public:

	//Constructor&destructor
	rofl_pktclassifier(datapacketx86* pkt_ref) :
		packetclassifier(pkt_ref),
		is_classified(false),
		eth_type(0)
	{
		fhead = ftail = NULL;
	}; 
	virtual ~rofl_pktclassifier()
	{
		classify_reset();
	};

	/*
	* Main classification methods. 
	*/
	virtual void classify(void);
	virtual void classify_reset(void);

	
	/*
	* header access
	*/	
	template<class T> T* 	header(unsigned int idx) const;

	virtual rofl::fetherframe* 	ether(unsigned int idx = 0) const;
	virtual rofl::fvlanframe* 	vlan(unsigned int idx = 0) 	const;
	virtual rofl::fmplsframe* 	mpls(unsigned int idx = 0) 	const;
	virtual rofl::farpv4frame* 	arpv4(unsigned int idx = 0) const;
	virtual rofl::fipv4frame* 	ipv4(unsigned int idx = 0) 	const;
	virtual rofl::ficmpv4frame* icmpv4(unsigned int idx = 0) const;
	virtual rofl::fudpframe* 	udp(unsigned int idx = 0) 	const;
	virtual rofl::ftcpframe* 	tcp(unsigned int idx = 0) 	const;
	virtual rofl::fsctpframe* 	sctp(unsigned int idx = 0) 	const;
	virtual rofl::fpppoeframe* 	pppoe(unsigned int idx = 0) const;
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

	//Flag classified
	bool is_classified;	

	//Head and tail of all frames
	rofl::fframe	*fhead;	
	rofl::fframe	*ftail;

	enum rofl_pktclassifier_type_t {
		ROFL_PKT_CLASSIFIER_ETHER		= 1,
		ROFL_PKT_CLASSIFIER_VLAN		= 2,
		ROFL_PKT_CLASSIFIER_MPLS		= 3,
		ROFL_PKT_CLASSIFIER_PPPOE		= 4,
		ROFL_PKT_CLASSIFIER_PPP			= 5,
		ROFL_PKT_CLASSIFIER_ARPV4		= 6,
		ROFL_PKT_CLASSIFIER_IPV4		= 7,
		ROFL_PKT_CLASSIFIER_ICMPV4		= 8,
		ROFL_PKT_CLASSIFIER_IPV6		= 9,
		ROFL_PKT_CLASSIFIER_ICMPV6		= 10,
		ROFL_PKT_CLASSIFIER_UDP			= 11,
		ROFL_PKT_CLASSIFIER_TCP			= 12,
		ROFL_PKT_CLASSIFIER_SCTP		= 13,
		ROFL_PKT_CLASSIFIER_MAX,
	};

	std::map<enum rofl_pktclassifier_type_t, std::vector<rofl::fframe*> > t_frames;

	virtual unsigned int n_ether() { return t_frames[ROFL_PKT_CLASSIFIER_ETHER ].size(); };
	virtual unsigned int n_vlan()  { return t_frames[ROFL_PKT_CLASSIFIER_VLAN  ].size(); };
	virtual unsigned int n_mpls()  { return t_frames[ROFL_PKT_CLASSIFIER_MPLS  ].size(); };
	virtual unsigned int n_pppoe() { return t_frames[ROFL_PKT_CLASSIFIER_PPPOE ].size(); };
	virtual unsigned int n_ppp()   { return t_frames[ROFL_PKT_CLASSIFIER_PPP   ].size(); };
	virtual unsigned int n_arpv4() { return t_frames[ROFL_PKT_CLASSIFIER_ARPV4 ].size(); };
	virtual unsigned int n_ipv4()  { return t_frames[ROFL_PKT_CLASSIFIER_IPV4  ].size(); };
	virtual unsigned int n_icmpv4(){ return t_frames[ROFL_PKT_CLASSIFIER_ICMPV4].size(); };
	virtual unsigned int n_ipv6()  { return t_frames[ROFL_PKT_CLASSIFIER_IPV6  ].size(); };
	virtual unsigned int n_icmpv6(){ return t_frames[ROFL_PKT_CLASSIFIER_ICMPV6].size(); };
	virtual unsigned int n_udp()   { return t_frames[ROFL_PKT_CLASSIFIER_UDP   ].size(); };
	virtual unsigned int n_tcp()   { return t_frames[ROFL_PKT_CLASSIFIER_TCP   ].size(); };
	virtual unsigned int n_sctp()  { return t_frames[ROFL_PKT_CLASSIFIER_SCTP  ].size(); };

	// ethernet type of payload (=innermost ethernet type)
	uint16_t eth_type; 

	virtual rofl::fframe* frame_insert(rofl::fframe *append_to, rofl::fframe *frame);
	virtual void frame_append(rofl::fframe *frame);
	virtual void frame_push(rofl::fframe *frame);
	virtual void frame_pop(rofl::fframe *frame);

	virtual void parse_ether	(uint8_t *data, size_t datalen);
	virtual void parse_vlan(uint8_t *data, size_t datalen);
	virtual void parse_mpls(uint8_t *data, size_t datalen);
	virtual void parse_pppoe(uint8_t *data, size_t datalen);
	virtual void parse_ppp(uint8_t *data, size_t datalen);
	virtual void parse_arpv4(uint8_t *data, size_t datalen);
	virtual void parse_ipv4	(uint8_t *data, size_t datalen);
	virtual void parse_icmpv4(uint8_t *data, size_t datalen);
	virtual void parse_udp(uint8_t *data, size_t datalen);
	virtual void parse_tcp(uint8_t *data, size_t datalen);
	virtual void parse_sctp	(uint8_t *data, size_t datalen);

};


#endif /* ROFL_PKTCLASSIFIER_H_ */
