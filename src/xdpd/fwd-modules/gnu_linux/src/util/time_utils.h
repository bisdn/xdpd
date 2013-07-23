/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TIME_UTILS_H
#define TIME_UTILS_H 1

#include <string.h>
#include <rofl.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>

//Extern C
ROFL_BEGIN_DECLS

uint64_t get_time_difference_ms(struct timeval *now, struct timeval *last);

//Extern C
ROFL_END_DECLS

#endif /* TIME_UTILS_H_ */
