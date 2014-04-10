//Guards used only when inlining
#ifndef ATOMIC_OPS_IMPL_INLINE__
#define ATOMIC_OPS_IMPL_INLINE__

//Must be the first one
#include <rofl/datapath/pipeline/platform/lock.h>
#include "rte_atomic_operations.h"

#include "../config.h"
#include <rte_config.h>
#include <rte_common.h>
#include <rte_atomic.h>

/// these functins increase by one the counter
STATIC_ATOMIC_INLINE__ void platform_atomic_inc64(uint64_t* counter, platform_mutex_t* mutex)
{
	rte_atomic64_inc((rte_atomic64_t*)counter);
}

STATIC_ATOMIC_INLINE__ void platform_atomic_inc32(uint32_t* counter, platform_mutex_t* mutex)
{
	rte_atomic32_inc((rte_atomic32_t*)counter);
}

/// These functions add value to the counter
STATIC_ATOMIC_INLINE__ void platform_atomic_add64(uint64_t* counter, uint64_t value, platform_mutex_t* mutex)
{
	rte_atomic64_add((rte_atomic64_t*)counter, value);
}

STATIC_ATOMIC_INLINE__ void platform_atomic_add32(uint32_t* counter, uint32_t value, platform_mutex_t* mutex)
{
	rte_atomic32_add((rte_atomic32_t*)counter, value);
}

STATIC_ATOMIC_INLINE__ void platform_atomic_dec32(uint32_t* counter, platform_mutex_t* mutex)
{
	rte_atomic32_dec((rte_atomic32_t*)counter);
}

#endif //Guards
