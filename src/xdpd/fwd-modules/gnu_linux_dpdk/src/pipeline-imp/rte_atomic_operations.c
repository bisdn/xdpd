#include <rofl/datapath/pipeline/platform/atomic_operations.h>
#include "../config.h"
#include <rte_config.h>
#include <rte_common.h>
#include <rte_atomic.h>



/// these functins increase by one the counter
void platform_atomic_inc64(uint64_t* counter, platform_mutex_t* mutex)
{
	rte_atomic64_inc((rte_atomic64_t*)counter);
}

void platform_atomic_inc32(uint32_t* counter, platform_mutex_t* mutex)
{
	rte_atomic32_inc((rte_atomic32_t*)counter);
}

/// These functions add value to the counter
void platform_atomic_add64(uint64_t* counter, uint64_t value, platform_mutex_t* mutex)
{
	rte_atomic64_add((rte_atomic64_t*)counter, value);
}

void platform_atomic_add32(uint32_t* counter, uint32_t value, platform_mutex_t* mutex)
{
	rte_atomic32_add((rte_atomic32_t*)counter, value);
}

void platform_atomic_dec32(uint32_t* counter, platform_mutex_t* mutex)
{
	rte_atomic32_dec((rte_atomic32_t*)counter);
}
