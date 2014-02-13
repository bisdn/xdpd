#include <rofl/datapath/pipeline/platform/lock.h>
#include <rofl/datapath/pipeline/platform/memory.h>

#include <pthread.h>

/*
*
* pthread compatible platform_lock interface implementation
*      Author: msune
*
*/

//Uncomment this line when you are inlining them in the pipeline
//#define STATIC_INLINE__ static inline
#define STATIC_INLINE__ 

/* MUTEX operations */
//Init&destroy
STATIC_INLINE__ platform_mutex_t* platform_mutex_init(void* params){

	pthread_mutex_t* mutex = (pthread_mutex_t*)platform_malloc_shared(sizeof(pthread_mutex_t));

	if(!mutex)
		return NULL;

	if( pthread_mutex_init(mutex, params) < 0){
		platform_free_shared(mutex);
		return NULL;
	}

	return (platform_mutex_t*)mutex;
}

STATIC_INLINE__ void platform_mutex_destroy(platform_mutex_t* mutex){
	pthread_mutex_destroy(mutex);
	platform_free_shared(mutex);
}

//Operations
STATIC_INLINE__ void platform_mutex_lock(platform_mutex_t* mutex){
	pthread_mutex_lock(mutex);
}

STATIC_INLINE__ void platform_mutex_unlock(platform_mutex_t* mutex){
	pthread_mutex_unlock(mutex);
}


/* RWLOCK */
//Init&destroy
STATIC_INLINE__ platform_rwlock_t* platform_rwlock_init(void* params){

	pthread_rwlock_t* rwlock = (pthread_rwlock_t*)platform_malloc_shared(sizeof(pthread_rwlock_t));

	if(!rwlock)
		return NULL;

	if(pthread_rwlock_init(rwlock, params) < 0){
		platform_free_shared(rwlock);
		return NULL;
	}
	
	return (platform_rwlock_t*)rwlock;
}

STATIC_INLINE__ void platform_rwlock_destroy(platform_rwlock_t* rwlock){
	pthread_rwlock_destroy(rwlock);
	platform_free_shared(rwlock);
}

//Read
STATIC_INLINE__ void platform_rwlock_rdlock(platform_rwlock_t* rwlock){
	pthread_rwlock_rdlock(rwlock);
}

STATIC_INLINE__ void platform_rwlock_rdunlock(platform_rwlock_t* rwlock){
	pthread_rwlock_unlock(rwlock);
}


//Write
STATIC_INLINE__ void platform_rwlock_wrlock(platform_rwlock_t* rwlock){
	pthread_rwlock_wrlock(rwlock);
}
STATIC_INLINE__ void platform_rwlock_wrunlock(platform_rwlock_t* rwlock){
	pthread_rwlock_unlock(rwlock);
}
