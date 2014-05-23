#include "pirl.h"

//Create an instance of PIRL
pirl_t* init_pirl(const int max_rate, const int max_rate_per_flow){

	pirl_t* pirl;

	if(max_rate < PIRL_MIN_RATE){
		//FIXME: log
		return NULL;
	}

	if(max_rate == PIRL_DISABLED && max_rate_per_flow != PIRL_DISABLED){
		//FIXME: log
		return NULL;
	}

 	pirl = (pirl_t*)malloc(sizeof(pirl_t));	
	
	//Set rate
	pirl->total_bucket.last_seen_ts = 0;
	pirl->max_rate = max_rate;
	pirl->max_rate_per_flow = max_rate_per_flow;

	return pirl;
}

/**
* Destroy PIRL instance
*/
void destroy_pirl(pirl_t* pirl){
	free(pirl);
}

