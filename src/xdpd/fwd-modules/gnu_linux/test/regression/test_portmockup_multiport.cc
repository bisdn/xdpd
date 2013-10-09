#include <memory>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>
#include <pthread.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/afa/fwd_module.h>
#include "io/iomanager.h"
#include "io/ports/mockup/ioport_mockup.h"
#include "processing/processingmanager.h"


#define TEST_DPID 0x1015

using namespace std;
using namespace xdpd::gnu_linux;

class DriverMultiPortMockupTestCase : public CppUnit::TestFixture{

	CPPUNIT_TEST_SUITE(DriverMultiPortMockupTestCase);
	//CPPUNIT_TEST(test_drop_packets);
	CPPUNIT_TEST(test_up_down);
	CPPUNIT_TEST_SUITE_END();

	//Test methods
	void test_drop_packets(void);
	void test_up_down(void);

	public:	
		//Suff
		of_switch_t* sw;
		switch_port_t *port1, *port2;
		ioport_mockup *mport1, *mport2;
		int write_fd1, write_fd2;

		void setUp(void);
		void tearDown(void);
};

#define PORT_NAME0 "fake0"
#define PORT_NAME1 "fake1"

/* Setup and tear down */
void DriverMultiPortMockupTestCase::setUp(){

	afa_result_t res;
	unsigned int of_port_num=0;
	fprintf(stderr,"<%s:%d> ************** Set up ************\n",__func__,__LINE__);

	res = fwd_module_init();//discovery of ports
	
	if( res != AFA_SUCCESS )
		exit(-1);

	//Initialize driver	
	char switch_name[] = "switch1";
	of1x_matching_algorithm_available ma_list[] = { of1x_matching_algorithm_loop };
	sw = fwd_module_create_switch(switch_name,TEST_DPID,OF_VERSION_12,1,(int *) ma_list);
	CPPUNIT_ASSERT(sw->platform_state); /* internal state */

	//Construct the port1
	port1 = switch_port_init((char*)PORT_NAME0, true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE);
	CPPUNIT_ASSERT(NULL != port1);


	mport1 = new ioport_mockup(port1); //using Port Mockup
	port1->platform_port_state = (platform_port_state_t*)mport1;
	write_fd1 = mport1->get_fake_write_fd();
	
	//Construct the port2
	port2 = switch_port_init((char*)PORT_NAME1, true/*will be overriden afterwards*/, PORT_TYPE_PHYSICAL, PORT_STATE_LIVE);
	CPPUNIT_ASSERT(NULL != port2);


	mport2 = new ioport_mockup(port2); //using Port Mockup
	port2->platform_port_state = (platform_port_state_t*)mport2;
	write_fd2 = mport2->get_fake_write_fd();
	
	//Fake discovery of port in the physical switch
	physical_switch_add_port(port1); 
	physical_switch_add_port(port2); 
	
	//Attach
	afa_result_t ret = fwd_module_attach_port_to_switch(TEST_DPID, PORT_NAME0 , &of_port_num); 	
	CPPUNIT_ASSERT(ret == AFA_SUCCESS);
	(void)ret;
 	CPPUNIT_ASSERT(of_port_num > 0);
	fprintf(stderr,"Port [%s] attached to sw [%s] at port #%u\n", PORT_NAME0, sw->name,of_port_num);
 
	of_port_num=0;
	ret = fwd_module_attach_port_to_switch(TEST_DPID, PORT_NAME1 , &of_port_num); 	
	CPPUNIT_ASSERT(ret == AFA_SUCCESS);
  	CPPUNIT_ASSERT(of_port_num > 0);
	fprintf(stderr,"Port [%s] attached to sw [%s] at port #%u\n", PORT_NAME1, sw->name,of_port_num);
 
	//NOTE here start of background_manager

}

void DriverMultiPortMockupTestCase::tearDown(){
	int ret;
	fprintf(stderr,"<%s:%d> ************** Tear Down ************\n",__func__,__LINE__);
	
	//delete switch
	if(	(ret=fwd_module_destroy_switch_by_dpid(sw->dpid))!=0){
		fprintf(stderr,"destroy switch failure!");
		exit(-1);

	}
	
	if((ret=fwd_module_destroy())!=0){
		fprintf(stderr,"driver failure!");
		exit(-1);
	}
}

/* Tests */
void DriverMultiPortMockupTestCase::test_drop_packets(void )
{
	int number_of_packets = 20;//20000;//10;//rand()%2000;
	ssize_t res;
	char buffer[ioport_mockup::SIMULATED_PKT_SIZE];

	fprintf(stderr,"************** Drop packets %s ************\n",__func__);
	cerr << "Number of packets to be sent..." << number_of_packets << endl;
	
	//Start port XXX: this should NOT be done this way. Driver
	rofl_result_t ret = iomanager::bring_port_up(mport1);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);
	(void)ret;
	ret = iomanager::bring_port_up(mport2);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);
	
	//Initialize buffer (prevent valgrind to complain)
	memset(buffer,0,sizeof(buffer));
	

	//srand(time(NULL)); //random seed
	circular_queue<datapacket_t, 1024>* rbuffer = ((struct logical_switch_internals*) sw->platform_state )->input_queues[0];
	
	//Enqueue packets
	for(int i=0;i<number_of_packets;i++){
		//cerr << "<" <<__func__<< __LINE__ <<">Writting packet to fake port i:" << i << endl;
		if((res=::write(write_fd1,buffer,ioport_mockup::SIMULATED_PKT_SIZE))<0)
			fprintf(stderr,"<%s:%d>Write error\n",__func__,__LINE__);
		if((res=::write(write_fd2,buffer,ioport_mockup::SIMULATED_PKT_SIZE))<0)
			fprintf(stderr,"<%s:%d>Write error\n",__func__,__LINE__);

		//Throttle a little bit
		usleep(rand()%250);
	}

	//Leave time to process just in case, before checking the buffer size
	sleep(2);
	(void)rbuffer;
	
	//No packets on the queue
	CPPUNIT_ASSERT(rbuffer->size() == 0);
}

void* write_to_ports(void* param){
	ssize_t res;

	DriverMultiPortMockupTestCase* test = (DriverMultiPortMockupTestCase*)param;
	char buffer[ioport_mockup::SIMULATED_PKT_SIZE];
	
	//Initialize buffer (prevent valgrind to complain)
	memset(buffer,0,sizeof(buffer));

	//Enqueue packets
	for(int i=0;i<5000;i++){
		cerr << "<" <<__func__<< __LINE__ <<">Writting packet to fake port i:" << i << endl;
		if(rand()%2 == 0){
			if((res=::write(test->write_fd1,buffer,ioport_mockup::SIMULATED_PKT_SIZE))<0)
				fprintf(stderr,"<%s:%d>Write error\n",__func__,__LINE__);
		}else{
			if((res=::write(test->write_fd2,buffer,ioport_mockup::SIMULATED_PKT_SIZE))<0)
				fprintf(stderr,"<%s:%d>Write error\n",__func__,__LINE__);
		}
		//Throttle a little bit
		usleep(rand()%250);
	}

	return NULL;
}

void DriverMultiPortMockupTestCase::test_up_down(){

	pthread_t thread_state;

	/*
	 * set a flow mod and an action OUTPUT to see how the packets go through
	 */
	fprintf(stderr,"************** Startting %s **************\n",__func__);

	//Start ports
	rofl_result_t ret = iomanager::bring_port_up(mport1);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);
	(void)ret;
	ret = iomanager::bring_port_up(mport2);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);

	//Launch writers
	pthread_create(&thread_state, NULL, write_to_ports, (void *)this);

	sleep(rand()%2);
	//Stop port 1
	ret = iomanager::bring_port_down(mport1);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);

	sleep(rand()%2);
	//Start port 1	
	ret = iomanager::bring_port_up(mport1);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);


	sleep(rand()%2);
	//Stop port 2
	ret = iomanager::bring_port_down(mport2);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);


	sleep(rand()%2);
	//Start port 2
	ret = iomanager::bring_port_up(mport2);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);


	usleep(rand()%250);
	//Stop both
	ret = iomanager::bring_port_down(mport1);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);
	ret = iomanager::bring_port_down(mport2);
	CPPUNIT_ASSERT(ret == ROFL_SUCCESS);



	//Wait for thread
	pthread_cancel(thread_state);
	pthread_join(thread_state,NULL);
}	

/*
* Test MAIN
*/
int main( int argc, char* argv[] )
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(DriverMultiPortMockupTestCase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );
	
	std::cerr<<"************** Test finished ************"<<std::endl;

	// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
