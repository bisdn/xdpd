/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
