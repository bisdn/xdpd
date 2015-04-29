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
#include <rofl_datapath.h>
#include <rofl/common/croflexception.h>

#include "pirl_config.h"

namespace xdpd {

class ePIRLBase				: public rofl::RoflException {};	// base error class for all switch_manager related errors
class ePIRLInvalidConf			: public ePIRLBase {};

/**
* @brief Packet in rate limiting class (PIRL) 
* @ingroup cmm_of
*
* @description Applies a token bucket strategy to rate limit PKT_IN events 
*/
class pirl{ 
	
	//Definition of a bucket
	typedef struct pirl_bucket{
		uint64_t last_seen_ts;
		int tokens;
	}pirl_bucket_t;


	//Total flows
	pirl_bucket_t bucket;

	//configured max rates
	int max_rate;

	
public:
	//Constants
	static const int PIRL_DISABLED=-1;

	//Number 
	static const int PIRL_DEFAULT_MAX_RATE=1024; //PKT_IN/s
	static const int PIRL_NUMBER_OF_BUCKETS_PER_S=10;
	static const int PIRL_MIN_RATE=PIRL_NUMBER_OF_BUCKETS_PER_S; 


	//Mgmt
	/**
	* Create an instance of PIRL
	*/
	pirl(const int max_rate=PIRL_DEFAULT_MAX_RATE);

	/**
	* Reconfigure PIRL's max_rate on a running instance
	*/ 
	rofl_result_t reconfigure(const int new_max_rate);

	/**
	* Apply PIRL rate limiting. To a certain packet.
	*
	* @return A boolean; true when should be dropped.
	*/
	inline bool filter_pkt(){

		uint64_t curr_ts = pirl_clock_gettime_ms()/100; 
		
		//Total rate limiting
		if(max_rate != PIRL_DISABLED){
			//Check if we have to reset
			if(bucket.last_seen_ts != curr_ts){
				reset_bucket(curr_ts);
			}
			//Decrement tokens
			if(pirl_dec() == false)
				return true;
		}
		
		return false;	
	} 

private:


	//Reset
	inline void reset_bucket(int curr_ts){
		bucket.last_seen_ts = curr_ts;
		bucket.tokens = max_rate/PIRL_NUMBER_OF_BUCKETS_PER_S;
	}

	//Increment
	inline bool pirl_dec(){
#ifdef PIRL_WITH_LOCKING
		do{
			int orig = bucket.tokens;
			if(orig == 0)
				return false;
			orig_dec = orig-1;
		}while(__sync_bool_compare_and_swap(&bucket.tokens, orig, orig_dec) != true);
		return true;
#else
		if(bucket.tokens > 0){
			bucket.tokens--;
			return true;
		}
#endif
		
		return false;
	}

}; //End of class definition

}// namespace xdpd

#endif //PIRL
