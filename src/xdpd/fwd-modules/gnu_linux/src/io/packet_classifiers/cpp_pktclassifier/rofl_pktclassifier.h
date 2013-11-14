/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ROFL_PKTCLASSIFIER_H
#define ROFL_PKTCLASSIFIER_H 

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
* @file rofl_pktclassifier.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief ROFL-based (C++) data packet classifier 
*
*/

namespace xdpd {
namespace gnu_linux {

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
	template<class T> T* 	header(int idx) const;

	virtual rofl::fetherframe* 	ether(int idx = 0) const;
	virtual rofl::fvlanframe* 	vlan(int idx = 0) 	const;
	virtual rofl::fmplsframe* 	mpls(int idx = 0) 	const;
	virtual rofl::farpv4frame* 	arpv4(int idx = 0) const;
	virtual rofl::fipv4frame* 	ipv4(int idx = 0) 	const;
	virtual rofl::ficmpv4frame* icmpv4(int idx = 0) const;
	virtual rofl::fipv6frame* 	ipv6(int idx = 0) 	const;
	virtual rofl::ficmpv6frame* icmpv6(int idx = 0) const;
	virtual rofl::fudpframe* 	udp(int idx = 0) 	const;
	virtual rofl::ftcpframe* 	tcp(int idx = 0) 	const;
	virtual rofl::fsctpframe* 	sctp(int idx = 0) 	const;
	virtual rofl::fpppoeframe* 	pppoe(int idx = 0) const;
	virtual rofl::fpppframe* 	ppp(int idx = 0) 	const;

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

	unsigned int n_ether() { return t_frames[ROFL_PKT_CLASSIFIER_ETHER ].size(); };
	unsigned int n_vlan()  { return t_frames[ROFL_PKT_CLASSIFIER_VLAN  ].size(); };
	unsigned int n_mpls()  { return t_frames[ROFL_PKT_CLASSIFIER_MPLS  ].size(); };
	unsigned int n_pppoe() { return t_frames[ROFL_PKT_CLASSIFIER_PPPOE ].size(); };
	unsigned int n_ppp()   { return t_frames[ROFL_PKT_CLASSIFIER_PPP   ].size(); };
	unsigned int n_arpv4() { return t_frames[ROFL_PKT_CLASSIFIER_ARPV4 ].size(); };
	unsigned int n_ipv4()  { return t_frames[ROFL_PKT_CLASSIFIER_IPV4  ].size(); };
	unsigned int n_icmpv4(){ return t_frames[ROFL_PKT_CLASSIFIER_ICMPV4].size(); };
	unsigned int n_ipv6()  { return t_frames[ROFL_PKT_CLASSIFIER_IPV6  ].size(); };
	unsigned int n_icmpv6(){ return t_frames[ROFL_PKT_CLASSIFIER_ICMPV6].size(); };
	unsigned int n_udp()   { return t_frames[ROFL_PKT_CLASSIFIER_UDP   ].size(); };
	unsigned int n_tcp()   { return t_frames[ROFL_PKT_CLASSIFIER_TCP   ].size(); };
	unsigned int n_sctp()  { return t_frames[ROFL_PKT_CLASSIFIER_SCTP  ].size(); };

	// ethernet type of payload (=innermost ethernet type)
	uint16_t eth_type; 

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

};

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* ROFL_PKTCLASSIFIER_H_ */
