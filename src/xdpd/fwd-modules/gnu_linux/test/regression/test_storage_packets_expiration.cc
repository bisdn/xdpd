//#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>
#include <unistd.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/afa/fwd_module.h>
#include "processing/ls_internal_state.h"
#include "io/datapacket_storage.h"
#include "bg_taskmanager.h"

#define TEST_DPID 0x1015
#define EXPIRATION_TIME 3

using namespace xdpd::gnu_linux;

/**
 * This test is suposed to check how the background tasks manager is 
 * exiring the old buffers from the datapacket_storage bin.
 * 
 * Therefore we need to create a switch, and a storage for it
 * we need to insert  a buffer on it and we need to check how the the
 * bg_tasks_manager is expiring the packets
 * 
 * No ports or packet-in will be involved in this test
 */

using namespace std;

class DriverStoragePacketsExpirationTestCase : public CppUnit::TestFixture{

	CPPUNIT_TEST_SUITE(DriverStoragePacketsExpirationTestCase);
	CPPUNIT_TEST(test_buffers_expiration);
	CPPUNIT_TEST_SUITE_END();

	//Test methods
	void test_buffers_expiration(void);
	
	//Suff
	of_switch_t* sw;

	public:
		void setUp(void);
		void tearDown(void);
};


/* Setup and tear down */
void DriverStoragePacketsExpirationTestCase::setUp(){

	afa_result_t res;
	fprintf(stderr,"<%s:%d> ************** Set up ************\n",__func__,__LINE__);
	
	res = fwd_module_init();//discovery of ports
	CPPUNIT_ASSERT( res == AFA_SUCCESS );
	(void)res;
	
	char switch_name[] = "switch1";
	of1x_matching_algorithm_available ma_list[] = { of1x_matching_algorithm_loop };
	/* 0->CONTROLLER, 1->CONTINUE, 2->DROP, 3->MASK */
	sw = fwd_module_create_switch(switch_name,TEST_DPID,OF_VERSION_12,1,(int *) ma_list);
	CPPUNIT_ASSERT(sw->platform_state); /* internal state */
	
	//change expiration time
	( (datapacket_storage*) ((struct logical_switch_internals*) sw->platform_state)->storage)->change_expiration_time(EXPIRATION_TIME);

}

void DriverStoragePacketsExpirationTestCase::tearDown(){
	int ret;
	fprintf(stderr,"<%s:%d> ************** Tear Down ************\n",__func__,__LINE__);
	
	//delete switch
	if(	(ret=fwd_module_destroy_switch_by_dpid(sw->dpid))!=0)
	{
		fprintf(stderr,"destroy switch failure!");
		exit(-1);
	}
	
	if((ret=fwd_module_destroy())!=0)
	{
		fprintf(stderr,"driver failure!");
		exit(-1);
	}
}

void DriverStoragePacketsExpirationTestCase::test_buffers_expiration(void )
{
		datapacket_t pkt;
		storeid id;
		int sto_size;
		
		fprintf(stderr,"<%s:%d> ************** test_buffers_expiration ************\n",__func__,__LINE__);
		
		sto_size = ( (datapacket_storage*) ((struct logical_switch_internals*) sw->platform_state)->storage)->get_storage_size();
		fprintf(stderr,"<%s:%d> size of storage= %d\n",__func__,__LINE__,sto_size);
		CPPUNIT_ASSERT(sto_size == 0);
		
		id = ( (datapacket_storage*) ((struct logical_switch_internals*) sw->platform_state)->storage)->store_packet(&pkt);
		CPPUNIT_ASSERT(id>=0);
		//wait and check expiration
		
		sto_size = ( (datapacket_storage*) ((struct logical_switch_internals*) sw->platform_state)->storage)->get_storage_size();
		fprintf(stderr,"<%s:%d> size of storage= %d\n",__func__,__LINE__,sto_size);
		CPPUNIT_ASSERT(sto_size == 1);
		
		sleep(EXPIRATION_TIME+LSW_TIMER_BUFFER_POOL_MS/1000+1);
		
		sto_size = ( (datapacket_storage*) ((struct logical_switch_internals*) sw->platform_state)->storage)->get_storage_size();
		fprintf(stderr,"<%s:%d> size of storage= %d\n",__func__,__LINE__,sto_size);
		CPPUNIT_ASSERT(sto_size == 0);
}

/*
* Test MAIN
*/
int main( int argc, char* argv[] )
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(DriverStoragePacketsExpirationTestCase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );
	
	std::cerr<<"************** Test finished ************"<<std::endl;

	// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}



