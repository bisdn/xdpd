#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

//Include ringbuffer
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <unistd.h>
#include <pthread.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow12/of12_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/matching_algorithms/matching_algorithms_available.h>
#include "util/ringbuffer.h"
#include "io/iomanager.h"
#include "io/bufferpool.h"
#include "processing/processingmanager.h"
#include "ls_internal_state.h"
#include "util/ringbuffer_c_wrapper.h"

#ifndef DEBUG
	#error "This test can only run in debug mode..."
#endif

using namespace std;

class ProcessingManagerTestCase : public CppUnit::TestFixture{

	CPPUNIT_TEST_SUITE(ProcessingManagerTestCase);
	CPPUNIT_TEST(test_processing);
	CPPUNIT_TEST_SUITE_END();

	//Test methods
	void test_processing(void);

	//Suff
	of_switch_t* sw;
	ringbuffer* buffer;
	struct logical_switch_internals lsi;

	public:
		void setUp(void);
		void tearDown(void);
};

/* Setup and tear down */
void ProcessingManagerTestCase::setUp(){
	
	of12_matching_algorithm_available matching_algorithms[4] = {of12_matching_algorithm_loop,of12_matching_algorithm_loop,of12_matching_algorithm_loop,of12_matching_algorithm_loop};
	
	//Init bufferpool
	bufferpool::init(2048);	

	//Initializae a switch
	sw = (of_switch_t*)of12_init_switch("test",0x12345,4, matching_algorithms);

	
	buffer = (ringbuffer*) ((struct logical_switch_internals*) sw->platform_state )->ringbuffer;
}

void ProcessingManagerTestCase::tearDown(){

	std::cerr<<"Tearing down testcase..."<<std::endl;
	
	//delete switch
	of_destroy_switch(sw);
}

/* Tests */
void ProcessingManagerTestCase::test_processing(){

	int number_of_packets;	

	//Set by pass of the pipeline
	processingmanager::by_pass_pipeline = true;
	
	//Launch threads
	processingmanager::start_ls_workers(sw);

	number_of_packets = rand()%2000;
	
	cerr << "Number of packets to be sent..." << number_of_packets << endl;

	//Enqueue packets
	for(int i=0;i<number_of_packets;i++){

		datapacket_t* pkt = bufferpool::get_free_buffer();

		//Enqueue
		buffer->blocking_write(pkt);
	
		//Throttle a little bit
		usleep(rand()%250);
	}

	processingmanager::stop_ls_workers(sw);
	
	//No packets on the queue
	CPPUNIT_ASSERT(buffer->size() == 0);
	
}


/*
* Test MAIN
*/
int main( int argc, char* argv[] )
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(ProcessingManagerTestCase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );

	// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
