/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GROUP_TABLE_SNAPSHOT_H
#define GROUP_TABLE_SNAPSHOT_H 

#include <iostream>
#include <string> 
#include <sstream>
#include <list> 
#include <rofl.h>
//#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_statistics.h>

/**
* @file group_table_snapshot.h 
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief C++ group table snapshot
*/

namespace xdpd {

class bucket_snapshot{
	
public:
	bucket_snapshot(of1x_stats_bucket_t* stats, of1x_stats_bucket_desc_msg_t* desc):
		weight(desc->weight),
		port(desc->port),
		group(desc->group),
		packet_count(stats->packet_count),
		byte_count(stats->byte_count)
	{};
	
	//Dumping operator
	//friend std::ostream& operator<<() {}
	
	uint16_t weight;
	uint32_t port;
	uint32_t group;
	of1x_action_group_t* actions;
	
	//stats
	uint64_t packet_count;
	uint64_t byte_count;
};
	
class openflow_group_table_snapshot{
public:
	
	openflow_group_table_snapshot(of1x_stats_group_msg_t* stats, of1x_stats_group_desc_msg_t* desc):
		group_id(stats->group_id),
		type(desc->type),
		num_of_buckets(stats->num_of_buckets),
		packet_count(stats->packet_count),
		byte_count(stats->byte_count)
	{};
	
	uint32_t group_id;
	of1x_group_type_t type;
	int num_of_buckets;
	
	//stats
	uint64_t packet_count;
	uint64_t byte_count;
};

}// namespace xdpd

#endif //GROUP_TABLE_SNAPSHOT_H