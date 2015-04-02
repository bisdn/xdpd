#include "time_utils.h"

/**
 * @name get_time_difference_ms
 * @brief returns the time difference between 2 timeval structs in ms
 * @param now latest time
 * @param last oldest time
 */
uint64_t get_time_difference_ms(struct timeval *now, struct timeval *last)
{
	/*diff = now -last; now > last !!*/
	struct timeval res;
	timersub(now,last,&res);
	
	return res.tv_sec * 1000 + res.tv_usec/1000;
}
