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
#include "io/datapacket_storage.h"

#define STORE_SIZE 15
#define EXPIRATION_SEC 3

using namespace std;
using namespace xdpd::gnu_linux;

class DataPacketStorageTestCase : public CppUnit::TestFixture{
	CPPUNIT_TEST_SUITE(DataPacketStorageTestCase);
	CPPUNIT_TEST(test_basic);
	CPPUNIT_TEST(test_saturation);
	CPPUNIT_TEST(test_expiration);
	CPPUNIT_TEST_SUITE_END();
	
	void test_basic(void);
	void test_saturation(void);
	void test_expiration(void);
	
	datapacket_storage *dps;
	datapacket_t pkt[STORE_SIZE], *recv;
	storeid id[STORE_SIZE];
	
public:
	void setUp(void);
	void tearDown(void);
};

void DataPacketStorageTestCase::setUp(){
	fprintf(stderr,"<%s:%d> ************** Set up ************\n",__func__,__LINE__);
	dps = new datapacket_storage(STORE_SIZE, EXPIRATION_SEC);
}

void DataPacketStorageTestCase::tearDown(){
	fprintf(stderr,"<%s:%d> ************** Tear Down ************\n",__func__,__LINE__);
	delete dps;
}

/* Tests */
void DataPacketStorageTestCase::test_basic(void )
{
	fprintf(stderr,"<%s:%d> ************** Test Basic ************\n",__func__,__LINE__);
	
	id[0] = dps->store_packet(&pkt[0]);
	
	CPPUNIT_ASSERT(dps->get_storage_size() == 1);
	
	recv = dps->get_packet(id[0]);
	
	CPPUNIT_ASSERT(recv == &pkt[0]);
	CPPUNIT_ASSERT(dps->get_storage_size() == 0);
}

void DataPacketStorageTestCase::test_saturation(void )
{
	int i, error;
	fprintf(stderr,"<%s:%d> ************** Test Saturation ************\n",__func__,__LINE__);
	
	for(i=0;i<STORE_SIZE;i++)
	{
		id[i] = dps->store_packet(&pkt[i]);
	}
	
	CPPUNIT_ASSERT(dps->get_storage_size() == STORE_SIZE);
	
	error = dps->store_packet(&pkt[0]);
	
	CPPUNIT_ASSERT(error<0);
	
	for(i=0;i<STORE_SIZE;i++)
	{
		recv = dps->get_packet(id[i]);
		CPPUNIT_ASSERT(recv == &pkt[i]);
	}
	CPPUNIT_ASSERT(dps->get_storage_size() == 0);
}

void DataPacketStorageTestCase::test_expiration(void)
{
	bool res;
	storeid res_id;
	//store a packet
	fprintf(stderr,"<%s:%d> ************** Test expiration ************\n",__func__,__LINE__);
	id[0] = dps->store_packet(&pkt[0]);
	//check time
	//fprintf(stderr,"<%s:%d> time stamp = %lu\n",);
	//chek does not need to be expired
	res = dps->oldest_packet_needs_expiration(&res_id);
	fprintf(stderr,"<%s:%d> oldest packet needs exp (1) = %d\n",__func__,__LINE__,res);
	CPPUNIT_ASSERT(res==false);
	//sleep
	sleep(EXPIRATION_SEC+1);
	
	// check that it now needs to expire
	res = dps->oldest_packet_needs_expiration(&res_id);
	fprintf(stderr,"<%s:%d> oldest packet needs exp (2) = %d\n",__func__,__LINE__,res);
	CPPUNIT_ASSERT(res==true);
	
	dps->get_packet(res_id);
	res = dps->oldest_packet_needs_expiration(&res_id);
	fprintf(stderr,"<%s:%d> extracted packet and recheck = %d\n",__func__,__LINE__,res);
	CPPUNIT_ASSERT(res==false);
}

/*
* Test MAIN
*/
int main( int argc, char* argv[] )
{
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(DataPacketStorageTestCase::suite()); // Add the top suite to the test runner
	runner.setOutputter(
			new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	// Run the test and don't wait a key if post build check.
	bool wasSuccessful = runner.run( "" );
	
	std::cerr<<"************** Test finished ************"<<std::endl;

	// Return error code 1 if the one of test failed.
	return wasSuccessful ? 0 : 1;
}
