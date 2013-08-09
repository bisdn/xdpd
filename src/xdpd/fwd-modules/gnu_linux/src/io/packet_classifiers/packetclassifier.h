/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PACKETCLASSIFIER_H
#define PACKETCLASSIFIER_H

#include <bitset>
#include <inttypes.h>
#include <sys/types.h>

#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/common/cmemory.h>
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
#include <rofl/datapath/pipeline/platform/memory.h>
//#include <rofl/datapath/pipeline/util/rofl_pipeline_utils.h>

/**
* @file packetclassifier.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief Pure abstract packet classifier class.
*
* All the packet classifiers associated with datapacketx86
* should be compliant with this interface.
*
*/

//fwd declarations (avoid circular dependencies)
class datapacketx86;

class packetclassifier{

public:
	//Constructor&destructor
	packetclassifier(datapacketx86* pkt_ref):pkt(pkt_ref){}; 
	virtual ~packetclassifier(){}; 

	/*
	* Main classification methods. 
	*/
	virtual void classify(void)=0;
	virtual void classify_reset(void)=0;

	/*
	* header access
	*/

	virtual rofl::fetherframe* ether(int idx = 0) const=0;
	virtual rofl::fvlanframe* vlan(int idx = 0) const=0;
	virtual rofl::fmplsframe* mpls(int idx = 0) const=0;
	virtual rofl::farpv4frame* arpv4(int idx = 0) const=0;
	virtual rofl::fipv4frame* ipv4(int idx = 0) const=0;
	virtual rofl::ficmpv4frame* icmpv4(int idx = 0) const=0;
	virtual rofl::fipv6frame* ipv6(int idx = 0) const=0;
	virtual rofl::ficmpv6frame* icmpv6(int idx = 0) const=0;
	virtual rofl::fudpframe* udp(int idx = 0) const=0;
	virtual rofl::ftcpframe* tcp(int idx = 0) const=0;
	virtual rofl::fsctpframe* sctp(int idx = 0) const=0;
	virtual rofl::fpppoeframe* pppoe(int idx = 0) const=0;
	virtual rofl::fpppframe* ppp(int idx = 0) const=0;
	virtual rofl::fgtpuframe* gtp(int idx = 0) const=0;

	/*
	 * pop operations
	 */
	virtual void pop_vlan(void)=0;
	virtual void pop_mpls(uint16_t ether_type)=0;
	virtual void pop_pppoe(uint16_t ether_type)=0;

	/*
	 * push operations
	 */
	virtual rofl::fvlanframe* push_vlan(uint16_t ether_type)=0;
	virtual rofl::fmplsframe* push_mpls(uint16_t ether_type)=0;
	virtual rofl::fpppoeframe* push_pppoe(uint16_t ether_type)=0;
	
	/*
	* dump
	*/
	virtual void dump(void)=0;

	/** returns length of packet starting at 'fframe' from up to including fframe 'to'
	 *
	 */
	virtual size_t
	get_pkt_len(
			rofl::fframe *from = (rofl::fframe*)0,
			rofl::fframe   *to = (rofl::fframe*)0) = 0;

protected:

	//Datapacket reference
	datapacketx86* pkt;

	/*
	* Wrappers for pkt push and pop so that we can use friendship in derived classes
	*/
	rofl_result_t pkt_push(unsigned int num_of_bytes, unsigned int offset=0);
	rofl_result_t pkt_pop(unsigned int num_of_bytes, unsigned int offset=0);

	rofl_result_t pkt_push(uint8_t* push_point, unsigned int num_of_bytes);
	rofl_result_t pkt_pop(uint8_t* pop_point, unsigned int num_of_bytes);
	
	
};



#endif /* PACKETCLASSIFIER_H_ */
