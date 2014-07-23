/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <unistd.h>
#include <pthread.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "util/circular_queue.h"

using namespace std;
using namespace xdpd::gnu_linux;

#define READERS 3
#define WRITERS 3

int *pool;

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
	static unsigned int randomIterations;
	static const unsigned int SLEEP_TIME_MS=20;	
	static const unsigned int MIN_ITERATIONS=50000;	
	static const unsigned int MAX_ITERATIONS=120000;	
	static circular_queue<datapacket_t>* buffer;

	public:
		void setUp(void);
		void tearDown(void);
};

unsigned int RingBufferTestCase::randomIterations = 0;
circular_queue<datapacket_t>* RingBufferTestCase::buffer = new circular_queue<datapacket_t>(1024);

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
	//buffer = new circular_queue<datapacket_t>(1024);
}

void RingBufferTestCase::tearDown(){
	//delete buffer;
}

/* Test specific methods */
void RingBufferTestCase::bufferFilling(){

	//Fills buffer and checks that it accepts MAX_SLOTS-1
	circular_queue<datapacket_t> buf(1024);
	int ret;

	fprintf(stderr,"MAx slots: %llu\n",buf.slots);

	for(unsigned int i=0;i<buf.slots;i++){
		
		//std::cerr << i << std::endl;

		ret = buf.non_blocking_write(NULL); //Fill
		
		if(i != (buf.slots-1)){
			CPPUNIT_ASSERT(ROFL_SUCCESS == ret);
		}else{
			CPPUNIT_ASSERT(ROFL_FAILURE == ret);
		}	
	}
	std::cerr<<"Size: "<<buf.size()<<std::endl;

	CPPUNIT_ASSERT(buf.size() == buf.slots-1);
}

void* RingBufferTestCase::blockingRead(void* obj){

	int id = ((int*)obj-(int*)NULL);
	unsigned int pkt_id, cnt=0;
	datapacket_t* pkt;

	fprintf(stderr, "#R%u Reading... %u\n", id, randomIterations/READERS);

	//Read up to N and quit
	for(unsigned int i=0; i < (randomIterations/READERS); i++){

		do{
			pkt = buffer->non_blocking_read();
		}while(!pkt);
		
		pkt_id = (unsigned int)(pkt - (datapacket_t*)0x01UL);
		
		//fprintf(stderr,"Read pkt %p %u\n", pkt, pkt_id);	
		
		if(pool[pkt_id] != 1){
			fprintf(stderr,"\n#%u Packet id: %u has value %d instead of 1.\n",id, pkt_id, pool[pkt_id]);
			//Dump status
			for(unsigned int j=0;j<randomIterations;j++){
				fprintf(stderr,"[%3u: %d] %s",j, pool[j], ((j+1)%10 == 0)?"\n":"");
			}
		}

		CPPUNIT_ASSERT(pool[pkt_id] == 1);
		pool[pkt_id] = 2;
	
		//20% prob. sleep
		if(rand()%100 > 80)
			usleep(SLEEP_TIME_MS);	
		cnt++;
	}
	
	fprintf(stderr, "#R%u Read... %u\n", id, cnt);
	
	return NULL;	
}

void* RingBufferTestCase::blockingWrite(void* obj){

	int ret;
	int id = ((int*)obj-(int*)NULL);
	unsigned int cnt=0, from, to;
	
	from = (id*(randomIterations/WRITERS));
	to = ((id+1)*randomIterations/WRITERS);
	fprintf(stderr, "#W%u Writing... %u [%u->%u] \n", id, randomIterations/WRITERS, from, to);
	
	//Write up to N and quit
	for(unsigned int i=from;i<to;i++){

		do{
			pool[i] = 1;
			ret = buffer->non_blocking_write(((datapacket_t*)0x1UL+(uint64_t)i));
		}while(ret != ROFL_SUCCESS);
		

		//20% prob. sleep
		if(rand()%100 > 80)
			usleep(SLEEP_TIME_MS);
		cnt++;
	}
	
	fprintf(stderr, "#W%u Wrote... %u\n", id, cnt);

	return NULL;	
}

void RingBufferTestCase::concurrentAccess(){

	unsigned int i;
	pthread_t writer[WRITERS], reader[READERS];

	//set the random number
	srand(time(NULL));
	randomIterations = ( rand() % (MAX_ITERATIONS-MIN_ITERATIONS) ) + MIN_ITERATIONS; 
	//randomIterations = MIN_ITERATIONS;

	//Normalize
	randomIterations = (randomIterations/READERS);
	randomIterations *=READERS;
	
	fprintf(stderr, "Iterations %u\n", randomIterations);

	pool = (int*)malloc(sizeof(int)*randomIterations);

	CPPUNIT_ASSERT(pool != NULL);

	memset(pool, 0, sizeof(int)*randomIterations);

	//Show #iterations
	for(i=0;i<READERS;i++)
		pthread_create(&reader[i], NULL, RingBufferTestCase::blockingRead, ((int*)0x0+i));
	for(i=0;i<WRITERS;i++)
		pthread_create(&writer[i], NULL, RingBufferTestCase::blockingWrite, ((int*)0x0+i));

	//join them
	for(i=0;i<READERS;i++)
		pthread_join(reader[i],NULL);	
	for(i=0;i<WRITERS;i++)
		pthread_join(writer[i],NULL);	

	for(i=0;i<randomIterations;i++)
		CPPUNIT_ASSERT(pool[i] == 2);

	free(pool);

	//asserts
	CPPUNIT_ASSERT(buffer->size() == 0);
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
