//Guards used only when inlining
#ifndef PTHREAD_LOCK_IMPL_INLINE
#define PTHREAD_LOCK_IMPL_INLINE

#include <pthread.h>

//Set 1 when inlining them in the pipeline
#define GNU_LINUX_INLINE_LOCKING 0

#if GNU_LINUX_INLINE_LOCKING == 1
	#define STATIC_LOCK_INLINE__ static inline
	typedef void platform_mutex_t;
	typedef void platform_rwlock_t;
#else
	#define STATIC_LOCK_INLINE__ 
	#include <rofl/datapath/pipeline/platform/lock.h>
#endif

#include <rofl/datapath/pipeline/platform/memory.h>


/* MUTEX operations */
//Init&destroy
STATIC_LOCK_INLINE__ platform_mutex_t* platform_mutex_init(void* params){

	pthread_mutex_t* mutex = (pthread_mutex_t*)platform_malloc_shared(sizeof(pthread_mutex_t));

	if(!mutex)
		return NULL;

	if( pthread_mutex_init(mutex, params) < 0){
		platform_free_shared(mutex);
		return NULL;
	}

	return (platform_mutex_t*)mutex;
}

STATIC_LOCK_INLINE__ void platform_mutex_destroy(platform_mutex_t* mutex){
	pthread_mutex_destroy(mutex);
	platform_free_shared(mutex);
}

//Operations
STATIC_LOCK_INLINE__ void platform_mutex_lock(platform_mutex_t* mutex){
	pthread_mutex_lock(mutex);
}

STATIC_LOCK_INLINE__ void platform_mutex_unlock(platform_mutex_t* mutex){
	pthread_mutex_unlock(mutex);
}


/* RWLOCK */
//Init&destroy
STATIC_LOCK_INLINE__ platform_rwlock_t* platform_rwlock_init(void* params){

	pthread_rwlock_t* rwlock = (pthread_rwlock_t*)platform_malloc_shared(sizeof(pthread_rwlock_t));

	if(!rwlock)
		return NULL;

	if(pthread_rwlock_init(rwlock, params) < 0){
		platform_free_shared(rwlock);
		return NULL;
	}
	
	return (platform_rwlock_t*)rwlock;
}

STATIC_LOCK_INLINE__ void platform_rwlock_destroy(platform_rwlock_t* rwlock){
	pthread_rwlock_destroy(rwlock);
	platform_free_shared(rwlock);
}

//Read
STATIC_LOCK_INLINE__ void platform_rwlock_rdlock(platform_rwlock_t* rwlock){
	pthread_rwlock_rdlock(rwlock);
}

STATIC_LOCK_INLINE__ void platform_rwlock_rdunlock(platform_rwlock_t* rwlock){
	pthread_rwlock_unlock(rwlock);
}


//Write
STATIC_LOCK_INLINE__ void platform_rwlock_wrlock(platform_rwlock_t* rwlock){
	pthread_rwlock_wrlock(rwlock);
}
STATIC_LOCK_INLINE__ void platform_rwlock_wrunlock(platform_rwlock_t* rwlock){
	pthread_rwlock_unlock(rwlock);
}


#endif //Guards used only when inlining
