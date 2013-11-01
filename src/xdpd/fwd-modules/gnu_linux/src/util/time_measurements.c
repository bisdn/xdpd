#include "time_measurements.h"

//Initialize global stats
time_measurements_t global_measurements = {0};


void tm_dump_measurements(void){

	ROFL_INFO("\nGNU/Linux time measurements summary\n");
	ROFL_INFO("-----------------------------------\n\n");

	//Do the calculations for stage time

	
	//Final stats
	ROFL_INFO("Total number of packets: %u\n",global_measurements.total_pkts);
}
