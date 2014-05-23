/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PIRL_CONFIG_H
#define PIRL_CONFIG_H 

/**
* @file pirl_config.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief PIRL settings 
*
* The only reason for this file to exist is to be able to reuse pirl.h/pirl.c without any change in several platforms
*
*/

#include <stdint.h>
#include <time.h>
#include "../../util/likely.h"

/**
* Returns the number of ms since epoch
*/
static inline uint64_t pirl_clock_gettime_ms(){
	struct timespec tp;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &tp);
	
	return tp.tv_sec*1000 + tp.tv_nsec/1000000; 
}


#endif //PIRL_CONFIG
