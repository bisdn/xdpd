#include "ringbuffer.h"

ringbuffer::ringbuffer()
{
	//Set 0 structure
	memset(&buffer, 0, sizeof(buffer));

	//Set pointers
	writep = buffer;
	readp = buffer;

	//Setting set
	state = RB_BUFFER_AVAILABLE;
	this->full_limit = FULL_LIMIT * MAX_SLOTS;
	this->critical_full_limit = CRITICALLY_FULL_LIMIT * MAX_SLOTS;

	//Init conditions
	pthread_cond_init(&write_cond, NULL);
	pthread_cond_init(&read_cond, NULL);

	//Mutexes
	pthread_mutex_init(&mutex_readers, NULL);
	pthread_mutex_init(&mutex_writers, NULL);
}

ringbuffer::~ringbuffer()
{
	//Destroy
	pthread_cond_destroy(&write_cond);
	pthread_cond_destroy(&read_cond);
	pthread_mutex_destroy(&mutex_readers);
	pthread_mutex_destroy(&mutex_writers);
}

//Read
datapacket_t* ringbuffer::non_blocking_read(void)
{
	datapacket_t* pkt;

#ifdef RB_ASM_IMP

	//TODO: put a pthread_mutex free imp

#else	
#ifdef RB_MULTI_READERS
	pthread_mutex_lock(&mutex_readers);
#endif
	if (is_empty()) {
#ifdef RB_MULTI_WRITERS
		pthread_mutex_unlock(&mutex_readers);
#endif
		return NULL;
	}

	pkt = *readp;
	circ_inc_pointer(&readp); // = (readp + 1) % MAX_SLOTS;

#ifdef RB_MULTI_READERS
	pthread_mutex_unlock(&mutex_readers);
#endif

	update_buffer_state();

	pthread_cond_broadcast(&write_cond);
	return pkt;
#endif
}

datapacket_t* ringbuffer::blocking_read(unsigned int seconds)
{
	datapacket_t* pkt;
	struct timespec timeout;

	//memset it(be nice to valgrind)
	memset(&timeout,0,sizeof(timeout));
	
	//Try it straight away
	pkt = non_blocking_read();
	
	while(!pkt) {

		//Acquire lock for pthread_cond_wait
		pthread_mutex_lock(&mutex_readers);

		//Sleep until signal or timeout (in case defined)
		if(seconds){
			timeout.tv_sec = time(NULL) + seconds;
			pthread_cond_timedwait(&read_cond, &mutex_readers,&timeout);
		}else	
			pthread_cond_wait(&read_cond, &mutex_readers);

		//Release it
		pthread_mutex_unlock(&mutex_readers);


		//Retry
		pkt = non_blocking_read();
		
		if(seconds)
			//if timeout, then only once needs to be tried and exit
			break;
	
	}
	return pkt;
}

//Write
int ringbuffer::non_blocking_write(datapacket_t* pkt)
{
#ifdef RB_ASM_IMP

	//TODO: put a pthread_mutex free imp

#else

#ifdef RB_MULTI_WRITERS
	pthread_mutex_lock(&mutex_writers);
#endif

	if (is_full()) {
#ifdef RB_MULTI_WRITERS
		pthread_mutex_unlock(&mutex_writers);
#endif
		return RB_FAILURE;
	}

	*writep = pkt;
	circ_inc_pointer(&writep); // = (readp + 1) % MAX_SLOTS;

#ifdef RB_MULTI_WRITERS
	pthread_mutex_unlock(&mutex_writers);
#endif
	update_buffer_state();

	pthread_cond_broadcast(&read_cond);

	return RB_SUCCESS;
#endif
}

int ringbuffer::blocking_write(datapacket_t* pkt, unsigned int seconds)
{
	int result;
	struct timespec timeout;

	//Try it straight away
	result = non_blocking_write(pkt);

	while(result == RB_FAILURE) {
	
		//Acquire lock for pthread_cond_wait
		pthread_mutex_lock(&mutex_writers);

		//Sleep until signal or timeout (in case defined)
		if(seconds){
			timeout.tv_sec = time(NULL) + seconds;
			pthread_cond_timedwait(&write_cond, &mutex_writers, &timeout);
		}else
	 		pthread_cond_wait(&write_cond, &mutex_writers);

		//Release it
		pthread_mutex_unlock(&mutex_writers);

		//Retry
		result = non_blocking_write(pkt);
		
		if(seconds)
			//if timeout, then only once needs to be tried and exit
			break;
	}

	return result;
}

#if 0
inline ringbuffer_state_t ringbuffer::get_buffer_state() {
	return state;
}
#endif
