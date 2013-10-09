#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

//Include circular_queue<datapacket_t,SLOTS>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <unistd.h>
#include <pthread.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "util/circular_queue.h"

using namespace std;
using namespace xdpd::gnu_linux;

#define SLOTS 1024

class RingBufferTestCase : public CppUnit::TestCase{

	CPPUNIT_TEST_SUITE(RingBufferTestCase);
	CPPUNIT_TEST(bufferFilling);
	CPPUNIT_TEST(concurrentAccess);
	CPPUNIT_TEST_SUITE_END();

	//Test methods
	void bufferFilling(void);
	void concurrentAccess(void);

	//Other methods
	static void* blockingRead(void* obj);
	static void* blockingWrite(void* obj);

	//Suff
	unsigned int randomIterations;
	static const unsigned int SLEEP_TIME_MS=200;	
	static const unsigned int MIN_ITERATIONS=5000;	
	static const unsigned int MAX_ITERATIONS=12000;	
	circular_queue<datapacket_t,SLOTS> buffer;

	public:
		void setUp(void);
		void tearDown(void);
};

/* Other CPPUnit stuff */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( RingBufferTestCase, "RingBufferTestCase" );

CppUnit::Test* suite(){
	CppUnit::TestFactoryRegistry &registry =
			  CppUnit::TestFactoryRegistry::getRegistry();

	registry.registerFactory(
	  &CppUnit::TestFactoryRegistry::getRegistry( "RingBufferTestCase" ) );
	return registry.makeTest();
}

/* Setup and tear down */
void RingBufferTestCase::setUp(){


}

void RingBufferTestCase::tearDown(){


}

/* Test specific methods */
void RingBufferTestCase::bufferFilling(){

	//Fills buffer and checks that it accepts MAX_SLOTS-1
	circular_queue<datapacket_t,SLOTS> buf;
	int ret;

	fprintf(stderr,"MAx slots: %llu\n",buf.MAX_SLOTS);

	for(unsigned int i=0;i<buf.MAX_SLOTS;i++){
		
		//std::cerr << i << std::endl;

		ret = buf.non_blocking_write(NULL); //Fill
		
		if(i != (buf.MAX_SLOTS-1)){
			CPPUNIT_ASSERT(ROFL_SUCCESS == ret);
		}else{
			CPPUNIT_ASSERT(ROFL_FAILURE == ret);
		}	
	}
	std::cerr<<"Size: "<<buf.size()<<std::endl;

	CPPUNIT_ASSERT(buf.size() == buf.MAX_SLOTS-1);
}

void* RingBufferTestCase::blockingRead(void* obj){

	datapacket_t* pkt;
	RingBufferTestCase* test = (RingBufferTestCase*)obj;

	cerr << "Reading..." << test->randomIterations << endl;

	//Read up to N and quit
	for(unsigned int i=0;i<test->randomIterations;i++){

		pkt = test->buffer.blocking_read();
		
		//cerr<<"Read ["<<i<<","<<pkt<<"]\n";	
		CPPUNIT_ASSERT(pkt != NULL);
		CPPUNIT_ASSERT(pkt == ((datapacket_t*)0x1)+i);
		
		//20% prob. sleep
		if(rand()%100 > 80)
			usleep(test->SLEEP_TIME_MS);	
	}
	
	return NULL;	
}

void* RingBufferTestCase::blockingWrite(void* obj){

	int ret;
	RingBufferTestCase* test = (RingBufferTestCase*)obj;

	cerr << "Writing..." << test->randomIterations << endl;
	
	//Write up to N and quit
	for(unsigned int i=0;i<test->randomIterations;i++){

		//cerr<<"Writing"<<i<<"\n";	
		ret = test->buffer.blocking_write(((datapacket_t*)0x1)+i);
		//ret = test->buffer.blocking_write(NULL);
		
		CPPUNIT_ASSERT(ret == ROFL_SUCCESS);
		
		//20% prob. sleep
		if(rand()%100 > 80)
			usleep(test->SLEEP_TIME_MS);	
	}

	return NULL;	
}

void RingBufferTestCase::concurrentAccess(){

	pthread_t writer, reader;


	//set the random number
	srand(time(NULL));
	randomIterations = ( rand() % (MAX_ITERATIONS-MIN_ITERATIONS) ) + MIN_ITERATIONS; 

	//Show #iterations
	//TODO
	//Launch two threads that read and write async	
	pthread_create(&reader,NULL, RingBufferTestCase::blockingRead, this);
	pthread_create(&writer,NULL, RingBufferTestCase::blockingWrite, this);

	//join them
	pthread_join(reader,NULL);	
	pthread_join(writer,NULL);	

	//asserts
	CPPUNIT_ASSERT(buffer.size() == 0);
	CPPUNIT_ASSERT(buffer.get_queue_state() == RB_BUFFER_AVAILABLE);
}


/*
* Test MAIN
*/
int main( int argc, char* argv[] ){

	// if command line contains "-selftest" then this is the post build check
	// => the output must be in the compiler error format.
	bool selfTest = (argc > 1) && (std::string("-selftest") == argv[1]);

	CppUnit::TextUi::TestRunner runner;
	runner.addTest( suite() );   // Add the top suite to the test runner

	if ( selfTest ){ 
		// Change the default outputter to a compiler error format outputter
		// The test runner owns the new outputter.
		runner.setOutputter( CppUnit::CompilerOutputter::defaultOutputter(
							    &runner.result(),
							    std::cerr ) );
	}

	// Run the test.
	bool wasSucessful = runner.run( "" );

	// Return error code 1 if any tests failed.
	return wasSucessful ? 0 : 1;
}
