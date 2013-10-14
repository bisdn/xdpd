/*
 * mmap_port.cc
 *
 *  Created on: Jan 7, 2013
 *      Author: tobi
 */

#include <sys/socket.h>
#include <string.h>

#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rofl/common/protocols/fetherframe.h>
#include <rofl/common/protocols/fvlanframe.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include "io/ports/mmap/ioport_mmap.h"
#include "io/bufferpool.h"

using namespace std;
using namespace xdpd::gnu_linux;

class MMAPPortTest: public CppUnit::TestFixture
{
	// create the suite
	CPPUNIT_TEST_SUITE(MMAPPortTest);
	CPPUNIT_TEST(mmap_single_read_test);
	CPPUNIT_TEST(mmap_single_write_test);
	CPPUNIT_TEST(mmap_single_read_vlan_test);
	// todo test enable port with send/recv packets
	// todo test disableport with send/recv packets
//	CPPUNIT_TEST(mmap_single_write_vlan_test); // fixme does not work with socket
	CPPUNIT_TEST_SUITE_END();

	// single read test
	void mmap_single_read_test(void);

	// single write test
	void mmap_single_write_test(void);

	// single read test with vlan
	void mmap_single_read_vlan_test(void);

	// single write test with vlan
	void mmap_single_write_vlan_test(void);

public:
	void setUp(void);
	void tearDown(void);

private:
	switch_port_t* of_port_state;	
	ioport_mmap *port;
	int sd;
	datapacket_t* pkt;
	datapacket_t* pkt_vlan;
};

#define PKT_SIZE 1400

/* Setup and tear down */
void
MMAPPortTest::setUp()
{
	int rc = 0;
	struct ifreq ifr;
	sd = -1;
	struct sockaddr_ll bind_addr;

	/* init send/recv port */
	sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sd < 0) {
		// Error
		fprintf(stderr, "Error in socket() creation - %s (are you root?)\n", strerror(errno));
		exit(-1);
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "veth1", sizeof(ifr.ifr_name));

	// read flags from interface
	if ((rc = ioctl(sd, SIOCGIFFLAGS, &ifr)) < 0) {
		fprintf(stderr, "Error in ioctl() call for SIOCGIFFLAGS - %s\n", strerror(errno));
		fprintf(stderr, "currently veth0 and veth1 have to be created manually :(\n");
		exit(-1);
	}

	// enable promiscuous flags
	ifr.ifr_flags |= IFF_PROMISC;
	if ((rc = ioctl(sd, SIOCSIFFLAGS, &ifr)) < 0) {
		fprintf(stderr, "Error in ioctl() call for SIOCSIFFLAGS - %s\n", strerror(errno));
		exit(-1);
	}

//
//	// todo set SO_SNDBUF?
//	int optval = 1600;
//	if ((rc = setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (int*)&optval, sizeof(optval))) < 0)
//		throw eSocketError();
//

	memset(&bind_addr, 0, sizeof(bind_addr));
	if ((rc = ioctl(sd, SIOCGIFINDEX, &ifr)) < 0) {
		fprintf(stderr, "Error in ioctl() call for SIOCSIFFLAGS - %s\n", strerror(errno));
		exit(-1);
	}

	// setup params
	bind_addr.sll_family 	= AF_PACKET;
	bind_addr.sll_protocol 	= htons(ETH_P_ALL);
	bind_addr.sll_ifindex 	= ifr.ifr_ifindex;
	bind_addr.sll_hatype 	= 0;
	bind_addr.sll_pkttype 	= 0;
	bind_addr.sll_halen 	= 0;

	// bind to local address
	if ((rc = bind(sd, (struct sockaddr *)&bind_addr, sizeof(struct sockaddr_ll))) < 0) {
		switch (rc) {
		case EADDRINUSE:
			fprintf(stderr, "Error in bind() EADDRINUSE - %s\n", strerror(errno));
			exit(-1);
		default:
			fprintf(stderr, "Error in bind() other - %s\n", strerror(errno));
			exit(-1);
		}
	}

	/* init tested port */
	of_port_state = switch_port_init((char*)"veth0", true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE);
	CPPUNIT_ASSERT(NULL != of_port_state);

	port = new ioport_mmap(of_port_state);

	port->enable();

	// Init bufferpool
	bufferpool::init(2048);
	datapacketx86* pkt_x86;

	// Allocate free buffer
	pkt = bufferpool::get_free_buffer();
	pkt_x86 = ((datapacketx86*)pkt->platform_state);

	// create the packet contents
	pkt_x86->init(NULL, 250, NULL,1);
	pkt_x86->headers->classify();

	pkt_x86->headers->ether(0)->set_dl_dst(rofl::cmacaddr("00:11:11:11:11:11"));
	pkt_x86->headers->ether(0)->set_dl_src(rofl::cmacaddr("00:22:22:22:22:22"));
	pkt_x86->headers->ether(0)->set_dl_type(0x0800);

	// Allocate free buffer
	pkt_vlan = bufferpool::get_free_buffer();
	pkt_x86 = ((datapacketx86*)pkt_vlan->platform_state);

	// create the packet contents
	pkt_x86->init(NULL, 254, NULL,1);
	pkt_x86->headers->classify();

	pkt_x86->headers->ether(0)->set_dl_dst(rofl::cmacaddr("00:11:11:11:11:11"));
	pkt_x86->headers->ether(0)->set_dl_src(rofl::cmacaddr("00:22:22:22:22:22"));
	pkt_x86->headers->ether(0)->set_dl_type(ETH_P_8021Q);
	pkt_x86->headers->classify();

	pkt_x86->headers->vlan(0)->set_dl_vlan_id(16);
	pkt_x86->headers->vlan(0)->set_dl_vlan_pcp(0x2);
	// pkt_x86->headers->vlan(0)->set_dl_vlan_cfi(true); // todo this fails if it is set (fail in mmap?)
	pkt_x86->headers->vlan(0)->set_dl_type(ETH_P_IP);
	// todo could fill in also ip layer data + payload
	
	//add queues
	unsigned int i;
	char queue_name[PORT_QUEUE_MAX_LEN_NAME];
	for(i=0;i<port->get_num_of_queues();i++){
		snprintf(queue_name, PORT_QUEUE_MAX_LEN_NAME, "%s%d", "queue", i);
		switch_port_add_queue(of_port_state,i,(char*)&queue_name, port->get_queue_size(i), 0, 0);
	}
}

void
MMAPPortTest::tearDown()
{
	// close socket
	if (-1 != sd) {
		close(sd);
	}

	if (NULL != pkt) {
		bufferpool::release_buffer(pkt);
	}
	if (NULL != pkt_vlan) {
		bufferpool::release_buffer(pkt_vlan);
	}
	delete port;
}

/* Test specific */
void
MMAPPortTest::mmap_single_read_test()
{
	datapacketx86* pkt_x86;
	pkt_x86 = ((datapacketx86*)pkt->platform_state);

	//printf("size = %zu\n", pkt_x86->headers->get_buffer_length());

	int rv;

	rv = send(sd, pkt_x86->get_buffer(), PKT_SIZE, 0);

//	if (-1 == rv) {
//		fprintf(stderr, "Error in send() - %s\n", strerror(errno));
//	} else {
//		printf("%d bytes send\n", rv);
//	}

	// reschedule
	usleep(0);

	rv = port->read_loop(port->get_read_fd(), 10);

	CPPUNIT_ASSERT( (1 == rv) );
	// fprintf(stderr, "read #packets=%d #bytes=%d\n", port->get_rx_count(), port->get_rx_bytes());

	// read the packet
	datapacket_t *rpkt = port->read();
	CPPUNIT_ASSERT(NULL != rpkt);
	datapacketx86* rpkt_x86 = (datapacketx86*)rpkt->platform_state;

	// check for correct length
	CPPUNIT_ASSERT(PKT_SIZE == rpkt_x86->get_buffer_length());

	// check the content
	CPPUNIT_ASSERT(0 == memcmp(pkt_x86->get_buffer(), rpkt_x86->get_buffer(), PKT_SIZE));

	// free the resource
	bufferpool::release_buffer(rpkt);
}

/* single write test */
void
MMAPPortTest::mmap_single_write_test()
{
	//int rv;
	unsigned int buckets_left;
	datapacketx86* pkt_x86;
	pkt_x86 = ((datapacketx86*)pkt->platform_state);

	/* get another buffer for sending, because the send packet will be consumed */
	datapacket_t *send_pkt = bufferpool::get_free_buffer();
	datapacketx86* send_x86_pkt = ((datapacketx86*)send_pkt->platform_state);
	send_x86_pkt->init(pkt_x86->get_buffer(), PKT_SIZE, NULL, 1, 0,true);

	// write a single packet to the output queue
	port->enqueue_packet(send_pkt, 0);

	// schedule a single packet from queue 0
	buckets_left = port->write(0, 1);

	// no tokens should be left
	CPPUNIT_ASSERT(0 == buckets_left);

	// reschedule
	usleep(0);

	// Allocate free buffer
	datapacket_t *rpkt = bufferpool::get_free_buffer();
	CPPUNIT_ASSERT(NULL != rpkt);

	datapacketx86 *rpkt_x86 = ((datapacketx86*)rpkt->platform_state);
	rpkt_x86->init(rpkt_x86->get_buffer(), PKT_SIZE, NULL, 1, 0,true);

	// read into rpkt buffer
	size_t read_bytes = read(sd, rpkt_x86->get_buffer(), PKT_SIZE);

	std::cout << "read_bytes: " << read_bytes << std::endl;

	// check the size
	CPPUNIT_ASSERT(read_bytes == send_x86_pkt->get_buffer_length());

	// check the contents of the packet
	CPPUNIT_ASSERT(0 == memcmp(rpkt_x86->get_buffer(), pkt_x86->get_buffer(), rpkt_x86->get_buffer_length()));

	bufferpool::release_buffer(rpkt);
}


/* Test specific */
void
MMAPPortTest::mmap_single_read_vlan_test()
{
	datapacketx86* pkt_x86;
	pkt_x86 = ((datapacketx86*)pkt_vlan->platform_state);

	// printf("size = %zu\n", pkt_x86->get_buffer_length());

	int rv;

	rv = send(sd, pkt_x86->get_buffer(), PKT_SIZE, 0);

//	if (-1 == rv) {
//		fprintf(stderr, "Error in send() - %s\n", strerror(errno));
//	} else {
//		printf("%d bytes send\n", rv);
//	}

	// reschedule
	usleep(0);

	rv = port->read_loop(port->get_read_fd(), 10);

	CPPUNIT_ASSERT(1 == rv);

	// fprintf(stderr, "read #packets=%d #bytes=%d\n", port->get_rx_count(), port->get_rx_bytes());

	// read the packet
	datapacket_t *rpkt = port->read();
	CPPUNIT_ASSERT(NULL != rpkt);
	datapacketx86* rpkt_x86 = (datapacketx86*)rpkt->platform_state;

	// check for correct length
	CPPUNIT_ASSERT(PKT_SIZE == rpkt_x86->get_buffer_length());

//	pkt_x86->headers->dump();
//	rpkt_x86->headers->dump();

	// check the content
	CPPUNIT_ASSERT(0 == memcmp(pkt_x86->get_buffer(), rpkt_x86->get_buffer(), rpkt_x86->get_buffer_length()));

	// free the resource
	bufferpool::release_buffer(rpkt);
}

/* single write test */
void
MMAPPortTest::mmap_single_write_vlan_test()
{
	//int rv;
	unsigned int buckets_left;
	datapacketx86* pkt_x86;
	pkt_x86 = ((datapacketx86*)pkt_vlan->platform_state);

	datapacket_t *send_pkt = bufferpool::get_free_buffer();
	datapacketx86* send_x86_pkt = ((datapacketx86*)send_pkt->platform_state);
	send_x86_pkt->init(pkt_x86->get_buffer(), PKT_SIZE, NULL, 1, 0, true);

	// write a single packet to the output queue
	port->enqueue_packet(send_pkt, 0);

	// schedule a single packet from queue 0
	buckets_left = port->write(0, 1);

	// no tokens should be left
	CPPUNIT_ASSERT(0 == buckets_left);

	// reschedule
	usleep(0);

	// Allocate free buffer
	datapacket_t *rpkt = bufferpool::get_free_buffer();
	CPPUNIT_ASSERT(NULL != rpkt);

	datapacketx86 *rpkt_x86 = ((datapacketx86*)rpkt->platform_state);
	rpkt_x86->init(rpkt_x86->get_buffer(), PKT_SIZE, NULL, 1, 0,true);

	// read into rpkt buffer
	size_t read_bytes = read(sd, rpkt_x86->get_buffer(), rpkt_x86->get_buffer_length());

	// check the size
	CPPUNIT_ASSERT(read_bytes == send_x86_pkt->get_buffer_length());

	// check the contents of the packet
	CPPUNIT_ASSERT(0 == memcmp(rpkt_x86->get_buffer(), pkt_x86->get_buffer(), rpkt_x86->get_buffer_length()));
}

/*
 * Test MAIN
 */
int
main(int argc, char* argv[])
{
	CppUnit::TextTestRunner runner;
	runner.addTest(MMAPPortTest::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr)
	);

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run();

// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
