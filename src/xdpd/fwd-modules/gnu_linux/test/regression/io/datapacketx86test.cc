/*
 * datapack.headers->tx86test.cc
 *
 *  Created on: 05.01.2013
 *      Author: andreas
 */

#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <rofl/common/protocols/fetherframe.h>
#include <rofl/common/protocols/fvlanframe.h>
#include <rofl/common/protocols/fmplsframe.h>
#include <rofl/common/protocols/fpppoeframe.h>
#include <rofl/common/protocols/fpppframe.h>
#include <rofl/common/protocols/fipv4frame.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "io/datapacketx86.h"


using namespace rofl;
using namespace std;
using namespace xdpd::gnu_linux;


class DataPacketX86Test : public CppUnit::TestFixture {
private:
	// push/pop operations
	cmemory mLeft; // memory buffer received/sent from/to left site
	cmemory mRight; // memory buffer received/sent from/to right site

	datapacketx86 pack; // data packet for manipulation

public:
	void setUp(void);
	void tearDown(void);

	void testPushPPPoE();
	void testPopPPPoE();

	CPPUNIT_TEST_SUITE(DataPacketX86Test);
	// CPPUNIT_TEST(testPushPPPoE);
	CPPUNIT_TEST(testPopPPPoE);
	CPPUNIT_TEST_SUITE_END();
};


void DataPacketX86Test::setUp()
{
	unsigned int right_num_bytes =
			sizeof(struct fetherframe::eth_hdr_t) + // ethernet
			sizeof(struct fvlanframe::vlan_hdr_t) + // vlan tag
			sizeof(struct fipv4frame::ipv4_hdr_t) + // ipv4 header
			sizeof(struct fudpframe::udp_hdr_t) + // udp header
			sizeof(uint8_t) * 16; // 16 bytes of payload

	/*
	 * pack.headers->t on the right
	 */
	mRight.resize(right_num_bytes);

	unsigned int idx = 0;
	// ethernet destination mac address
	mRight[idx++] = 0x00;
	mRight[idx++] = 0x11;
	mRight[idx++] = 0x11;
	mRight[idx++] = 0x11;
	mRight[idx++] = 0x11;
	mRight[idx++] = 0x11;
	// ethernet source mac address
	mRight[idx++] = 0x00;
	mRight[idx++] = 0x22;
	mRight[idx++] = 0x22;
	mRight[idx++] = 0x22;
	mRight[idx++] = 0x22;
	mRight[idx++] = 0x22;
	// ethernet type: VLAN tag
	mRight[idx++] = 0x81;
	mRight[idx++] = 0x00;

	// vlan vid + pcp
	mRight[idx++] = 0x77;
	mRight[idx++] = 0x77;
	// vlan ethernet type: ipv4 (=0x0800)
	mRight[idx++] = 0x08;
	mRight[idx++] = 0x00;

	// ipv4: version + ihl
	mRight[idx++] = 0x45;
	mRight[idx++] = 0x00;
	// ipv4: dscp+ecn
	mRight[idx++] = 0x00;
	// ipv4: total length (here: 20 bytes header + 8 bytes UDP + 16 bytes payload)
	mRight[idx++] = 0x2c;
	// ipv4: identification
	mRight[idx++] = 0x55;
	mRight[idx++] = 0x55;
	// ipv4: flags + offset
	mRight[idx++] = 0x00;
	mRight[idx++] = 0x00;
	// ipv4: ttl (here: 64)
	mRight[idx++] = 0x40;
	// ipv4: protocol (here: UDP [17==0x11])
	mRight[idx++] = 0x11;
	// ipv4: hdr checksum
	mRight[idx++] = 0x00;
	mRight[idx++] = 0x00;
	// ipv4: src IP address (here: 10.1.1.1)
	mRight[idx++] = 0x0a;
	mRight[idx++] = 0x01;
	mRight[idx++] = 0x01;
	mRight[idx++] = 0x01;
	// ipv4: dst IP address (here: 10.2.2.2)
	mRight[idx++] = 0x0a;
	mRight[idx++] = 0x02;
	mRight[idx++] = 0x02;
	mRight[idx++] = 0x02;

	// udp: src port (here: 0x3333)
	mRight[idx++] = 0x33;
	mRight[idx++] = 0x33;
	// udp: dst port (here: 0x4444)
	mRight[idx++] = 0x44;
	mRight[idx++] = 0x44;
	// udp: length (header + data, here: 16 bytes)
	mRight[idx++] = 0x00;
	mRight[idx++] = 0x10;
	// udp: checksum
	mRight[idx++] = 0xcc;
	mRight[idx++] = 0xcc;

	// payload
	for (int i = 0; i < 16; i++){
		mRight[idx++] = 0x99;
	}

	//fprintf(stderr, "mRight: %s\n", mRight.c_str());

	/*
	 * pack.headers->t on the left
	 */
	unsigned int left_num_bytes =
			sizeof(struct fetherframe::eth_hdr_t) + // ethernet
			sizeof(struct fpppoeframe::pppoe_hdr_t) + // pppoe tag
			sizeof(struct fpppframe::ppp_hdr_t) + // ppp tag
			sizeof(struct fipv4frame::ipv4_hdr_t) + // ipv4 header
			sizeof(struct fudpframe::udp_hdr_t) + // udp header
			sizeof(uint8_t) * 16; // 16 bytes of payload

	mLeft.resize(left_num_bytes);

	idx = 0;
	// ethernet destination mac address
	mLeft[idx++] = 0x00;
	mLeft[idx++] = 0x33;
	mLeft[idx++] = 0x33;
	mLeft[idx++] = 0x33;
	mLeft[idx++] = 0x33;
	mLeft[idx++] = 0x33;
	// ethernet source mac address
	mLeft[idx++] = 0x00;
	mLeft[idx++] = 0x44;
	mLeft[idx++] = 0x44;
	mLeft[idx++] = 0x44;
	mLeft[idx++] = 0x44;
	mLeft[idx++] = 0x44;
	// ethernet type: pppoe session tag
	mLeft[idx++] = 0x88;
	mLeft[idx++] = 0x64;

	// pppoe version + type
	mLeft[idx++] = 0x11;
	// pppoe code
	mLeft[idx++] = 0x00;
	// pppoe sessid
	mLeft[idx++] = 0xaa;
	mLeft[idx++] = 0xaa;
	// pppoe length
	mLeft[idx++] = 0x00;
	mLeft[idx++] = 0x2e;
	// ppp code (here: IPv4)
	mLeft[idx++] = 0x00;
	mLeft[idx++] = 0x21;

	// ipv4: version + ihl
	mLeft[idx++] = 0x45;
	mLeft[idx++] = 0x00;
	// ipv4: dscp+ecn
	mLeft[idx++] = 0x00;
	// ipv4: total length (here: 20 bytes header + 8 bytes UDP header + 16 bytes payload)
	mLeft[idx++] = 0x2c;
	// ipv4: identification
	mLeft[idx++] = 0x55;
	mLeft[idx++] = 0x55;
	// ipv4: flags + offset
	mLeft[idx++] = 0x00;
	mLeft[idx++] = 0x00;
	// ipv4: ttl (here: 64)
	mLeft[idx++] = 0x40;
	// ipv4: protocol (here: UDP [17==0x11])
	mLeft[idx++] = 0x11;
	// ipv4: hdr checksum
	mLeft[idx++] = 0x00;
	mLeft[idx++] = 0x00;
	// ipv4: src IP address (here: 10.1.1.1)
	mLeft[idx++] = 0x0a;
	mLeft[idx++] = 0x01;
	mLeft[idx++] = 0x01;
	mLeft[idx++] = 0x01;
	// ipv4: dst IP address (here: 10.2.2.2)
	mLeft[idx++] = 0x0a;
	mLeft[idx++] = 0x02;
	mLeft[idx++] = 0x02;
	mLeft[idx++] = 0x02;

	// udp: src port (here: 0x3333)
	mLeft[idx++] = 0x33;
	mLeft[idx++] = 0x33;
	// udp: dst port (here: 0x4444)
	mLeft[idx++] = 0x44;
	mLeft[idx++] = 0x44;
	// udp: length (header + data, here: 16 bytes)
	mLeft[idx++] = 0x00;
	mLeft[idx++] = 0x10;
	// udp: checksum
	mLeft[idx++] = 0xcc;
	mLeft[idx++] = 0xcc;

	// payload
	for (int i = 0; i < 16; i++){
		mLeft[idx++] = 0x99;
	}

	//fprintf(stderr, "mLeft: %s\n", mLeft.c_str());
};



void DataPacketX86Test::tearDown()
{

};


void DataPacketX86Test::testPushPPPoE()
{
	pack.init(mRight.somem(), mRight.memlen(), NULL, 1, 1, true);

	pack.headers->classify();
	pack.headers->pop_vlan();
	pack.headers->ether(0)->set_dl_dst(cmacaddr("00:33:33:33:33:33"));
	pack.headers->ether(0)->set_dl_src(cmacaddr("00:44:44:44:44:44"));

	pack.headers->push_pppoe(fpppoeframe::PPPOE_ETHER_SESSION);

	pack.headers->pppoe(0)->set_pppoe_code(0x0000);
	pack.headers->pppoe(0)->set_pppoe_sessid(0xaaaa);
	pack.headers->pppoe(0)->set_pppoe_type(fpppoeframe::PPPOE_TYPE);
	pack.headers->pppoe(0)->set_pppoe_vers(fpppoeframe::PPPOE_VERSION);
	pack.headers->pppoe(0)->set_hdr_length(pack.headers->ipv4(0)->get_ipv4_length() + sizeof(fpppframe::ppp_hdr_t));
	pack.headers->ppp(0)->set_ppp_prot(fpppframe::PPP_PROT_IPV4);

	rofl::cmemory mResult(pack.get_buffer(), pack.get_buffer_length());

	CPPUNIT_ASSERT(mLeft == mResult);

	pack.destroy();
};


void DataPacketX86Test::testPopPPPoE()
{
	pack.init(mLeft.somem(), mLeft.memlen(), NULL, 1, 1, true);

	pack.headers->classify();

	pack.headers->pop_pppoe(rofl::fipv4frame::IPV4_ETHER);

	pack.headers->ether(0)->set_dl_dst(cmacaddr("00:11:11:11:11:11"));
	pack.headers->ether(0)->set_dl_src(cmacaddr("00:22:22:22:22:22"));

	pack.headers->push_vlan(rofl::fvlanframe::VLAN_CTAG_ETHER);
	pack.headers->vlan(0)->set_dl_vlan_cfi(true);
	pack.headers->vlan(0)->set_dl_vlan_id(0x777);
	pack.headers->vlan(0)->set_dl_vlan_pcp(0x3);

	rofl::cmemory mResult(pack.get_buffer(), pack.get_buffer_length());

	CPPUNIT_ASSERT(mRight == mResult);

	pack.destroy();
};


int main(int argc, char** argv)
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest( DataPacketX86Test::suite() );
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the tests.
	bool wasSucessful = runner.run();

	// Return error code 1 if the one of test failed.
	return wasSucessful ? 0 : 1;
}
