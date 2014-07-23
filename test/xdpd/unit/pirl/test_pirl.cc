/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
#include "xdpd/openflow/pirl/pirl.h"

using namespace std;
using namespace xdpd;

#define MAX_RATE_S 2000

class PIRLTestCase : public CppUnit::TestFixture{
	CPPUNIT_TEST_SUITE(PIRLTestCase);
	CPPUNIT_TEST(test_max_rate);
	CPPUNIT_TEST(test_cost);
	CPPUNIT_TEST_SUITE_END();
	
	void test_max_rate(void);
	void test_cost(void);
	pirl pirl_inst;	
public:
	void setUp(void);
	void tearDown(void);
};

void PIRLTestCase::setUp(){
	pirl_inst.reconfigure(MAX_RATE_S);
}

void PIRLTestCase::tearDown(){
}

#define MAX_ITERATIONS 0xFFFFFFF

/* Test max rate; Tests */
void PIRLTestCase::test_max_rate(void)
{
	int cnt=0;
	struct timespec tp_init, tp_final;
        int n_buckets, init_ms, final_ms;
	
	clock_gettime(/*CLOCK_MONOTONIC_COARSE*/CLOCK_REALTIME, &tp_init);

	for(int i=0;i<MAX_ITERATIONS;i++){
		if(pirl_inst.filter_pkt() == false)
			cnt++;
	}
	
	clock_gettime(/*CLOCK_MONOTONIC_COARSE*/CLOCK_REALTIME, &tp_final);

	//Calculate number of real buckets
	init_ms = tp_init.tv_sec*1000 + tp_init.tv_nsec/1000000;
	final_ms = tp_final.tv_sec*1000 + tp_final.tv_nsec/1000000;
	n_buckets = (final_ms - init_ms) /(1000/pirl::PIRL_NUMBER_OF_BUCKETS_PER_S) + 1; 
	if((final_ms - init_ms) % (1000/pirl::PIRL_NUMBER_OF_BUCKETS_PER_S) != 0)
		n_buckets++;
	float seconds = ((float)(final_ms-init_ms))/1000;
	float rate =  ((float)cnt)/seconds; 
	int rate_normalized = ((int)(rate/n_buckets))*n_buckets;
	fprintf(stderr,"Outputed rate (pkt/s): %f, normalized (pkts/s): %f, cnt %d, n_buckets %d, seconds %f\n", rate, (float)rate_normalized, cnt, n_buckets, seconds); 
	CPPUNIT_ASSERT( (n_buckets*MAX_RATE_S/pirl::PIRL_NUMBER_OF_BUCKETS_PER_S) >= cnt );
}

inline uint64_t rdtsc() {
	#if defined(__GNUC__)
	#   if defined(__i386__)
	    uint64_t x;
	    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	    return x;
	#   elif defined(__x86_64__)
	    uint32_t hi, lo;
	    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	    return ((uint64_t)lo) | ((uint64_t)hi << 32);
	#   else
	#       error Unsupported architecture.
	#   endif
	#elif defined(_MSC_VER)
	    return __rdtsc();
	#else
	#   error Other compilers not supported...
	#endif
}

/* Test max rate; Tests */
void PIRLTestCase::test_cost(void)
{
	unsigned int cnt=0;
	uint64_t ticks_initial, ticks_final;

	ticks_initial = rdtsc();

	for(cnt=0;cnt<MAX_ITERATIONS;cnt++)
		pirl_inst.filter_pkt();
	
	ticks_final = rdtsc();
	
	fprintf(stderr,"Cost in ticks %llu\n", (long long unsigned)(ticks_final-ticks_initial)/MAX_ITERATIONS);
}
/*
* Test MAIN
*/
int main( int argc, char* argv[] )
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(PIRLTestCase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );
	
	std::cerr<<"************** Test finished ************"<<std::endl;

	// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
