/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SWITCH_SNAPSHOT_H
#define SWITCH_SNAPSHOT_H 

#include <iostream>
#include <string> 
#include <sstream>
#include <list> 
#include <rofl.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/matching_algorithms/matching_algorithms.h>

/**
* @file switch_snapshot.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief C++ switch snapshot
*/

namespace xdpd {


class openflow_switch_table_snapshot{

public:
	
	openflow_switch_table_snapshot(of1x_flow_table_t* table):
			number(table->number),
			num_of_entries(table->num_of_entries),
			max_entries(table->max_entries),
			default_action(table->default_action),
			matching_algorithm(table->matching_algorithm),
			config_match(table->config.match),
			config_wildcards(table->config.wildcards),
			config_write_actions(table->config.write_actions),
			config_apply_actions(table->config.apply_actions),
			config_metadata_match(table->config.metadata_match),
			config_metadata_write(table->config.metadata_write),
			config_instructions(table->config.instructions),
			config_table_miss_config(table->config.table_miss_config),
			stats_lookup(table->stats.lookup_count),
			stats_matched(table->stats.matched_count)		
	{};

	//Dumping operator
	friend std::ostream& operator<<(std::ostream& os, openflow_switch_table_snapshot const& t)
	{
		os << "\t[table:"<<t.number<<", # entries:"<<t.num_of_entries <<", MA:"<<t.matching_algorithm<<", miss-config:"<<t.get_default_action_str()<<", stats{lookup:"<<t.stats_lookup<<", matched:"<<t.stats_matched<<"}]\n";
		os <<std::hex<< "\t [{ matches:0x"<<t.config_match.__submap[0]<<"-"<<t.config_match.__submap[1];
		os<<", wildcards:0x"<<t.config_wildcards.__submap[0]<<"-"<<t.config_wildcards.__submap[1];
		os<<", inst:0x"<<t.config_instructions;
		os<<", MNE:0x"<<t.max_entries<<"}]\n"<<std::dec;
		return os;
	}

	std::string get_default_action_str() const{
		switch(default_action){
			case OF1X_TABLE_MISS_CONTINUE:
				return std::string("NEXT_TABLE");
			case OF1X_TABLE_MISS_DROP:
				return std::string("DROP");
			case OF1X_TABLE_MISS_CONTROLLER:
				return std::string("CTL");
			
			default:
				std::stringstream ss;
				ss << "UNKNOWN("<< config_table_miss_config <<")";
				return ss.str();
		}
	}

	/**
	* Table number
	*/
	unsigned int number;

	/**
	* Number of entries installed
	*/
	unsigned int num_of_entries;

	/**
	* Maximum number of entries allowed in this table
	*/
	unsigned int max_entries;
	
	//
	// Table configuration
	//

	/**
	* Default action on miss 
	*/
	of1x_flow_table_miss_config_t default_action;

	/**
	* Matching algorithm
	*/
	enum of1x_matching_algorithm_available matching_algorithm;

	/**
	* Bitmap of (1 << OF1X_MATCH_*) that indicate the fields the table can match on. 
	*/
	bitmap128_t config_match;

	/**
	* Bitmap of (1 << OF1X_MATCH_*) wildcards that are supported by the table. 
	*/	 	
	bitmap128_t config_wildcards;
	
	/**
	* Bitmap of (1 << OF1X_AT_* that are supported by the table with OFPIT_WRITE_ACTIONS. 
	*/
	bitmap128_t config_write_actions;
	
	/**
	* Bitmap of (1 << OF1X_AT_* that are supported by the table with OFPIT_APPLY_ACTIONS. 
	*/
	bitmap128_t config_apply_actions;

	/**
	* Bits of metadata table can match. 
	*/
	bitmap64_t config_metadata_match;

	/** 
	* Bits of metadata table can write. 
	*/
	bitmap64_t config_metadata_write;

 	/**
	* Bitmap of OF1X_IT_* values supported. 
	*/
	bitmap32_t config_instructions;

	/**
	* Bitmap of OF1X_TABLE_MISS_* values 
	*/
	bitmap32_t config_table_miss_config;

	//
	// Statistics
	//
	
	/**
	* Number of packets looked up in table. 
	*/
	uint64_t stats_lookup;

	/**
	* Number of packets that hit table. 
	*/
	uint64_t stats_matched;
		
};

/**
* @brief C++ switch snapshot 
* @ingroup cmm_mgmt
*/
class openflow_switch_snapshot {

public:

	openflow_switch_snapshot(void){};
	
	openflow_switch_snapshot(of_switch_snapshot_t* sw){

		of1x_switch_t* snapshot = (of1x_switch_t*)sw;	

		//Assign m
		version = snapshot->of_ver;
		dpid = snapshot->dpid;
		name = std::string(snapshot->name);
		num_of_tables = snapshot->pipeline.num_of_tables; 
		num_of_buffers = snapshot->pipeline.num_of_buffers; 
		miss_send_len = snapshot->pipeline.miss_send_len; 
		capabilities = snapshot->pipeline.capabilities; 
	
		//Construct 
		for(unsigned int i=0;i<num_of_tables;i++)
			tables.push_back(openflow_switch_table_snapshot(&snapshot->pipeline.tables[i]));
	}

	//Dumping operator
	friend std::ostream& operator<<(std::ostream& os, openflow_switch_snapshot const& sw)
	{
	    os << "OpenFlow switch: "<<sw.name<<"\n"; 
	    os << "\tversion:"<<sw.get_version_str()<<", # tables:"<<sw.num_of_tables<<", capabilities:0x"<<std::hex<<sw.capabilities<<", miss_send_len:"<<std::dec<<sw.miss_send_len<<"\n"; 
		//TODO: add controllers
	
	    return os;
	}

	std::string get_version_str() const{
		switch(version){
			case OF_VERSION_10:
				return std::string("1.0");
			case OF_VERSION_12:
				return std::string("1.2");
			case OF_VERSION_13:
				return std::string("1.3");
			default: 
				assert(0);
				return std::string("Unknown");
		}
	}

	/**
	* OpenFlow running version
	*/
	of_version_t version; 

	/**
	* Datapath ID
	*/
	uint64_t dpid;

	/**
	* Switch name 
	*/
	std::string name;

	//
	// Pipeline
	//
	
	/**
	* Number of flow tables
	*/
	unsigned int num_of_tables;

	/**
	* Number of buffers
	*/
	unsigned int num_of_buffers;

	/**
	* Miss-send length configured
	*/
	uint16_t miss_send_len;

	/**
	* OF1X_CAP_* bitmap
	*/
	bitmap32_t capabilities; 

	//
	// Group and flow tables
	//
	
	/**
	* Openflow tables
	*/ 
	std::list<openflow_switch_table_snapshot> tables; 
	
	//FIXME: add groups
	
};


}// namespace xdpd

#endif /* SWITCH_SNAPSHOT_H_ */
