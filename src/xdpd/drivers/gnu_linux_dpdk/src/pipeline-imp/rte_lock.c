//Guards used only when inlining
#ifndef RTE_LOCK_IMPL_INLINE__
#define RTE_LOCK_IMPL_INLINE__

#include "lock.h"
#include <rofl/datapath/pipeline/platform/memory.h>
#include "../config.h"
#include <rte_config.h>
#include <rte_common.h>
#include <rte_atomic.h>
#include <rte_spinlock.h>
#include <rte_rwlock.h>


/* MUTEX operations */
//Init&destroy
LOCK_INLINE__ platform_mutex_t* platform_mutex_init(void* params){

	rte_spinlock_t* mutex = (rte_spinlock_t*)platform_malloc_shared(sizeof(rte_spinlock_t));

	if(!mutex)
		return NULL;

	rte_spinlock_init((rte_spinlock_t*)mutex);

	return (platform_mutex_t*)mutex;
}

LOCK_INLINE__ void platform_mutex_destroy(platform_mutex_t* mutex){
	platform_free_shared(mutex);
}

//Operations
LOCK_INLINE__ void platform_mutex_lock(platform_mutex_t* mutex){
	rte_spinlock_lock((rte_spinlock_t*)mutex);
}

LOCK_INLINE__ void platform_mutex_unlock(platform_mutex_t* mutex){
	rte_spinlock_unlock((rte_spinlock_t*)mutex);
}


/* RWLOCK */
//Init&destroy
LOCK_INLINE__ platform_rwlock_t* platform_rwlock_init(void* params){

	rte_rwlock_t* rwlock = (rte_rwlock_t*)platform_malloc_shared(sizeof(rte_rwlock_t));

	if(!rwlock)
		return NULL;
	
	rte_rwlock_init((rte_rwlock_t*)rwlock);
	
	return (platform_rwlock_t*)rwlock;
}

LOCK_INLINE__ void platform_rwlock_destroy(platform_rwlock_t* rwlock){
	platform_free_shared(rwlock);
}

//Read
LOCK_INLINE__ void platform_rwlock_rdlock(platform_rwlock_t* rwlock){
	rte_rwlock_read_lock((rte_rwlock_t*)rwlock);
}

LOCK_INLINE__ void platform_rwlock_rdunlock(platform_rwlock_t* rwlock){
	rte_rwlock_read_unlock((rte_rwlock_t*)rwlock);
}


//Write
LOCK_INLINE__ void platform_rwlock_wrlock(platform_rwlock_t* rwlock){
	rte_rwlock_write_lock((rte_rwlock_t*)rwlock);
}
LOCK_INLINE__ void platform_rwlock_wrunlock(platform_rwlock_t* rwlock){
	rte_rwlock_write_unlock((rte_rwlock_t*)rwlock);
}

#endif //Guards 
