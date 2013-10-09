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
#include "io/datapacket_storage.h"
#include "io/ports/mockup/ioport_mockup.h"
#include "processing/processingmanager.h"


#define TEST_DPID 0x1015

using namespace std;
using namespace xdpd::gnu_linux;

class DriverPortMockupTestCase : public CppUnit::TestFixture{

	CPPUNIT_TEST_SUITE(DriverPortMockupTestCase);
	CPPUNIT_TEST(test_bufferpool_saturation);
	CPPUNIT_TEST(test_drop_packets);
	CPPUNIT_TEST(test_output);
	CPPUNIT_TEST(test_flow_expiration);
	CPPUNIT_TEST_SUITE_END();

	//Test methods
	void test_drop_packets(void);
	void test_output(void);
	void test_bufferpool_saturation(void);
	void test_flow_expiration(void);
	
	//Suff
	of_switch_t* sw;
	switch_port_t* port;
	ioport_mockup* mport;
	int write_fd;

	public:
		void setUp(void);
		void tearDown(void);
};

#define PORT_NAME "fake0"

/* Setup and tear down */
void DriverPortMockupTestCase::setUp(){

	afa_result_t res;
	unsigned int of_port_num=0;
	fprintf(stderr,"<%s:%d> ************** Set up ************\n",__func__,__LINE__);
	
	res = fwd_module_init();//discovery of ports
	if( res != AFA_SUCCESS )
		exit(-1);

	char switch_name[] = "switch1";
	of1x_matching_algorithm_available ma_list[] = { of1x_matching_algorithm_loop };
	/* 0->CONTROLLER, 1->CONTINUE, 2->DROP, 3->MASK */
	sw = fwd_module_create_switch(switch_name,TEST_DPID,OF_VERSION_12,1,(int *) ma_list);
	CPPUNIT_ASSERT(sw->platform_state); /* internal state */

	//Construct the port group
	port = switch_port_init((char*)PORT_NAME, true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE);
	CPPUNIT_ASSERT(NULL != port);

	mport = new ioport_mockup(port); //using Port Mockup
	port->platform_port_state = (platform_port_state_t*)mport;
	write_fd = mport->get_fake_write_fd();

	//Fake discovery of port in the physical switch
	physical_switch_add_port(port); 
	
	//Attach
	afa_result_t ret = fwd_module_attach_port_to_switch(TEST_DPID, PORT_NAME , &of_port_num); 
	CPPUNIT_ASSERT(ret == AFA_SUCCESS);
 	CPPUNIT_ASSERT(of_port_num > 0);
	fprintf(stderr,"Port [%s] attached to sw [%s] at port #%u\n", PORT_NAME, sw->name,of_port_num);
 
	//NOTE here start of background_manager

}

void DriverPortMockupTestCase::tearDown(){
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


/* Tests */
void DriverPortMockupTestCase::test_drop_packets(void )
{
	ssize_t res;
	int number_of_packets = 20;//20000;//10;//rand()%2000;
	char buffer[ioport_mockup::SIMULATED_PKT_SIZE];

	fprintf(stderr,"<%s:%d> ************** drop_packets ************\n",__func__,__LINE__);
	cerr << "Number of packets to be sent..." << number_of_packets << endl;
	
	//Start port XXX: this should NOT be done this way. Driver 
	rofl_result_t ret = iomanager::bring_port_up(mport);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);
	
	//Initialize buffer (prevent valgrind to complain)
	memset(buffer,0,sizeof(buffer));
	

	//Get ringbuffer
	circular_queue<datapacket_t, 1024>* rbuffer = ((struct logical_switch_internals*) sw->platform_state )->input_queues[0];
	
	//Enqueue packets
	for(int i=0;i<number_of_packets;i++){
		//cerr << "<" <<__func__<< __LINE__ <<">Writting packet to fake port i:" << i << endl;
		res = ::write(write_fd,buffer,ioport_mockup::SIMULATED_PKT_SIZE);
		if (res<0)
			fprintf(stderr,"<%s:%d> Write error\n",__func__,__LINE__);

		//Throttle a little bit
		usleep(rand()%250);
	}

	//Leave time to process just in case, before checking the buffer size
	sleep(2);
	
	//No packets on the queue
	CPPUNIT_ASSERT(rbuffer->size() == 0);
}


void DriverPortMockupTestCase::test_output(){
	ssize_t res;
	int number_of_packets = 20;
	wrap_uint_t field;
	char buffer[ioport_mockup::SIMULATED_PKT_SIZE];
	//Initialize buffer (prevent valgrind to complain)
	memset(buffer,0,sizeof(buffer));
	/*
	 * set a flow mod and an action OUTPUT to see how the packets go through
	 */
	fprintf(stderr,"<%s:%d>************** Starting test output action**************\n",__func__,__LINE__);
	of1x_match_t *match = of1x_init_port_in_match(NULL,NULL,1);
	of1x_flow_entry_t *entry = of1x_init_flow_entry(NULL,NULL, false);
	of1x_action_group_t* ac_group = of1x_init_action_group(NULL);
	entry->priority = 1;
	
	of1x_add_match_to_entry(entry,match);
	field.u64 = 1;
	of1x_push_packet_action_to_group(ac_group, of1x_init_packet_action(/*(of1x_switch_t*)sw,*/ OF1X_AT_OUTPUT, field, NULL,NULL));
	of1x_add_instruction_to_group(&entry->inst_grp, OF1X_IT_APPLY_ACTIONS, ac_group , NULL, NULL, 0);
	of1x_add_flow_entry_table( ((of1x_switch_t *)sw)->pipeline, 0, entry, false, false );
	
	//Start port XXX: this should NOT be done this way. Driver
	iomanager::bring_port_up(mport);
	
	circular_queue<datapacket_t, 1024>* rbuffer = ((struct logical_switch_internals*) sw->platform_state )->input_queues[0];
	
	//Enqueue packets
	for(int i=0;i<number_of_packets;i++){
		res = ::write(write_fd,buffer,ioport_mockup::SIMULATED_PKT_SIZE);
		if (res<0)
			fprintf(stderr,"<%s:%d> Write error\n",__func__,__LINE__);

		//Throttle a little bit
		usleep(rand()%250);
	}

	//Leave time to process just in case, before checking the buffer size
	sleep(2);
	
	//No packets on the queue
	CPPUNIT_ASSERT(rbuffer->size() == 0);
}

void DriverPortMockupTestCase::test_flow_expiration(){
	/*add a flow mod in the table and check that is expiring*/
	int sec_exp = 2;
	wrap_uint_t field;
	
	fprintf(stderr,"<%s:%d>************** Initialize test flow expiration **************\n",__func__,__LINE__);
	of1x_match_t *match = of1x_init_port_in_match(NULL,NULL,1);
	of1x_flow_entry_t *entry = of1x_init_flow_entry(NULL,NULL,false);
	of1x_action_group_t* ac_group = of1x_init_action_group(NULL);
	entry->priority = 1;
	
	__of1x_fill_new_timer_entry_info(entry,sec_exp,0); /*idle TO disabled*/
	
	of1x_add_match_to_entry(entry,match);
	field.u64 = 1;
	of1x_push_packet_action_to_group(ac_group, of1x_init_packet_action(/*(of1x_switch_t*)sw,*/ OF1X_AT_OUTPUT, field, NULL,NULL));
	of1x_add_instruction_to_group(&entry->inst_grp, OF1X_IT_APPLY_ACTIONS, ac_group , NULL, NULL, 0);
	of1x_add_flow_entry_table( ((of1x_switch_t *)sw)->pipeline, 0, entry, false, false );
	
	fprintf(stderr,"<%s:%d> table 0 num of entries %d\n",__func__,__LINE__,((of1x_switch_t*)sw)->pipeline->tables[0].num_of_entries);
	CPPUNIT_ASSERT(((of1x_switch_t*)sw)->pipeline->tables[0].num_of_entries == 1 );
	
	sleep(sec_exp + 2);
	
	CPPUNIT_ASSERT(((of1x_switch_t*)sw)->pipeline->tables[0].num_of_entries == 0 );
	fprintf(stderr,"<%s:%d> table 0 num of entries %d\n",__func__,__LINE__,((of1x_switch_t*)sw)->pipeline->tables[0].num_of_entries);
	
}

void DriverPortMockupTestCase::test_bufferpool_saturation(){

	/*
	 * check that when there is no flow mod packets are discarded without memory leaks
	 * also check that there are still buffers enough (must send more buffers than bufferpool has)
	 */
	
	int number_of_packets;
	ssize_t res;
	char buffer[ioport_mockup::SIMULATED_PKT_SIZE];
	
	fprintf(stderr,"<%s:%d> ************** bufferpool_saturation ************\n",__func__,__LINE__);
	
	//Initialize buffer (prevent valgrind to complain)
	memset(buffer,0,sizeof(buffer));
	
	circular_queue<datapacket_t, 1024>* rbuffer = ((struct logical_switch_internals*) sw->platform_state )->input_queues[0];

	//We are going to force LS threads to be stopped and fill in the LS queue
	processingmanager::stop_ls_workers(sw);

	//Start port XXX: this should NOT be done this way. Driver
	rofl_result_t ret = iomanager::bring_port_up(mport);  
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);

	number_of_packets = rbuffer->MAX_SLOTS+10;

	cerr << "Sending number of packets: " << number_of_packets << endl;
	//Enqueue packets
	for(int i=0;i<number_of_packets;i++){
		//cerr << "$$$$$$" <<__func__<< __LINE__ <<">Writting packet to fake port i:" << i << endl;
		res = ::write(write_fd,buffer,ioport_mockup::SIMULATED_PKT_SIZE);
		if (res<0)
			fprintf(stderr,"<%s:%d> Write error\n",__func__,__LINE__);

		usleep(rand()%250); //Throttle a little bit
	}
	
	//Give some time to buffer packes (read from mockup)
	sleep(2);
	
	//No packets on the queue
	cerr << "buffering status ["<<rbuffer->size()<<","<<rbuffer->MAX_SLOTS<<"]"<< endl;
	
	//Check buffer is full
	CPPUNIT_ASSERT(rbuffer->size() == (rbuffer->MAX_SLOTS-1));

	//restart processing threads	
	processingmanager::start_ls_workers(sw);

	//Give some time to process
	sleep(3);
	
	//No packets on the queue
	CPPUNIT_ASSERT(rbuffer->size() == 0);

}	


/*
* Test MAIN
*/
int main( int argc, char* argv[] )
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(DriverPortMockupTestCase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );
	
	std::cerr<<"************** Test finished ************"<<std::endl;

	// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
