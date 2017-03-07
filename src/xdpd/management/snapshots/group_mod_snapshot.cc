#include "group_mod_snapshot.h"

//Translators for matches and actions
#include "../../openflow/openflow12/of12_translation_utils.h"
#include "../../openflow/openflow13/of13_translation_utils.h"

using namespace xdpd;

bucket_snapshot::bucket_snapshot(of1x_stats_bucket_t* stats, rofl::openflow::cofbucket& bucket):
		rofl::openflow::cofbucket(bucket.get_version(), bucket.get_weight(), bucket.get_watch_port(), bucket.get_watch_group()),
		packet_count(stats->packet_count),
		byte_count(stats->byte_count)
{
		set_actions() = bucket.get_actions();
}

rofl_result_t
bucket_snapshot::map_bucket_list(of_version_t ver, int num_of_buckets, of1x_stats_bucket_t* stats, of1x_stats_bucket_desc_msg_t* desc, std::list<bucket_snapshot>& buckets){
	of1x_stats_bucket_t* stats_ptr=stats;
	std::map<uint32_t, rofl::openflow::cofbucket>::iterator bu_it;
	rofl::openflow::cofbuckets bclist(ver);
	
	switch(ver){
			case OF_VERSION_10:
				assert(0);
				break;
			case OF_VERSION_12:
				of12_translation_utils::of12_map_reverse_bucket_list(bclist, desc);
				break;
			case OF_VERSION_13:
				of13_translation_utils::of13_map_reverse_bucket_list(bclist, desc);
				break;
			default:
				assert(0);
				break;
	}
	
	try{ 
		//Translate group mods
		for (auto bucket_id : bclist.keys()) {
			buckets.push_back(bucket_snapshot(stats_ptr, bclist.set_bucket(bucket_id)));
			stats_ptr++;
		}
	}catch(...){
		assert(0);
		return ROFL_FAILURE;
	}

	return ROFL_SUCCESS;
}

openflow_group_mod_snapshot::openflow_group_mod_snapshot(of_version_t ver, of1x_stats_group_msg_t* stats, of1x_stats_group_desc_msg_t* desc):
		group_id(stats->group_id),
		type(desc->type),
		num_of_buckets(stats->num_of_buckets),
		packet_count(stats->packet_count),
		byte_count(stats->byte_count)
{
	//create bucket snapshots
	if(bucket_snapshot::map_bucket_list(ver, stats->num_of_buckets, stats->bucket_stats, desc->bucket, buckets)!=ROFL_SUCCESS){
		assert(0);
		//TODO throw eOfSmGeneralError;
	}
}

rofl_result_t
openflow_group_mod_snapshot::map_group_mods_msg(of_version_t ver, of1x_stats_group_msg_t* stats, of1x_stats_group_desc_msg_t* desc, std::list<openflow_group_mod_snapshot>& group_mods){
	of1x_stats_group_msg_t* stats_it;
	of1x_stats_group_desc_msg_t* desc_it;
	
	try{ 
		//Translate group mods
		for(stats_it = stats, desc_it = desc; (stats_it && desc_it); stats_it=stats_it->next, desc_it=desc_it->next){
			group_mods.push_back(openflow_group_mod_snapshot(ver, stats_it, desc_it));
	}
	}catch(...){
		assert(0);
		return ROFL_FAILURE;
	}

	return ROFL_SUCCESS;
}
