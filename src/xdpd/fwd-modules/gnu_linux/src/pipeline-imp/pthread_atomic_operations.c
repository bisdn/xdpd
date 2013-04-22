#include <rofl/datapath/pipeline/platform/atomic_operations.h>


/// these functins increase by one the counter
void platform_atomic_inc64(uint64_t* counter, platform_mutex_t* mutex)
{
	platform_mutex_lock(mutex);
	(*counter)++;
	platform_mutex_unlock(mutex);
}

void platform_atomic_inc32(uint32_t* counter, platform_mutex_t* mutex)
{
	platform_mutex_lock(mutex);
	(*counter)++;
	platform_mutex_unlock(mutex);
}

/// These functions add value to the counter
void platform_atomic_add64(uint64_t* counter, uint64_t* value, platform_mutex_t* mutex)
{
	platform_mutex_lock(mutex);
	(*counter)+=(*value);
	platform_mutex_unlock(mutex);
}

void platform_atomic_add32(uint32_t* counter, uint32_t* value, platform_mutex_t* mutex)
{
	platform_mutex_lock(mutex);
	(*counter)+=(*value);
	platform_mutex_unlock(mutex);
}

void platform_atomic_dec32(uint32_t* counter, platform_mutex_t* mutex)
{
	platform_mutex_lock(mutex);
	(*counter)--;
	platform_mutex_unlock(mutex);
}
