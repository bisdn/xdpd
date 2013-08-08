#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <unistd.h>
#include <pthread.h>
#include <rofl/pipeline/common/datapacket.h>
#include <rofl/pipeline/switch_port.h>
#include "io/scheduler/epoll_ioscheduler.h"
#include "io/ports/mockup/ioport_mockup.h"
#include "io/iomanager.h"
#include "io/bufferpool.h"

using namespace std;

class MockupPortTestcase : public CppUnit::TestFixture{

	CPPUNIT_TEST_SUITE(MockupPortTestcase);
	CPPUNIT_TEST(test_single_port);
	CPPUNIT_TEST_SUITE_END();

	//Test methods
	void test_single_port(void);

	//Suff
	switch_port_t* port;
	int write_fd;

	public:
		void setUp(void);
		void tearDown(void);
};

/* Other CPPUnit stuff */
// CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( MockupPortTestcase, "MockupPortTestcase" );

/* Setup and tear down */
void MockupPortTestcase::setUp(){

	//Init bufferpool
	bufferpool::init(2048);	
	//Set to by-pass processing systemLoop	
	epoll_ioscheduler::set_by_pass_processing(true);
	
	//Construct the port group	
	port = new switch_port_t;

	//Port Mockup	
	ioport_mockup* mport = new ioport_mockup();
	port->platform_port_state = (platform_port_state_t*)mport;
	write_fd = mport->get_fake_write_fd();
	//std::cerr<<"Pointer:"<<mport<<std::endl;	
		
}

void MockupPortTestcase::tearDown(){

	delete (ioport_mockup*)port->platform_port_state;
	delete port;
	
}

/* Test specific */
void MockupPortTestcase::test_single_port(){

	#define ioport_mockup::SIMULATED_PKT_SIZE 100
	unsigned int id;
	char buffer[ioport_mockup::SIMULATED_PKT_SIZE];

	//init the random number gen.
	srand(time(NULL));

	//Create port group
	safevector<switch_port_t*> grp;
	
	//Add to safefector port	 
	grp.push_back(port);
	
	//Add to portgroup pool
	id = iomanager::add_group(grp,1);
	
	//Launch
	iomanager::start_group(id);
	
	//Send stuff for the stdin so that port receives it and processes it
	for(int i=0;i<1200;i++){

		//Write (simulate packet
		::write(write_fd,buffer,ioport_mockup::SIMULATED_PKT_SIZE);
	
		//Sleep random ms  
		//usleep(rand()%1000 + 50);
	}

	sleep(50);

	//Stop it
	iomanager::stop_group(id);

	//delete it from schedulable groups
	iomanager::delete_group(id);
}


/*
* Test MAIN
*/
int main( int argc, char* argv[] )
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(MockupPortTestcase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );

// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
