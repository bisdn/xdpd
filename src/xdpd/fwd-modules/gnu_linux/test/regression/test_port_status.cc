#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/afa/fwd_module.h>
#include "io/iomanager.h"
#include "io/ports/mmap/ioport_mmap.h"
#include "processing/processingmanager.h"

/**
 * This test must check if the proper messages to hcl
 * are sended when port if configured up and down
 * or when the link fails
 */

#define TEST_DPID 0x1015
#define PORT_NAME "veth0"

using namespace std;

class DriverPortStatusTestCase : public CppUnit::TestFixture{

	CPPUNIT_TEST_SUITE(DriverPortStatusTestCase);
	CPPUNIT_TEST(bring_up_down_only);
	CPPUNIT_TEST_SUITE_END();

	//Test methods
	void bring_up_down_only(void);
	
	//Suff
	of_switch_t* sw;
	char port_name[12];

	public:
		void setUp(void);
		void tearDown(void);
};


/* Setup and tear down */
void DriverPortStatusTestCase::setUp(){

	afa_result_t res;
	unsigned int of_port_num = 0;
	
	fprintf(stderr,"<%s:%d> ************** Set up ************\n",__func__,__LINE__);
	snprintf(port_name, 6, "%s", PORT_NAME);
	
	res = fwd_module_init();//discovery of ports
		
	if( res != AFA_SUCCESS )
		exit(-1);

	//Create switch
	char switch_name[] = "switch1";
	of1x_matching_algorithm_available ma_list[] = { of1x_matching_algorithm_loop };
	/* 0->CONTROLLER, 1->CONTINUE, 2->DROP, 3->MASK */
	sw = fwd_module_create_switch(switch_name,TEST_DPID,OF_VERSION_12,1,(int *) ma_list);
	CPPUNIT_ASSERT(sw->platform_state); /*ringbuffer + datapacket_storage*/

	//Attach
	res = fwd_module_attach_port_to_switch(TEST_DPID, port_name , &of_port_num); 	
	CPPUNIT_ASSERT( res == AFA_SUCCESS);

 	CPPUNIT_ASSERT(of_port_num > 0);
	fprintf(stderr,"Port [%s] attached to sw [%s] at port #%u\n", port_name, sw->name,of_port_num);
}

void DriverPortStatusTestCase::tearDown(){
	int ret;
	fprintf(stderr,"<%s:%d> ************** Tear Down ************\n",__func__,__LINE__);
	
	//explicit detach
	if((ret = fwd_module_detach_port_from_switch(TEST_DPID,PORT_NAME))!=AFA_SUCCESS){
		fprintf(stderr,"detach port failed");
		exit(-1);
	}
	
	//delete switch
	if(	(ret=fwd_module_destroy_switch_by_dpid(sw->dpid))!=AFA_SUCCESS){
		fprintf(stderr,"destroy switch failure!");
		exit(-1);
	}
	
	if((ret=fwd_module_destroy())!=AFA_SUCCESS){
		fprintf(stderr,"driver failure!");
		exit(-1);
	}
}
 

/* Tests */
void DriverPortStatusTestCase::bring_up_down_only(){

	afa_result_t res;

	//Bring up port
	res = fwd_module_enable_port(port_name);
	CPPUNIT_ASSERT(res == AFA_SUCCESS);
	(void)res;

	//Wait some time
	sleep(5);

	//Bring down
	res = fwd_module_disable_port(port_name);
	CPPUNIT_ASSERT(res == AFA_SUCCESS);

}

/*
* Test MAIN
*/
int main( int argc, char* argv[] )
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(DriverPortStatusTestCase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );
	
	std::cerr<<"************** Test finished ************"<<std::endl;

	// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
