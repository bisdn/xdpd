//Guards used only when inlining
#ifndef PTHREAD_LOCK_IMPL_INLINE__
#define PTHREAD_LOCK_IMPL_INLINE__

#include "lock.h"
#include <pthread.h>
#include <rofl/datapath/pipeline/platform/memory.h>

/* MUTEX operations */
//Init&destroy
LOCK_INLINE__ platform_mutex_t* platform_mutex_init(void* params){

	pthread_mutex_t* mutex = (pthread_mutex_t*)platform_malloc_shared(sizeof(pthread_mutex_t));

	if(!mutex)
		return NULL;

	if( pthread_mutex_init((pthread_mutex_t*)mutex, (pthread_mutexattr_t*)params) < 0){
		platform_free_shared((pthread_mutex_t*)mutex);
		return NULL;
	}

	return (platform_mutex_t*)mutex;
}

LOCK_INLINE__ void platform_mutex_destroy(platform_mutex_t* mutex){
	pthread_mutex_destroy((pthread_mutex_t*)mutex);
	platform_free_shared((pthread_mutex_t*)mutex);
}

//Operations
LOCK_INLINE__ void platform_mutex_lock(platform_mutex_t* mutex){
	pthread_mutex_lock((pthread_mutex_t*)mutex);
}

LOCK_INLINE__ void platform_mutex_unlock(platform_mutex_t* mutex){
	pthread_mutex_unlock((pthread_mutex_t*)mutex);
}


/* RWLOCK */
//Init&destroy
LOCK_INLINE__ platform_rwlock_t* platform_rwlock_init(void* params){

	pthread_rwlock_t* rwlock = (pthread_rwlock_t*)platform_malloc_shared(sizeof(pthread_rwlock_t));

	if(!rwlock)
		return NULL;

	if(pthread_rwlock_init((pthread_rwlock_t*)rwlock, (pthread_rwlockattr_t*)params) < 0){
		platform_free_shared((pthread_rwlock_t*)rwlock);
		return NULL;
	}
	
	return (platform_rwlock_t*)rwlock;
}

LOCK_INLINE__ void platform_rwlock_destroy(platform_rwlock_t* rwlock){
	pthread_rwlock_destroy((pthread_rwlock_t*)rwlock);
	platform_free_shared((pthread_rwlock_t*)rwlock);
}

//Read
LOCK_INLINE__ void platform_rwlock_rdlock(platform_rwlock_t* rwlock){
	pthread_rwlock_rdlock((pthread_rwlock_t*)rwlock);
}

LOCK_INLINE__ void platform_rwlock_rdunlock(platform_rwlock_t* rwlock){
	pthread_rwlock_unlock((pthread_rwlock_t*)rwlock);
}


//Write
LOCK_INLINE__ void platform_rwlock_wrlock(platform_rwlock_t* rwlock){
	pthread_rwlock_wrlock((pthread_rwlock_t*)rwlock);
}
LOCK_INLINE__ void platform_rwlock_wrunlock(platform_rwlock_t* rwlock){
	pthread_rwlock_unlock((pthread_rwlock_t*)rwlock);
}

#endif //Guards 
