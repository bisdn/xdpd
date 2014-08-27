/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GROUP_MOD_SNAPSHOT_H
#define GROUP_MOD_SNAPSHOT_H 

#include <iostream>
#include <string> 
#include <sstream>
#include <list> 
#include <rofl.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/common/openflow/cofbucket.h>
//#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_statistics.h>

/**
* @file group_table_snapshot.h 
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief C++ group table snapshot
*/

namespace xdpd {

class bucket_snapshot : public rofl::openflow::cofbucket{
	
public:
	
	//stats
	uint64_t packet_count;
	uint64_t byte_count;
	
	bucket_snapshot(of1x_stats_bucket_t* stats, rofl::openflow::cofbucket& bucket);
	rofl_result_t map_bucket_list(of_version_t ver, unsigned int num_of_buckets, of1x_stats_bucket_t* stats, of1x_stats_bucket_desc_msg_t* desc, std::list<bucket_snapshot>& buckets);
	
	//Dumping operator
	friend std::ostream& operator<<(std::ostream& os, const bucket_snapshot& b)
	{
		os << " weight:"<< b.get_weight()<<" group:"<<b.get_watch_group()<<" port:"<<b.get_watch_port()
		<<" packet count:"<<b.packet_count<<" byte count:"<<b.byte_count;

		//TODO: use (non-exisiting yet) single line dumpers
		os << " {, actions"; 
		os << b.get_actions();
		os << " }\n";
		
		return os;
	}
};
	
class openflow_group_mod_snapshot{
public:
	
	uint32_t group_id;
	of1x_group_type_t type;
	int num_of_buckets;
	
	//buckets
	std::list<bucket_snapshot> buckets;
	
	//stats
	uint64_t packet_count;
	uint64_t byte_count;
	
	openflow_group_mod_snapshot(of_version_t ver, of1x_stats_group_msg_t* stats, of1x_stats_group_desc_msg_t* desc);
	rofl_result_t map_group_mods_msg(of_version_t ver, of1x_stats_group_msg_t* stats, of1x_stats_group_desc_msg_t* desc, std::list<openflow_group_mod_snapshot>& group_mods);
	
	//Dumping operator
	friend std::ostream& operator<<(std::ostream& os, const openflow_group_mod_snapshot& g)
	{
		std::list<bucket_snapshot>::iterator b_it;
		
		os << " id:"<< g.group_id<<" type:"<<g.type<<" # buckets:"<<g.num_of_buckets
		<<" packet count:"<<g.packet_count<<" byte count:"<<g.byte_count;
		
		//TODO: use (non-exisiting yet) single line dumpers
		os << " {buckets";
		for(b_it=g.buckets.begin(); b_it!=g.buckets.end(); b_it++){
			os << *b_it;
		}
		os << " }\n";
		
		return os;
	}
};

}// namespace xdpd

#endif //GROUP_MOD_SNAPSHOT_H