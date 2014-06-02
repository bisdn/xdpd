#include "pirl.h"

using namespace xdpd;

//Create an instance of PIRL
pirl::pirl(const int max_rate){
	if(reconfigure(max_rate) != ROFL_SUCCESS){
		//TODO: log
		throw ePIRLInvalidConf();	
	}
}

/**
* Reconfigure PIRL's max_rates
*/ 
rofl_result_t pirl::reconfigure(const int new_max_rate){
	if(new_max_rate < PIRL_MIN_RATE && new_max_rate != PIRL_DISABLED){
		//FIXME: log
		return ROFL_FAILURE;	
	}

	//Set rate
	bucket.last_seen_ts = 0;
	max_rate = new_max_rate;

	return ROFL_SUCCESS;
}
