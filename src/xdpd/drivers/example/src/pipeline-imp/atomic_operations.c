/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <rofl/datapath/pipeline/platform/lock.h>
#include <rofl/datapath/pipeline/platform/atomic_operations.h>

/// these functins increase by one the counter
void platform_atomic_inc64(uint64_t* counter, platform_mutex_t* mutex)
{
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	__sync_add_and_fetch(counter, 1UL);
#else
	platform_mutex_lock(mutex);
	(*counter)++;
	platform_mutex_unlock(mutex);
#endif	
}

void platform_atomic_inc32(uint32_t* counter, platform_mutex_t* mutex)
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
void platform_atomic_add64(uint64_t* counter, uint64_t value, platform_mutex_t* mutex)
{
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	__sync_add_and_fetch(counter, value);
#else
	platform_mutex_lock(mutex);
	(*counter)+=value;
	platform_mutex_unlock(mutex);
#endif
}

void platform_atomic_add32(uint32_t* counter, uint32_t value, platform_mutex_t* mutex)
{
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	__sync_add_and_fetch(counter, value);
#else
	platform_mutex_lock(mutex);
	(*counter)+=value;
	platform_mutex_unlock(mutex);
#endif
}

void platform_atomic_dec32(uint32_t* counter, platform_mutex_t* mutex)
{
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	__sync_sub_and_fetch(counter, 1);
#else
	platform_mutex_lock(mutex);
	(*counter)--;
	platform_mutex_unlock(mutex);
#endif
}
