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
#include <time.h>
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
	
public:
	void setUp(void);
	void tearDown(void);
};

void BufferpoolTestCase::setUp(){
	fprintf(stderr,"<%s:%d> ************** BufferpoolTestCase Set up ************\n",__func__,__LINE__);
}

void BufferpoolTestCase::tearDown(){
	fprintf(stderr,"<%s:%d> ************** BufferpoolTestCase Tear Down ************\n",__func__,__LINE__);
	bufferpool::destroy();
}

void work(uint16_t id){
	datapacket_t *pkt;
	uint64_t i;
	fprintf(stderr,"<%s:%d> [%u] starting\n",__func__,__LINE__, id);
	for(i=0; i<0x10000; i++){
		pkt = bufferpool::get_buffer();
		
		CPPUNIT_ASSERT(pkt != NULL);
		
		pkt->__cookie = id;
		
		//fprintf(stderr,"<%s:%d> [%d] Buffer recieved pkt = %p \n",__func__,__LINE__, id, pkt);
		
		usleep(rand() % 20 + 1);
		
		CPPUNIT_ASSERT(pkt->__cookie==id);
		
		bufferpool::release_buffer(pkt);
		//fprintf(stderr,"<%s:%d> [%d] Buffer released pkt = %p \n",__func__,__LINE__, id, pkt);
	}
	fprintf(stderr,"<%s:%d> [%u] finished\n",__func__,__LINE__, id);
}

/* Tests */
void BufferpoolTestCase::test_basic(void )
{
	bufferpool::init();
	srand (time(NULL));
	
	fprintf(stderr,"<%s:%d> ************** BufferpoolTestCase Test Basic ************\n",__func__,__LINE__);
	std::thread fir (work, 1);     // spawn new thread that calls work()
	std::thread sec (work, 2);
	std::thread thi (work, 3);
	std::thread fou (work, 4);
	std::thread fiv (work, 5);
	
	
	
	fir.join();                // pauses until first finishes
	sec.join();               // pauses until second finishes
	thi.join();
	fou.join();
	fiv.join();
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
