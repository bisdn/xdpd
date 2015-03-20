/**
* This is a unit test that must check the proper 
* funcionality of datapacket_storage
*
*/

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>
#include <unistd.h>
#include <thread>
#include "io/bufferpool.h"

#define STORE_SIZE 15
#define EXPIRATION_SEC 3

using namespace std;
using namespace xdpd::gnu_linux;

extern "C"{

void platform_packet_drop(datapacket_t* pkt){};

}


class BufferpoolTestCase : public CppUnit::TestFixture{
	CPPUNIT_TEST_SUITE(BufferpoolTestCase);
	CPPUNIT_TEST(test_basic);
	CPPUNIT_TEST_SUITE_END();
	
	void test_basic(void);
	
	//datapacket_t pkt[STORE_SIZE], *recv;
	//storeid id[STORE_SIZE];
	
public:
	void setUp(void);
	void tearDown(void);
};

void BufferpoolTestCase::setUp(){
	fprintf(stderr,"<%s:%d> ************** BufferpoolTestCase Set up ************\n",__func__,__LINE__);
	bufferpool::init();
}

void BufferpoolTestCase::tearDown(){
	fprintf(stderr,"<%s:%d> ************** BufferpoolTestCase Tear Down ************\n",__func__,__LINE__);
	bufferpool::destroy();
}

void work(){
	datapacket_t *pkt;
	
	pkt = bufferpool::get_buffer();
	fprintf(stderr,"<%s:%d> Buffer recieved pkt = %p \n",__func__,__LINE__, pkt);
	CPPUNIT_ASSERT(pkt!=NULL); //assert that... what?!!
	
	bufferpool::release_buffer(pkt);
	fprintf(stderr,"<%s:%d> Buffer released pkt = %p \n",__func__,__LINE__, pkt);
}

/* Tests */
void BufferpoolTestCase::test_basic(void )
{
	fprintf(stderr,"<%s:%d> ************** BufferpoolTestCase Test Basic ************\n",__func__,__LINE__);
	std::thread first (work);     // spawn new thread that calls work()
	std::thread second (work);
	
	first.join();                // pauses until first finishes
	second.join();               // pauses until second finishes
	
}

/*
* Test MAIN
*/
int main( int argc, char* argv[] )
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(BufferpoolTestCase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );
	
	std::cerr<<"************** Test finished ************"<<std::endl;

	// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
