#include "of13_endpoint.h"

#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include "of13_translation_utils.h"

using namespace xdpd;

/*
* Constructor and destructor
*/
of13_endpoint::of13_endpoint(
		openflow_switch* sw,
		int reconnect_start_timeout,
		caddress const& controller_addr,
		caddress const& binding_addr)  throw (eOfSmErrorOnCreation) {

	//Reference back to the sw
	this->sw = sw;

	//Set bitmaps
	crofbase::get_versionbitmap().add_ofp_version(rofl::openflow13::OFP_VERSION);
	cofhello_elem_versionbitmap versionbitmap;
	versionbitmap.add_ofp_version(openflow13::OFP_VERSION);
	
	//Connect to controller
	crofbase::rpc_connect_to_ctl(versionbitmap, reconnect_start_timeout, controller_addr);
}

/*
*
* Handling endpoint messages routines
*
*/

void
of13_endpoint::handle_features_request(
		crofctl& ctl,
		cofmsg_features_request& msg,
		uint8_t aux_id)
{
	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)fwd_module_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw eRofBase();
	
	uint32_t num_of_tables 	= of13switch->pipeline.num_of_tables;
	uint32_t num_of_buffers = of13switch->pipeline.num_of_buffers;
	uint32_t capabilities 	= of13switch->pipeline.capabilities;
	
	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_features_reply(
			msg.get_xid(),
			sw->dpid,
			num_of_buffers,	// n_buffers
			num_of_tables,	// n_tables
			capabilities,	// capabilities
			aux_id);		// auxiliary connection id
}



void
of13_endpoint::handle_get_config_request(
		crofctl& ctl,
		cofmsg_get_config_request& msg,
		uint8_t aux_id)
{
	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)fwd_module_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw eRofBase();
	
	uint16_t flags 			= of13switch->pipeline.capabilities;
	uint16_t miss_send_len 	= of13switch->pipeline.miss_send_len;

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_get_config_reply(msg.get_xid(), flags, miss_send_len);
}



void
of13_endpoint::handle_set_config(
		crofctl& ctl,
		cofmsg_set_config& msg,
		uint8_t aux_id)
{
	//Instruct the driver to process the set config
	if(AFA_FAILURE == fwd_module_of1x_set_pipeline_config(sw->dpid, msg.get_flags(), msg.get_miss_send_len())){
		throw eTableModBadConfig();
	}
}



void
of13_endpoint::handle_packet_out(
		crofctl& ctl,
		cofmsg_packet_out& msg,
		uint8_t aux_id)
{
	of1x_action_group_t* action_group = of1x_init_action_group(NULL);

	try{
		of13_translation_utils::of13_map_flow_entry_actions(&ctl, sw, msg.set_actions(), action_group, NULL); //TODO: is this OK always NULL?
	}catch(...){
		of1x_destroy_action_group(action_group);
		throw;
	}

	/* assumption: driver can handle all situations properly:
	 * - data and datalen both 0 and buffer_id != OFP_NO_BUFFER
	 * - buffer_id == OFP_NO_BUFFER and data and datalen both != 0
	 * - everything else is an error?
	 */
	if (AFA_FAILURE == fwd_module_of1x_process_packet_out(sw->dpid,
							msg.get_buffer_id(),
							msg.get_in_port(),
							action_group,
							msg.get_packet().soframe(), msg.get_packet().framelen())){
		// log error
		//FIXME: send error
	}

	of1x_destroy_action_group(action_group);
}



void
of13_endpoint::handle_flow_mod(
		crofctl& ctl,
		cofmsg_flow_mod& msg,
		uint8_t aux_id)
{
	switch (msg.get_command()) {
	case openflow13::OFPFC_ADD: {
		flow_mod_add(ctl, msg);
	} break;
	case openflow13::OFPFC_MODIFY: {
		flow_mod_modify(ctl, msg, false);
	} break;
	case openflow13::OFPFC_MODIFY_STRICT: {
		flow_mod_modify(ctl, msg, true);
	} break;
	case openflow13::OFPFC_DELETE: {
		flow_mod_delete(ctl, msg, false);
	} break;
	case openflow13::OFPFC_DELETE_STRICT: {
		flow_mod_delete(ctl, msg, true);
	} break;
	default:
		throw eFlowModBadCommand();
	}
}



void
of13_endpoint::flow_mod_add(
		crofctl& ctl,
		cofmsg_flow_mod& msg)
{
	uint8_t table_id = msg.get_table_id();
	afa_result_t res;
	of1x_flow_entry_t *entry=NULL;

	// sanity check: table for table-id must exist
	if ( (table_id > sw->num_of_tables) && (table_id != openflow13::OFPTT_ALL) ){
		rofl::logging::error << "[xdpd][of13][flow-mod-add] unable to add flow-mod due to " <<
				"invalid table-id:" << msg.get_table_id() << " on dpt:" << sw->dpname << std::endl;
		throw eFlowModBadTableId();
	}

	try{
		entry = of13_translation_utils::of13_map_flow_entry(&ctl, &msg, sw);
	}catch(...){
		rofl::logging::error << "[xdpd][of13][flow-mod-add] unable to map flow-mod entry to internal representation on dpt:" << sw->dpname << std::endl;
		throw eFlowModUnknown();
	}

	if(!entry){
		throw eFlowModUnknown();//Just for safety, but shall never reach this
	}

	if (AFA_SUCCESS != (res = fwd_module_of1x_process_flow_mod_add(sw->dpid,
								msg.get_table_id(),
								&entry,
								msg.get_buffer_id(),
								msg.get_flags() & openflow13::OFPFF_CHECK_OVERLAP,
								msg.get_flags() & openflow13::OFPFF_RESET_COUNTS))){
		// log error
		rofl::logging::error << "[xdpd][of13][flow-mod-add] error inserting flow-mod on dpt:" << sw->dpname << std::endl;
		of1x_destroy_flow_entry(entry);

		if(res == AFA_FM_OVERLAP_FAILURE){
			throw eFlowModOverlap();
		}else{
			throw eFlowModTableFull();
		}
	}
}



void
of13_endpoint::flow_mod_modify(
		crofctl& ctl,
		cofmsg_flow_mod& pack,
		bool strict)
{
	of1x_flow_entry_t *entry=NULL;

	// sanity check: table for table-id must exist
	if (pack.get_table_id() > sw->num_of_tables)
	{
		rofl::logging::error << "[xdpd][of13][flow-mod-modify] unable to modify flow-mod due to " <<
				"invalid table-id:" << pack.get_table_id() << " on dpt:" << sw->dpname << std::endl;
		throw eFlowModBadTableId();
	}

	try{
		entry = of13_translation_utils::of13_map_flow_entry(&ctl, &pack, sw);
	}catch(...){
		rofl::logging::error << "[xdpd][of13][flow-mod-modify] unable to map flow-mod entry to internal representation on dpt:" << sw->dpname << std::endl;
		throw eFlowModUnknown();
	}

	if(!entry){
		throw eFlowModUnknown();//Just for safety, but shall never reach this
	}


	of1x_flow_removal_strictness_t strictness = (strict) ? STRICT : NOT_STRICT;


	if(AFA_SUCCESS != fwd_module_of1x_process_flow_mod_modify(sw->dpid,
								pack.get_table_id(),
								&entry,
								pack.get_buffer_id(),
								strictness,
								pack.get_flags() & openflow13::OFPFF_RESET_COUNTS)){
		rofl::logging::error << "[xdpd][of13][flow-mod-modify] error modifying flow-mod on dpt:" << sw->dpname << std::endl;
		of1x_destroy_flow_entry(entry);

		throw eFlowModBase();
	}
}



void
of13_endpoint::flow_mod_delete(
		crofctl& ctl,
		cofmsg_flow_mod& pack,
		bool strict)
{

	of1x_flow_entry_t *entry=NULL;

	try{
		entry = of13_translation_utils::of13_map_flow_entry(&ctl, &pack, sw);
	}catch(...){
		rofl::logging::error << "[xdpd][of13][flow-mod-delete] unable to map flow-mod entry to internal representation on dpt:" << sw->dpname << std::endl;
		throw eFlowModUnknown();
	}

	if(!entry)
		throw eFlowModUnknown();//Just for safety, but shall never reach this


	of1x_flow_removal_strictness_t strictness = (strict) ? STRICT : NOT_STRICT;

	if(AFA_SUCCESS != fwd_module_of1x_process_flow_mod_delete(sw->dpid,
								pack.get_table_id(),
								entry,
								pack.get_out_port(),
								pack.get_out_group(),
								strictness)) {
		rofl::logging::error << "[xdpd][of13][flow-mod-delete] error deleting flow-mod on dpt:" << sw->dpname << std::endl;
		of1x_destroy_flow_entry(entry);
		throw eFlowModBase();
	}

	//Always delete entry
	of1x_destroy_flow_entry(entry);
}



void
of13_endpoint::handle_desc_stats_request(
		crofctl& ctl,
		cofmsg_desc_stats_request& msg,
		uint8_t aux_id)
{
	std::string mfr_desc("eXtensible Data Path");
	std::string hw_desc(XDPD_VERSION);
	std::string sw_desc(XDPD_VERSION);
	std::string serial_num("0");
	std::string dp_desc("xDP");

	cofdesc_stats_reply desc_stats(
			ctl.get_version(),
			mfr_desc,
			hw_desc,
			sw_desc,
			serial_num,
			dp_desc);

	ctl.send_desc_stats_reply(msg.get_xid(), desc_stats);
}



void
of13_endpoint::handle_flow_stats_request(
		crofctl& ctl,
		cofmsg_flow_stats_request& msg,
		uint8_t aux_id)
{
	//Map the match structure from OpenFlow to packet_matches_t
	of1x_flow_entry_t* entry = of1x_init_flow_entry(NULL, NULL, false);

	try{
		of13_translation_utils::of13_map_flow_entry_matches(&ctl, msg.get_flow_stats().get_match(), sw, entry);
	}catch(...){
		of1x_destroy_flow_entry(entry);
		throw eBadRequestBadStat();
	}

	//Ask the Forwarding Plane to process stats
	of1x_stats_flow_msg_t* fp_msg = fwd_module_of1x_get_flow_stats(
													sw->dpid,
													msg.get_flow_stats().get_table_id(),
													msg.get_flow_stats().get_cookie(),
													msg.get_flow_stats().get_cookie_mask(),
													msg.get_flow_stats().get_out_port(),
													msg.get_flow_stats().get_out_group(),
													&entry->matches);

	if(!fp_msg){
		of1x_destroy_flow_entry(entry);
		throw eBadRequestBadStat();
	}

	//Construct OF message
	of1x_stats_single_flow_msg_t *elem = fp_msg->flows_head;

	std::vector<cofflow_stats_reply> flow_stats;

	for(elem = fp_msg->flows_head; elem; elem = elem->next){

		cofmatch match;
		of13_translation_utils::of13_map_reverse_flow_entry_matches(elem->matches, match);

		cofinstructions instructions(ctl.get_version());
		of13_translation_utils::of13_map_reverse_flow_entry_instructions((of1x_instruction_group_t*)(elem->inst_grp), instructions);


		flow_stats.push_back(
				cofflow_stats_reply(
						ctl.get_version(),
						elem->table_id,
						elem->duration_sec,
						elem->duration_nsec,
						elem->priority,
						elem->idle_timeout,
						elem->hard_timeout,
						elem->cookie,
						elem->packet_count,
						elem->byte_count,
						match,
						instructions));
	}


	try{
		//Send message
		ctl.send_flow_stats_reply(msg.get_xid(), flow_stats);
	}catch(...){
		of1x_destroy_stats_flow_msg(fp_msg);
		of1x_destroy_flow_entry(entry);
		throw;
	}
	//Destroy FP stats
	of1x_destroy_stats_flow_msg(fp_msg);
	of1x_destroy_flow_entry(entry);
}



void
of13_endpoint::handle_aggregate_stats_request(
		crofctl& ctl,
		cofmsg_aggr_stats_request& msg,
		uint8_t aux_id)
{
	//Map the match structure from OpenFlow to packet_matches_t
	 of1x_flow_entry_t* entry = of1x_init_flow_entry(NULL, NULL, false);

	if(!entry)
		throw eBadRequestBadStat();

	try{
		of13_translation_utils::of13_map_flow_entry_matches(&ctl, msg.get_aggr_stats().get_match(), sw, entry);
	}catch(...){
		of1x_destroy_flow_entry(entry);
		throw eBadRequestBadStat();
	}

	//TODO check error while mapping

	//Ask the Forwarding Plane to process stats
	of1x_stats_flow_aggregate_msg_t* fp_msg = fwd_module_of1x_get_flow_aggregate_stats(sw->dpid,
					msg.get_aggr_stats().get_table_id(),
					msg.get_aggr_stats().get_cookie(),
					msg.get_aggr_stats().get_cookie_mask(),
					msg.get_aggr_stats().get_out_port(),
					msg.get_aggr_stats().get_out_group(),
					&entry->matches);

	if(!fp_msg){
		of1x_destroy_flow_entry(entry);
		throw eBadRequestBadStat();
	}

	try{
		cofaggr_stats_reply aggr_stats_reply(
				ctl.get_version(),
				fp_msg->packet_count,
				fp_msg->byte_count,
				fp_msg->flow_count);
		//Construct OF message
		ctl.send_aggr_stats_reply(msg.get_xid(), aggr_stats_reply);
	}catch(...){
		of1x_destroy_stats_flow_aggregate_msg(fp_msg);
		of1x_destroy_flow_entry(entry);
		throw;
	}

	//Destroy FP stats
	of1x_destroy_stats_flow_aggregate_msg(fp_msg);
	of1x_destroy_flow_entry(entry);
}



void
of13_endpoint::handle_table_stats_request(
		crofctl& ctl,
		cofmsg_table_stats_request& msg,
		uint8_t aux_id)
{
	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)fwd_module_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw eRofBase();
	
	unsigned int num_of_tables = of13switch->pipeline.num_of_tables;
	std::vector<coftable_stats_reply> table_stats;

	for (unsigned int n = 0; n < num_of_tables; n++) {
	
		of1x_flow_table_t *table = &of13switch->pipeline.tables[n];

		table_stats.push_back(
				coftable_stats_reply(
					ctl.get_version(),
					table->number,
					(table->num_of_entries),
					(table->stats.lookup_count),
					(table->stats.matched_count)
				));
	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_table_stats_reply(msg.get_xid(), table_stats, false);
}



void
of13_endpoint::handle_port_stats_request(
		crofctl& ctl,
		cofmsg_port_stats_request& msg,
		uint8_t aux_id)
{

	switch_port_snapshot_t* port;
	uint32_t port_no = msg.get_port_stats().get_portno();

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)fwd_module_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw eRofBase();

	std::vector<cofport_stats_reply> port_stats;

	/*
	 *  send statistics for all ports
	 */
	if (openflow13::OFPP_ALL == port_no){

		//we check all the positions in case there are empty slots
		for (unsigned int n = 1; n < of13switch->max_ports; n++){
	
			port = of13switch->logical_ports[n].port; 
	
			if((port != NULL) && (of13switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED)){

				port_stats.push_back(
						cofport_stats_reply(
								ctl.get_version(),
								port->of_port_num,
								port->stats.rx_packets,
								port->stats.tx_packets,
								port->stats.rx_bytes,
								port->stats.tx_bytes,
								port->stats.rx_dropped,
								port->stats.tx_dropped,
								port->stats.rx_errors,
								port->stats.tx_errors,
								port->stats.rx_frame_err,
								port->stats.rx_over_err,
								port->stats.rx_crc_err,
								port->stats.collisions,
								/*duration_sec=*/0,
								/*duration_nsec=*/0));
			}
	 	}

	}else{
		/*
		 * send statistics for only one port
		 */
		
		// search for the port with the specified port-number
		//we check all the positions in case there are empty slots
		for (unsigned int n = 1; n < of13switch->max_ports; n++){
			
			port = of13switch->logical_ports[n].port; 

			if( 	(port != NULL) &&
				(of13switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED) &&
				(port->of_port_num == port_no)
			){
				//Mapping of port state
				port_stats.push_back(
						cofport_stats_reply(
								ctl.get_version(),
								port->of_port_num,
								port->stats.rx_packets,
								port->stats.tx_packets,
								port->stats.rx_bytes,
								port->stats.tx_bytes,
								port->stats.rx_dropped,
								port->stats.tx_dropped,
								port->stats.rx_errors,
								port->stats.tx_errors,
								port->stats.rx_frame_err,
								port->stats.rx_over_err,
								port->stats.rx_crc_err,
								port->stats.collisions,
								/*duration_sec=*/0,
								/*duration_nsec=*/0));

				break;
			}
	 	}

		// if port_no was not found, body.memlen() is 0
	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_port_stats_reply(msg.get_xid(), port_stats, false);
}




void
of13_endpoint::handle_queue_stats_request(
		crofctl& ctl,
		cofmsg_queue_stats_request& pack,
		uint8_t aux_id)
{

	switch_port_snapshot_t* port = NULL;
	unsigned int portnum = pack.get_queue_stats().get_port_no();
	unsigned int queue_id = pack.get_queue_stats().get_queue_id();

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)fwd_module_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw eRofBase();


	if( ((portnum >= of13switch->max_ports) && (portnum != openflow13::OFPP_ALL)) || portnum == 0){
		//Destroy the snapshot
		of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);
		throw eBadRequestBadPort(); 	//Invalid port num
	}

	std::vector<cofqueue_stats_reply> stats;

	/*
	* port num
	*/

	//we check all the positions in case there are empty slots
	for (unsigned int n = 1; n < of13switch->max_ports; n++){

		port = of13switch->logical_ports[n].port;

		if ((openflow13::OFPP_ALL != portnum) && (port->of_port_num != portnum))
			continue;


		if((port != NULL) && (of13switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED)/* && (port->of_port_num == portnum)*/){

			if (OFPQ_ALL == queue_id){

				// TODO: iterate over all queues

				for(unsigned int i=0; i<port->max_queues; i++){
					if(!port->queues[i].set)
						continue;

					//Set values
					stats.push_back(
							cofqueue_stats_reply(
									ctl.get_version(),
									port->of_port_num,
									i,
									port->queues[i].stats.tx_bytes,
									port->queues[i].stats.tx_packets,
									port->queues[i].stats.overrun,
									/*duration_sec=*/0,
									/*duration_nsec=*/0));
				}

			} else {

				if(queue_id >= port->max_queues){
					//Destroy the snapshot
					of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);
					throw eBadRequestBadPort(); 	//FIXME send a BadQueueId error
				}

				//Check if the queue is really in use
				if(port->queues[queue_id].set){
					//Set values
					stats.push_back(
							cofqueue_stats_reply(
									ctl.get_version(),
									portnum,
									queue_id,
									port->queues[queue_id].stats.tx_bytes,
									port->queues[queue_id].stats.tx_packets,
									port->queues[queue_id].stats.overrun,
									/*duration_sec=*/0,
									/*duration_nsec=*/0));

				}
			}
		}
	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);
	
	ctl.send_queue_stats_reply(pack.get_xid(), stats);
}



void
of13_endpoint::handle_group_stats_request(
		crofctl& ctl,
		cofmsg_group_stats_request& msg,
		uint8_t aux_id)
{
	// we need to get the statistics, build a packet and send it
	unsigned int i;
	cmemory body(0);
	unsigned int num_of_buckets;
	of1x_stats_group_msg_t *g_msg, *g_msg_all;

	uint32_t group_id = msg.get_group_stats().get_group_id();
	
	if(group_id==openflow13::OFPG_ALL){
		g_msg_all = fwd_module_of1x_get_group_all_stats(sw->dpid, group_id);
	}
	else{
		g_msg_all = fwd_module_of1x_get_group_stats(sw->dpid, group_id);
	}
	
	if(g_msg_all==NULL){
		//TODO handle error
		logging::error << "[xdpd][of13][group-stats] unable to retrieve group statistics from pipeline" << std::endl;
	}
	
	std::vector<cofgroup_stats_reply> group_stats;
	
	for(g_msg = g_msg_all; g_msg; g_msg = g_msg->next){
		num_of_buckets = g_msg->num_of_buckets;

		cofgroup_stats_reply stats(
				ctl.get_version(),
				/*msg->get_group_stats().get_group_id(),*/
				htobe32(g_msg->group_id),
				htobe32(g_msg->ref_count),
				htobe64(g_msg->packet_count),
				htobe64(g_msg->byte_count),
				/*duration_sec=*/0,
				/*duration_nsec=*/0,
				num_of_buckets);

		for(i=0;i<num_of_buckets;i++) {
			stats.get_bucket_counter(i).packet_count = g_msg->bucket_stats[i].packet_count;
			stats.get_bucket_counter(i).byte_count = g_msg->bucket_stats[i].byte_count;
		}
		
		group_stats.push_back(stats);
	}

	try{
		//Send the group stats
		ctl.send_group_stats_reply(msg.get_xid(), group_stats);
	}catch(...){
		of1x_destroy_stats_group_msg(g_msg_all);
		throw;
	}
	
	//Destroy the g_msg
	of1x_destroy_stats_group_msg(g_msg_all);
}



void
of13_endpoint::handle_group_desc_stats_request(
		crofctl& ctl,
		cofmsg_group_desc_stats_request& msg,
		uint8_t aux_id)
{
	rofl::openflow::cofgroupdescs groupdescs(ctl.get_version());

	of1x_group_table_t group_table;
	of1x_group_t *group_it;
	if(fwd_module_of1x_fetch_group_table(sw->dpid,&group_table)!=AFA_SUCCESS){

		//TODO throw exeption
	}
	
	for(group_it=group_table.head;group_it;group_it=group_it->next){
		cofbuckets bclist(ctl.get_version());
		of13_translation_utils::of13_map_reverse_bucket_list(bclist,group_it->bc_list);
		
		groupdescs.set_group_desc(group_it->id).set_group_type(group_it->type);
		groupdescs.set_group_desc(group_it->id).set_group_id(group_it->id);
		groupdescs.set_group_desc(group_it->id).set_buckets(bclist);
	}

	ctl.send_group_desc_stats_reply(msg.get_xid(), groupdescs);
}



void
of13_endpoint::handle_group_features_stats_request(
		crofctl& ctl,
		cofmsg_group_features_stats_request& msg,
		uint8_t aux_id)
{
	cofgroup_features_stats_reply group_features_reply(ctl.get_version());

	group_features_reply.set_types((uint32_t)0);
	group_features_reply.set_capabilities((uint32_t)0);
	group_features_reply.set_max_groups()[0] = (uint32_t)0;
	group_features_reply.set_max_groups()[1] = (uint32_t)0;
	group_features_reply.set_max_groups()[2] = (uint32_t)0;
	group_features_reply.set_max_groups()[3] = (uint32_t)0;
	group_features_reply.set_actions()[0] = (uint32_t)0;
	group_features_reply.set_actions()[1] = (uint32_t)0;
	group_features_reply.set_actions()[2] = (uint32_t)0;
	group_features_reply.set_actions()[3] = (uint32_t)0;

	//TODO: fill in group_features_reply, when groups are implemented

	ctl.send_group_features_stats_reply(msg.get_xid(), group_features_reply);
}



void
of13_endpoint::handle_table_features_stats_request(
		crofctl& ctl,
		cofmsg_table_features_stats_request& msg,
		uint8_t aux_id)
{
	// TODO: check for pipeline definition within request and configure pipeline accordingly

	rofl::openflow::coftables tables(ctl.get_version());

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)fwd_module_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw eRofBase();

	unsigned int num_of_tables = of13switch->pipeline.num_of_tables;
	for (unsigned int n = 0; n < num_of_tables; n++) {

		of1x_flow_table_t* table = &of13switch->pipeline.tables[n];
		of1x_flow_table_config_t* tc = &table->config;
		uint8_t table_id = table->number;

		tables.set_table(table_id).set_table_id(table_id);
		tables.set_table(table_id).set_config(0);
		tables.set_table(table_id).set_max_entries(table->max_entries);
		tables.set_table(table_id).set_name(std::string(table->name, strnlen(table->name, OFP_MAX_TABLE_NAME_LEN)));
		tables.set_table(table_id).set_metadata_match(tc->metadata_match);
		tables.set_table(table_id).set_metadata_write(tc->metadata_write);

		of13_translation_utils::of13_map_bitmap_instructions(&tc->instructions, tables.set_table(table_id).set_properties().set_tfp_instructions());
		of13_translation_utils::of13_map_bitmap_instructions(&tc->instructions, tables.set_table(table_id).set_properties().set_tfp_instructions_miss());
		of13_translation_utils::of13_map_bitmap_matches(&tc->match, tables.set_table(table_id).set_properties().set_tfp_match());

		if(tc->instructions & ( 1 << OF1X_IT_APPLY_ACTIONS)) {
			of13_translation_utils::of13_map_bitmap_actions(&tc->apply_actions, tables.set_table(table_id).set_properties().set_tfp_apply_actions());
			of13_translation_utils::of13_map_bitmap_actions(&tc->apply_actions, tables.set_table(table_id).set_properties().set_tfp_apply_actions_miss());
			of13_translation_utils::of13_map_bitmap_set_fields(&tc->apply_setfields, tables.set_table(table_id).set_properties().set_tfp_apply_setfield());
			of13_translation_utils::of13_map_bitmap_set_fields(&tc->apply_setfields, tables.set_table(table_id).set_properties().set_tfp_apply_setfield_miss());
		}
		if(tc->instructions & ( 1 << OF1X_IT_WRITE_ACTIONS)) {
			of13_translation_utils::of13_map_bitmap_actions(&tc->write_actions, tables.set_table(table_id).set_properties().set_tfp_write_actions());
			of13_translation_utils::of13_map_bitmap_actions(&tc->write_actions, tables.set_table(table_id).set_properties().set_tfp_write_actions_miss());
			of13_translation_utils::of13_map_bitmap_set_fields(&tc->write_setfields, tables.set_table(table_id).set_properties().set_tfp_write_setfield());
			of13_translation_utils::of13_map_bitmap_set_fields(&tc->write_setfields, tables.set_table(table_id).set_properties().set_tfp_write_setfield_miss());
		}
		if(tc->instructions & ( 1 << OF1X_IT_GOTO_TABLE)) {
			for (unsigned int i = n + 1; i < num_of_tables; i++) {
				tables.set_table(table_id).set_properties().set_tfp_next_tables().add_table_id(i);
				tables.set_table(table_id).set_properties().set_tfp_next_tables_miss().add_table_id(i);
			}
		}
	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_table_features_stats_reply(msg.get_xid(), tables, 0);
}



void
of13_endpoint::handle_port_desc_stats_request(
		crofctl& ctl,
		cofmsg_port_desc_stats_request& msg,
		uint8_t aux_id)
{
	logical_switch_port_t* ls_port;
	switch_port_snapshot_t* _port;

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)fwd_module_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw eRofBase();

	// array of structures ofp_port
	rofl::cofports ports(ctl.get_version());

	//we check all the positions in case there are empty slots
	for (unsigned int n = 1; n < of13switch->max_ports; n++){

		ls_port = &of13switch->logical_ports[n];
		_port = ls_port->port;

		if(_port!=NULL && ls_port->attachment_state!=LOGICAL_PORT_STATE_DETACHED){

			//Mapping of port state
			assert(n == _port->of_port_num);

			cofport port(ctl.get_version());

			port.set_port_no(_port->of_port_num);
			port.set_hwaddr(cmacaddr(_port->hwaddr, OFP_ETH_ALEN));
			port.set_name(std::string(_port->name));

			uint32_t config = 0;
			if(!_port->up)
				config |= openflow13::OFPPC_PORT_DOWN;
			if(_port->drop_received)
				config |= openflow13::OFPPC_NO_RECV;
			if(!_port->forward_packets)
				config |= openflow13::OFPPC_NO_FWD;
			if(!_port->of_generate_packet_in)
				config |= openflow13::OFPPC_NO_PACKET_IN;

			port.set_config(config);
			port.set_state(_port->state);
			port.set_curr(_port->curr);
			port.set_advertised(_port->advertised);
			port.set_supported(_port->supported);
			port.set_peer(_port->peer);
			port.set_curr_speed(of13_translation_utils::get_port_speed_kb(_port->curr_speed));
			port.set_max_speed(of13_translation_utils::get_port_speed_kb(_port->curr_max_speed));

			ports.add_port(_port->of_port_num) = port;
		}
 	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_port_desc_stats_reply(msg.get_xid(), ports, 0);
}



void
of13_endpoint::handle_experimenter_stats_request(
		crofctl& ctl,
		cofmsg_experimenter_stats_request& pack,
		uint8_t aux_id)
{
	//TODO: when exp are supported 
}







rofl_result_t
of13_endpoint::process_packet_in(
		uint8_t table_id,
		uint8_t reason,
		uint32_t in_port,
		uint32_t buffer_id,
		uint8_t* pkt_buffer,
		uint32_t buf_len,
		uint16_t total_len,
		packet_matches_t* matches)
{
	try {
		//Transform matches 
		cofmatch match;
		of13_translation_utils::of13_map_reverse_packet_matches(matches, match);

		send_packet_in_message(
				buffer_id,
				total_len,
				reason,
				table_id,
				/*cookie=*/0,
				/*in_port=*/0, // OF1.0 only
				match,
				pkt_buffer, buf_len);

		return ROFL_SUCCESS;

	} catch (...) {

#if 0
		if (buffer_id == OF1XP_NO_BUFFER) {
			rofl::logging::error << "[xdpd][of13][packet-in] unable to send Packet-In message" << std::endl;

			return AFA_FAILURE;
		}

		rofl::logging::error << "[xdpd][of13][packet-in] unable to send Packet-In message, dropping packet from occupied pkt slot" << std::endl;

		of1x_action_group_t* action_group = of1x_init_action_group(NULL);

		try{
			rofl::cofactions actions(rofl::openflow13::OFP_VERSION);
			of13_translation_utils::of13_map_flow_entry_actions(NULL, sw, actions, action_group, NULL);
		}catch(...){
			of1x_destroy_action_group(action_group);
			return AFA_FAILURE;
		}

		/* assumption: driver can handle all situations properly:
		 * - data and datalen both 0 and buffer_id != OFP_NO_BUFFER
		 * - buffer_id == OFP_NO_BUFFER and data and datalen both != 0
		 * - everything else is an error?
		 */
		if (AFA_FAILURE == fwd_module_of1x_process_packet_out(sw->dpid,
								buffer_id,
								in_port,
								action_group,
								NULL, 0)){
			// log error
			rofl::logging::crit << "[xdpd][of13][packet-in] unable drop stored packet: this may lead to a deadlock situation!" << std::endl;
		}

		of1x_destroy_action_group(action_group);
#endif

		return ROFL_FAILURE;
	}

	return ROFL_FAILURE;
}

/*
* Port async notifications processing 
*/

rofl_result_t of13_endpoint::notify_port_attached(const switch_port_snapshot_t* port){

	uint32_t config=0x0;

	//Compose port config
	if(!port->up) config |= openflow13::OFPPC_PORT_DOWN;
	if(!port->of_generate_packet_in) config |= openflow13::OFPPC_NO_PACKET_IN;
	if(!port->forward_packets) config |= openflow13::OFPPC_NO_FWD;
	if(port->drop_received) config |= openflow13::OFPPC_NO_RECV;
			
	
	cofport ofport(openflow13::OFP_VERSION);
	ofport.set_port_no(port->of_port_num);
	ofport.set_hwaddr(cmacaddr((uint8_t*)port->hwaddr, OFP_ETH_ALEN));
	ofport.set_name(std::string(port->name));
	ofport.set_config(config);
	ofport.set_state(port->state);
	ofport.set_curr(port->curr);
	ofport.set_advertised(port->advertised);
	ofport.set_supported(port->supported);
	ofport.set_peer(port->peer);
	ofport.set_curr_speed(of13_translation_utils::get_port_speed_kb(port->curr_speed));
	ofport.set_max_speed(of13_translation_utils::get_port_speed_kb(port->curr_max_speed));
	
	//Send message
	send_port_status_message(openflow13::OFPPR_ADD, ofport);

	return ROFL_SUCCESS;
}

rofl_result_t of13_endpoint::notify_port_detached(const switch_port_snapshot_t* port){

	uint32_t config=0x0;

	//Compose port config
	if(!port->up) config |= openflow13::OFPPC_PORT_DOWN;
	if(!port->of_generate_packet_in) config |= openflow13::OFPPC_NO_PACKET_IN;
	if(!port->forward_packets) config |= openflow13::OFPPC_NO_FWD;
	if(port->drop_received) config |= openflow13::OFPPC_NO_RECV;
	
	cofport ofport(openflow13::OFP_VERSION);
	ofport.set_port_no(port->of_port_num);
	ofport.set_hwaddr(cmacaddr((uint8_t*)port->hwaddr, OFP_ETH_ALEN));
	ofport.set_name(std::string(port->name));
	ofport.set_config(config);
	ofport.set_state(port->state);
	ofport.set_curr(port->curr);
	ofport.set_advertised(port->advertised);
	ofport.set_supported(port->supported);
	ofport.set_peer(port->peer);
	ofport.set_curr_speed(of13_translation_utils::get_port_speed_kb(port->curr_speed));
	ofport.set_max_speed(of13_translation_utils::get_port_speed_kb(port->curr_max_speed));
	
	//Send message
	send_port_status_message(openflow13::OFPPR_DELETE, ofport);

	return ROFL_SUCCESS;
}

rofl_result_t of13_endpoint::notify_port_status_changed(const switch_port_snapshot_t* port){

	uint32_t config=0x0;

	//Compose port config
	if(!port->up) config |= openflow13::OFPPC_PORT_DOWN;
	if(!port->of_generate_packet_in) config |= openflow13::OFPPC_NO_PACKET_IN;
	if(!port->forward_packets) config |= openflow13::OFPPC_NO_FWD;
	if(port->drop_received) config |= openflow13::OFPPC_NO_RECV;
	
	//Notify OF controller
	cofport ofport(openflow13::OFP_VERSION);
	ofport.set_port_no(port->of_port_num);
	ofport.set_hwaddr(cmacaddr((uint8_t*)port->hwaddr, OFP_ETH_ALEN));
	ofport.set_name(std::string(port->name));
	ofport.set_config(config);
	ofport.set_state(port->state);
	ofport.set_curr(port->curr);
	ofport.set_advertised(port->advertised);
	ofport.set_supported(port->supported);
	ofport.set_peer(port->peer);
	ofport.set_curr_speed(of13_translation_utils::get_port_speed_kb(port->curr_speed));
	ofport.set_max_speed(of13_translation_utils::get_port_speed_kb(port->curr_max_speed));
	
	//Send message
	send_port_status_message(openflow13::OFPPR_MODIFY, ofport);

	return ROFL_SUCCESS; // ignore this notification
}





void
of13_endpoint::handle_barrier_request(
		crofctl& ctl,
		cofmsg_barrier_request& pack,
		uint8_t aux_id)
{
	//Since we are not queuing messages currently
	ctl.send_barrier_reply(pack.get_xid());
}









rofl_result_t
of13_endpoint::process_flow_removed(
		uint8_t reason,
		of1x_flow_entry *entry)
{
	cofmatch match;
	uint32_t sec,nsec;

	of13_translation_utils::of13_map_reverse_flow_entry_matches(entry->matches.head, match);

	//get duration of the flow mod
	of1x_stats_flow_get_duration(entry, &sec, &nsec);

	send_flow_removed_message(
			match,
			entry->cookie,
			entry->priority,
			reason,
			entry->table->number,
			sec,
			nsec,
			entry->timer_info.idle_timeout,
			entry->timer_info.hard_timeout,
			entry->stats.packet_count,
			entry->stats.byte_count);

	return ROFL_SUCCESS;
}



void
of13_endpoint::handle_group_mod(
		crofctl& ctl,
		cofmsg_group_mod& msg,
		uint8_t aux_id)
{
	//throw eNotImplemented(std::string("of13_endpoint::handle_group_mod()"));
	//steps:
	/* 1- map the packet
	 * 2- check for errors
	 * 3- call driver function?
	 */

#if 0
	// sanity check: check for invalid actions => FIXME: fake for oftest12, there are numerous
	// combinations, where an action list may be invalid, especially when heterogeneous tables
	// in terms of capabilities exist!
	for (cofbuckets::iterator it = msg->get_buckets().begin(); it != msg->get_buckets().end(); ++it) {
		cofbucket& bucket = (*it);
		for (cofaclist::iterator jt = bucket.actions.begin(); jt != bucket.actions.end(); ++jt) {
			cofaction& action = (*jt);
			switch (action.get_type()) {
			case OFPAT_OUTPUT: {
				if (be32toh(action.oac_output->port) == OFPP_ANY) {
					throw eBadActionBadOutPort();
				}
			} break;
			default: {
				// do nothing
			} break;
			}
		}
	}
#endif

	rofl_of1x_gm_result_t ret_val;
 	of1x_bucket_list_t* bucket_list=of1x_init_bucket_list();
	
	switch(msg.get_command()){
		case openflow13::OFPGC_ADD:
			of13_translation_utils::of13_map_bucket_list(&ctl, sw, msg.get_buckets(), bucket_list);
			ret_val = fwd_module_of1x_group_mod_add(sw->dpid, (of1x_group_type_t)msg.get_group_type(), msg.get_group_id(), &bucket_list);
			break;
			
		case openflow13::OFPGC_MODIFY:
			of13_translation_utils::of13_map_bucket_list(&ctl, sw, msg.get_buckets(), bucket_list);
			ret_val = fwd_module_of1x_group_mod_modify(sw->dpid, (of1x_group_type_t)msg.get_group_type(), msg.get_group_id(), &bucket_list);
			break;
		
		case openflow13::OFPGC_DELETE:
			ret_val = fwd_module_of1x_group_mod_delete(sw->dpid, msg.get_group_id());
			break;
		
		default:
			ret_val = ROFL_OF1X_GM_BCOMMAND;
			break;
	}
	if( (ret_val != ROFL_OF1X_GM_OK) || (msg.get_command() == openflow13::OFPGC_DELETE) )
		of1x_destroy_bucket_list(bucket_list);
	
	//Throw appropiate exception based on the return code
	switch(ret_val){
		case ROFL_OF1X_GM_OK:
			break;
		case ROFL_OF1X_GM_EXISTS:
			throw eGroupModExists();
			break;
		case ROFL_OF1X_GM_INVAL:
			throw eGroupModInvalGroup();
			break;
		case ROFL_OF1X_GM_WEIGHT:
			throw eGroupModWeightUnsupported();
			break;
		case ROFL_OF1X_GM_OGRUPS:
			throw eGroupModOutOfGroups();
			break;
		case ROFL_OF1X_GM_OBUCKETS:
			throw eGroupModOutOfBuckets();
			break;
		case ROFL_OF1X_GM_CHAIN:
			throw eGroupModChainingUnsupported();
			break;
		case ROFL_OF1X_GM_WATCH:
			throw eGroupModWatchUnsupported();
			break;
		case ROFL_OF1X_GM_LOOP:
			throw eGroupModLoop();
			break;
		case ROFL_OF1X_GM_UNKGRP:
			throw eGroupModUnknownGroup();
			break;
		case ROFL_OF1X_GM_CHNGRP:
			throw eGroupModChainedGroup();
			break;
		case ROFL_OF1X_GM_BTYPE:
			throw eGroupModBadType();
			break;
		case ROFL_OF1X_GM_BCOMMAND:
			throw eGroupModBadCommand();
			break;
		case ROFL_OF1X_GM_BBUCKET:
			throw eGroupModBadBucket();
			break;
		case ROFL_OF1X_GM_BWATCH:
			throw eGroupModBadWatch();
			break;
		case ROFL_OF1X_GM_EPERM:
			throw eGroupModEperm();
			break;
		default:
			/*Not a valid value - Log error*/
			break;
	}
}



void
of13_endpoint::handle_table_mod(
		crofctl& ctl,
		cofmsg_table_mod& msg,
		uint8_t aux_id)
{
	// table config is different in OF1.3
	// well, in fact, it's deprecated, but kept for backwards compatibility ;)
	// hence, we ignore incoming messages of type OFPT_TABLE_MOD
}



void
of13_endpoint::handle_port_mod(
		crofctl& ctl,
		cofmsg_port_mod& msg,
		uint8_t aux_id)
{




	uint32_t config, mask, advertise, port_num;

	config 		= msg.get_config();
	mask 		= msg.get_mask();
	advertise 	= msg.get_advertise();
	port_num 	= msg.get_port_no();

	//Check if port_num FLOOD
	//TODO: Inspect if this is right. Spec does not clearly define if this should be supported or not

	switch (port_num) {
	case rofl::openflow13::OFPP_MAX:
	case rofl::openflow13::OFPP_IN_PORT:
	case rofl::openflow13::OFPP_TABLE:
	case rofl::openflow13::OFPP_NORMAL:
	case rofl::openflow13::OFPP_FLOOD:
	case rofl::openflow13::OFPP_CONTROLLER:
	case rofl::openflow13::OFPP_LOCAL:
	case rofl::openflow13::OFPP_ANY: {
		throw ePortModBadPort();
	}
	case rofl::openflow13::OFPP_ALL: {

		of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)fwd_module_get_switch_snapshot_by_dpid(sw->dpid);
		if(!of13switch)
			throw eRofBase();
		//we check all the positions in case there are empty slots
		for (unsigned int n = 1; n < of13switch->max_ports; n++){
			logical_switch_port_t* ls_port = &of13switch->logical_ports[n];
			switch_port_snapshot_t* _port = ls_port->port;
			if(_port!=NULL && ls_port->attachment_state!=LOGICAL_PORT_STATE_DETACHED){
				//Mapping of port state
				assert(n == _port->of_port_num);

				port_set_config(sw->dpid, _port->of_port_num, config, mask, advertise);
			}
	 	}
		//Destroy the snapshot
		of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	} break;
	default: {

		port_set_config(sw->dpid, port_num, config, mask, advertise);

	};
	}
}



void
of13_endpoint::port_set_config(
		uint64_t dpid,
		uint32_t portno,
		uint32_t config,
		uint32_t mask,
		uint32_t advertise)
{

	//Drop received
	if( mask &  openflow13::OFPPC_NO_RECV )
		if( AFA_FAILURE == fwd_module_of1x_set_port_drop_received_config(dpid, portno, config & openflow13::OFPPC_NO_RECV ) )
			throw ePortModBase(); 
	//No forward
	if( mask &  openflow13::OFPPC_NO_FWD )
		if( AFA_FAILURE == fwd_module_of1x_set_port_forward_config(dpid, portno, !(config & openflow13::OFPPC_NO_FWD) ) )
			throw ePortModBase(); 
	//No packet in
	if( mask &  openflow13::OFPPC_NO_PACKET_IN )
		if( AFA_FAILURE == fwd_module_of1x_set_port_generate_packet_in_config(dpid, portno, !(config & openflow13::OFPPC_NO_PACKET_IN) ) )
			throw ePortModBase(); 

	//Advertised
	if( advertise )
		if( AFA_FAILURE == fwd_module_of1x_set_port_advertise_config(dpid, portno, advertise)  )
			throw ePortModBase(); 

	//Port admin down //TODO: evaluate if we can directly call fwd_module_enable_port_by_num instead
	if( mask &  openflow13::OFPPC_PORT_DOWN ){
		if( (config & openflow13::OFPPC_PORT_DOWN)  ){
			//Disable port
			if( AFA_FAILURE == fwd_module_bring_port_down_by_num(dpid, portno) ){
				throw ePortModBase(); 
			}
		}else{
			if( AFA_FAILURE == fwd_module_bring_port_up_by_num(dpid, portno) ){
				throw ePortModBase(); 
			}
		}
	}
#if 0
	/*
	 * in case of an error, use one of these exceptions:
	 */
	throw ePortModBadAdvertise();
	throw ePortModBadConfig();
	throw ePortModBadHwAddr();
	throw ePortModBadPort();
#endif
}




void
of13_endpoint::handle_queue_get_config_request(
		crofctl& ctl,
		cofmsg_queue_get_config_request& pack,
		uint8_t aux_id)
{
	switch_port_snapshot_t* port;
	unsigned int portnum = pack.get_port_no();

	//FIXME: send error? => yes, if portnum is unknown, just throw the appropriate exception
	if (0 /*add check for existence of port*/)
		throw eBadRequestBadPort();

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)fwd_module_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw eRofBase();


	cofpacket_queue_list pql(ctl.get_version());

	//we check all the positions in case there are empty slots
	for(unsigned int n = 1; n < of13switch->max_ports; n++){

		port = of13switch->logical_ports[n].port; 

		if(port == NULL)
			continue;

		if (of13switch->logical_ports[n].attachment_state != LOGICAL_PORT_STATE_ATTACHED)
			continue;

		if ((openflow13::OFPP_ALL != portnum) && (port->of_port_num != portnum))
			continue;

		for(unsigned int i=0; i<port->max_queues; i++){
			if(!port->queues[i].set)
				continue;

			cofpacket_queue pq(ctl.get_version());
			pq.set_queue_id(port->queues[i].id);
			pq.set_port(port->of_port_num);
			pq.get_queue_prop_list().next() = cofqueue_prop_min_rate(ctl.get_version(), port->queues[i].min_rate);
			pq.get_queue_prop_list().next() = cofqueue_prop_max_rate(ctl.get_version(), port->queues[i].max_rate);
			//fprintf(stderr, "min_rate: %d\n", port->queues[i].min_rate);
			//fprintf(stderr, "max_rate: %d\n", port->queues[i].max_rate);

			pql.next() = pq;
		}
	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);
		
	ctl.send_queue_get_config_reply(pack.get_xid(), pack.get_port_no(), pql);
}



void
of13_endpoint::handle_experimenter_message(
		crofctl& ctl,
		cofmsg_experimenter& pack,
		uint8_t aux_id)
{
	// TODO
}



void
of13_endpoint::handle_ctrl_open(crofctl *ctrl)
{
	ROFL_INFO("[sw: %s]Controller %s:%u is in CONNECTED state. \n", sw->dpname.c_str() , ctrl->get_peer_addr().c_str()); //FIXME: add role
}



void
of13_endpoint::handle_ctrl_close(crofctl *ctrl)
{
	ROFL_INFO("[sw: %s] Controller %s:%u has DISCONNECTED. \n", sw->dpname.c_str() ,ctrl->get_peer_addr().c_str()); //FIXME: add role

}

