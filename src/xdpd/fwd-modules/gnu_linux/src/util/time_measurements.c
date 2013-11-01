#include "time_measurements.h"

//Initialize global stats
time_measurements_t global_measurements = {0};

static void tm_dump_row(const char* message, uint64_t accumulated_time, uint64_t number_of_packets, uint64_t total_number_of_packets){
	
	ROFL_INFO("%s\t\t%.3f\t%.02f\t%u\n", message, accumulated_time, number_of_packets, total_number_of_packets);
}

void tm_dump_measurements(void){

	unsigned int i;	
//	double tt_fast_path, tt_slow_path;

	ROFL_INFO("\nGNU/Linux time measurements summary\n");
	ROFL_INFO("-----------------------------------\n\n");

	ROFL_INFO("Stage name \t\t| Average time \t| Percentage overall time \t| #pkts\n");

	//Do the calculations for stage time
	for(i=0;i<TM_MAX;i++){
		tm_dump_row("hallo", 1 , 2 ,3);
	}	
	
	ROFL_INFO("\n\n");
	
	//Final stats
//	ROFL_INFO("Average transit fast path: %u\n", average_fast_path);
	ROFL_INFO("Total ticks S1:  (%.2f)\n", (global_measurements.accumulated[1]/global_measurements.total_pkts));
	ROFL_INFO("Total ticks S2:  (%.2f)\n", (global_measurements.accumulated[2]/global_measurements.total_pkts));
	ROFL_INFO("Total ticks S3:  (%.2f)\n", (global_measurements.accumulated[3]/global_measurements.total_pkts));
//	ROFL_INFO("Average transit slow path: %u\n", average_slow_path);
	ROFL_INFO("Total RX packets: %u (100%)\n",global_measurements.total_pkts);
	ROFL_INFO("Total PKT_IN packets: %u (%.2f\%)\n",global_measurements.total_pktin_pkts, ((float)global_measurements.total_pktin_pkts/global_measurements.total_pkts*100) );
	ROFL_INFO("Total TX packets (excluding PKT_OUTs): %u (%.2f\%)\n",global_measurements.total_output_pkts, ((float)global_measurements.total_output_pkts/global_measurements.total_pkts*100));
	ROFL_INFO("Total Dropped trying to enqueue to switch(s): %u (%.2f\%)\n",global_measurements.total_S3_dropped_pkts, ((float)global_measurements.total_S3_dropped_pkts/global_measurements.total_pkts*100));
	ROFL_INFO("Total Dropped trying to enqueue to output port(s): %u (%.2f\%)\n",global_measurements.total_SA6_dropped_pkts, ((float)global_measurements.total_SA6_dropped_pkts/global_measurements.total_pkts*100));
	ROFL_INFO("Total Dropped trying to enqueue to PKT_IN queue: %u (%.2f\%)\n\n",global_measurements.total_SB6_dropped_pkts, ((float)global_measurements.total_SB6_dropped_pkts/global_measurements.total_pkts*100));
}
