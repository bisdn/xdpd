#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>
#include <endian.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/afa/fwd_module.h>
#include "io/iomanager.h"
#include "io/ports/mmap/ioport_mmap.h"
#include "processing/processingmanager.h"


#define TEST_DPID 0x1015

using namespace std;

class DriverMMAPPortTestCase : public CppUnit::TestFixture{

	CPPUNIT_TEST_SUITE(DriverMMAPPortTestCase);
	CPPUNIT_TEST(bring_up_down_only);
	//CPPUNIT_TEST(test_no_flowmod);
	CPPUNIT_TEST_SUITE_END();

	//Test methods
	void bring_up_down_only(void);

	//Utils
	void install_flow_mod(void);
	
	//Suff
	of_switch_t* sw;
	char port_name[12];

	public:
		void setUp(void);
		void tearDown(void);
};


/* Setup and tear down */
void DriverMMAPPortTestCase::setUp(){

	afa_result_t res;
	unsigned int of_port_num=0;
	
	fprintf(stderr,"<%s:%d> ************** Set up ************\n",__func__,__LINE__);
	snprintf(port_name, 6, "%s", "veth0");
	
	res = fwd_module_init();//discovery of ports
		
	if( res != AFA_SUCCESS )
		exit(-1);


	//Create switch
	char switch_name[] = "switch1";
	of1x_matching_algorithm_available ma_list[] = { of1x_matching_algorithm_loop };
	/* 0->CONTROLLER, 1->CONTINUE, 2->DROP, 3->MASK */
	sw = fwd_module_create_switch(switch_name,TEST_DPID,OF_VERSION_12,1,(int *) ma_list);
	CPPUNIT_ASSERT(sw->platform_state); /*ringbuffer*/


	//Attach
	res = fwd_module_attach_port_to_switch(TEST_DPID, port_name , &of_port_num); 	
	CPPUNIT_ASSERT( res == AFA_SUCCESS);

 	CPPUNIT_ASSERT(of_port_num > 0);
	fprintf(stderr,"Port [%s] attached to sw [%s] at port #%u\n", port_name, sw->name,of_port_num);
 

	install_flow_mod();
}

void DriverMMAPPortTestCase::tearDown(){
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

void DriverMMAPPortTestCase::install_flow_mod(){
	fprintf(stderr,"Installing flow_mod\n");

	of1x_match_t *match = of1x_init_port_in_match(NULL,NULL,1);
	of1x_match_t *match2 = of1x_init_eth_src_match(NULL,NULL, (htobe64(0x86f3d23e8c30)>>16)&0xFFFFFFFFFFFF, 0xFFFFFFFFFFFF);
	of1x_flow_entry_t *entry = of1x_init_flow_entry(NULL,NULL,false);
	of1x_action_group_t* ac_group = of1x_init_action_group(NULL);
	entry->priority = 1;
	
	wrap_uint_t field;
	field.u64 = (htobe64(0x012345678901)>>16)&0xFFFFFFFFFFFF;
	
	of1x_add_match_to_entry(entry,match);
	of1x_add_match_to_entry(entry,match2);
	of1x_push_packet_action_to_group(ac_group, of1x_init_packet_action(/*(of1x_switch_t*)sw,*/ OF1X_AT_SET_FIELD_ETH_SRC, field, NULL,NULL));
	fprintf(stderr,"Big endian MAC: %lx",htobe64(0x0000012345678901));
	field.u64 = 1;
	of1x_push_packet_action_to_group(ac_group, of1x_init_packet_action(/*(of1x_switch_t*)sw,*/ OF1X_AT_OUTPUT, field, NULL,NULL));
	of1x_add_instruction_to_group(&entry->inst_grp, OF1X_IT_APPLY_ACTIONS, ac_group , NULL, NULL, 0);
	of1x_add_flow_entry_table( ((of1x_switch_t *)sw)->pipeline, 0,entry,false,false );
	
	CPPUNIT_ASSERT(((of1x_switch_t*)sw)->pipeline->tables[0].num_of_entries == 1 );

}	

/* Tests */
void DriverMMAPPortTestCase::bring_up_down_only(){

	afa_result_t res;

	//Bring up port
	res = fwd_module_enable_port(port_name);
	CPPUNIT_ASSERT(res == AFA_SUCCESS);
	(void)res;

	//Wait some time
	sleep(10);

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
	runner.addTest(DriverMMAPPortTestCase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );
	
	std::cerr<<"************** Test finished ************"<<std::endl;

	// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
