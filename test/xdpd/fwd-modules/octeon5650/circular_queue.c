#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "../../src/hal_octeon5650/platform_messages.h"
#include "cutil.h"


void test_insert_and_extract_messages(driver_message_t *messages)
{
	driver_message_ring_t *ring;
	ring = init_driver_message_ring();
	driver_message_t rx_message;
	
	int i;
	
	for(i=0; i<MESSAGES_RING_QUEUE_MAX_ELEMENTS;i++)
	{
		if(i==MESSAGES_RING_QUEUE_MAX_ELEMENTS-1)
			assert(	driver_message_write(ring, messages[i])!=0);
		else
			assert(	driver_message_write(ring, messages[i])==0);
	}
	
	for (i=0; i<MESSAGES_RING_QUEUE_MAX_ELEMENTS;i++)
	{
		if(i==MESSAGES_RING_QUEUE_MAX_ELEMENTS-1)
		{
			assert(driver_message_read(ring,&rx_message)!=0);
		}
		else
		{
			assert(driver_message_read(ring,&rx_message)==0);
			//fprintf(stderr, "<%s:%d> message type %d i=%d\n",__func__, __LINE__, rx_message.type,i);
			assert(rx_message.type == i);
		}
	}
	
	driver_message_ring_destroy(ring);
	
	fprintf(stderr,"<%s:%d> Test passed\n",__func__,__LINE__);
}

void test_random_messages(driver_message_ring_t *ring ,driver_message_t *messages)
{
	int i,rand_num;
	static int message_cnt=0;
	driver_message_t rx_message;
	int index = 33;
	
	rand_num = rand()%(2*MESSAGES_RING_QUEUE_MAX_ELEMENTS);
	
	//insert
	for(i=0;i<rand_num;i++)
	{
		if(message_cnt+1==MESSAGES_RING_QUEUE_MAX_ELEMENTS)
			assert(driver_message_write(ring,messages[index])!=0);
		else
		{
			assert(driver_message_write(ring,messages[index])==0);
			message_cnt++;
		}
	}
	
	//fprintf(stderr,"<%s:%d> Test passed (inserted: %d)\n",__func__,__LINE__, rand_num);
	
	rand_num = rand()%(2*MESSAGES_RING_QUEUE_MAX_ELEMENTS);
	
	//extract
	for(i=0;i<rand_num;i++)
	{
		if(message_cnt==0)
			assert(driver_message_read(ring,&rx_message)!=0);
		else
		{
			assert(driver_message_read(ring,&rx_message)==0);
			assert(rx_message.type==index);
			message_cnt--;
		}
	}
	
	//fprintf(stderr,"<%s:%d> Test passed (extracted: %d)\n",__func__,__LINE__, rand_num);
}

void setup_test(driver_message_t * messages)
{
	int i;
	for (i=0;i<MESSAGES_RING_QUEUE_MAX_ELEMENTS;i++)
		messages[i].type = i;
}

void cleanup_test()
{
	
}

int main(int argc, char *argv[])
{
	int i;
	driver_message_t messages[MESSAGES_RING_QUEUE_MAX_ELEMENTS];
	driver_message_ring_t *ring;
	setup_test(messages);

	test_insert_and_extract_messages(messages);
	
	ring = init_driver_message_ring();
	for(i=0;i<50;i++)
		test_random_messages(ring, messages);
	driver_message_ring_destroy(ring);
	fprintf(stderr,"<%s:%d> Random Test passed\n",__func__,__LINE__);
	
	fprintf(stderr,"<%s:%d> Tests for circular queue passed\n",__func__,__LINE__);
	return EXIT_SUCCESS;
}