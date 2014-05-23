/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PIRL_H
#define PIRL_H 

/**
* @file pirl.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Packet-In Rate Limiter (PIRL)
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <rofl/datapath/pipeline/common/packet_matches.h>

#include "pirl_config.h"

//Value to disable max rate limiter (both)
#define PIRL_DISABLED -1

//Number 
#define PIRL_NUMBER_OF_BUCKETS_PER_S 10
#define PIRL_MIN_RATE PIRL_NUMBER_OF_BUCKETS_PER_S 


/**
* Bucket
*/
typedef struct pirl_bucket{
	uint64_t last_seen_ts;
	int tokens;
}pirl_bucket_t;

/**
* PIRL instance state
*/
typedef struct pirl{

	//Total flows
	pirl_bucket_t total_bucket;

	//Per flow buckets
	//FIXME

	//configured max rates
	int max_rate;
	int max_rate_per_flow; 
}pirl_t;

//C++ extern C
ROFL_BEGIN_DECLS


//Mgmt

/**
* Create an instance of PIRL
*/
pirl_t* init_pirl(const int max_rate, const int max_rate_per_flow);

/**
* Destroy PIRL instance
*/
void destroy_pirl(pirl_t* pirl);

/**
* Reconfigure PIRL's max_rates
*/ 
rofl_result_t reconfigure_pirl(const int new_max_rate, const int new_max_rate_per_flow);


//Filtering

//Reset
static inline void reset_bucket(pirl_t* pirl, pirl_bucket_t* bucket, int curr_ts){
	bucket->last_seen_ts = curr_ts;
	bucket->tokens = pirl->max_rate/PIRL_NUMBER_OF_BUCKETS_PER_S;
}

//Increment
static inline bool pirl_dec(pirl_bucket_t* bucket){
#ifdef PIRL_WITH_LOCKING
	do{
		int orig = bucket->tokens;
		if(orig == 0)
			return false;
		orig_dec = orig-1;
	}while(__sync_bool_compare_and_swap(&bucket->tokens, orig, orig_dec) != true);
	return true;
#else
	if(bucket->tokens > 0){
		bucket->tokens--;
		return true;
	}
#endif
	
	return false;
}

/**
* Apply PIRL rate limiting. To a certain packet.
*
* @return A boolean; true when should be dropped.
*/
static inline bool pirl_filter_pkt(pirl_t* pirl, packet_matches_t* matches){

	uint64_t curr_ts = pirl_clock_gettime_ms()/100; 
	
	//Total rate limiting
	if(pirl->max_rate != PIRL_DISABLED){
		//Check if we have to reset
		if(pirl->total_bucket.last_seen_ts != curr_ts){
			reset_bucket(pirl, &pirl->total_bucket, curr_ts);
		}
		//Decrement tokens
		if(pirl_dec(&pirl->total_bucket) == false)
			return true;
	}else{
		//If max rate is disabled then per flow is also disabled => don't drop
		return false;
	}

	//Per flow rate limiting
	if(pirl->max_rate_per_flow != PIRL_DISABLED){
		//FIXME:
	}

	return false;	
} 

//C++ extern C
ROFL_END_DECLS

#endif //PIRL
