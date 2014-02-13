#include <rofl/datapath/pipeline/platform/atomic_operations.h>

//Using GCC atomic built-ins when available
//Defining them as static inline, such that this file can be symlinked 
//when compiling ROFL-pipeline to inline them

//Uncomment this line when you are inlining them in the pipeline
//#define STATIC_INLINE__ static inline
#define STATIC_INLINE__ 

/// these functins increase by one the counter
STATIC_INLINE__ void platform_atomic_inc64(uint64_t* counter, platform_mutex_t* mutex)
{
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	__sync_add_and_fetch(counter, 1UL);
#else
	platform_mutex_lock(mutex);
	(*counter)++;
	platform_mutex_unlock(mutex);
#endif	
}

STATIC_INLINE__ void platform_atomic_inc32(uint32_t* counter, platform_mutex_t* mutex)
{
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	__sync_add_and_fetch(counter, 1);
#else
	platform_mutex_lock(mutex);
	(*counter)++;
	platform_mutex_unlock(mutex);
#endif	
}

/// These functions add value to the counter
STATIC_INLINE__ void platform_atomic_add64(uint64_t* counter, uint64_t* value, platform_mutex_t* mutex)
{
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	__sync_add_and_fetch(counter, value);
#else
	platform_mutex_lock(mutex);
	(*counter)+=(*value);
	platform_mutex_unlock(mutex);
#endif
}

STATIC_INLINE__ void platform_atomic_add32(uint32_t* counter, uint32_t* value, platform_mutex_t* mutex)
{
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	__sync_add_and_fetch(counter, value);
#else
	platform_mutex_lock(mutex);
	(*counter)+=(*value);
	platform_mutex_unlock(mutex);
#endif
}

STATIC_INLINE__ void platform_atomic_dec32(uint32_t* counter, platform_mutex_t* mutex)
{
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	__sync_sub_and_fetch(counter, 1);
#else
	platform_mutex_lock(mutex);
	(*counter)--;
	platform_mutex_unlock(mutex);
#endif
}
