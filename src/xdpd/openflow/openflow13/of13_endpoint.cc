#include "of13_endpoint.h"

#include <rofl/datapath/hal/driver.h>
#include <rofl/common/utils/c_logger.h>
#include "of13_translation_utils.h"
#include "../../management/system_manager.h"

using namespace xdpd;

/*
* Constructor and destructor
*/
of13_endpoint::of13_endpoint(
		openflow_switch* sw,
		int reconnect_start_timeout,
		const rofl::openflow::cofhello_elem_versionbitmap& versionbitmap,
		enum rofl::csocket::socket_type_t socket_type,
		cparams const& socket_params) throw (eOfSmErrorOnCreation) :
				of_endpoint(versionbitmap, socket_type, socket_params) {

	//Reference back to the sw
	this->sw = sw;

	//Connect to controller
	crofbase::add_ctl(crofbase::get_idle_ctlid(), versionbitmap).connect(rofl::cauxid(0), socket_type, socket_params);
}

/*
*
* Handling endpoint messages routines
*
*/

void
of13_endpoint::handle_ctl_close(crofctl& ctl)
{
	crofbase::add_ctl(crofbase::get_idle_ctlid(), versionbitmap).connect(rofl::cauxid(0), socket_type, socket_params);
}



void
of13_endpoint::handle_features_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_features_request& msg)
{
	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)hal_driver_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw rofl::eRofBase();
	
	uint32_t num_of_tables 	= of13switch->pipeline.num_of_tables;
	uint32_t num_of_buffers = of13switch->pipeline.num_of_buffers;
	uint32_t capabilities 	= of13switch->pipeline.capabilities;
	
	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_features_reply(
			auxid,
			msg.get_xid(),
			sw->dpid,
			num_of_buffers,	// n_buffers
			num_of_tables,	// n_tables
			capabilities,	// capabilities
			auxid.get_id());	// auxiliary connection id
}



void
of13_endpoint::handle_get_config_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_get_config_request& msg)
{
	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)hal_driver_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw rofl::eRofBase();
	
	uint16_t flags 			= of13switch->pipeline.capabilities;
	uint16_t miss_send_len 	= of13switch->pipeline.miss_send_len;

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_get_config_reply(auxid, msg.get_xid(), flags, miss_send_len);
}



void
of13_endpoint::handle_set_config(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_set_config& msg)
{
	//Instruct the driver to process the set config
	if(HAL_FAILURE == hal_driver_of1x_set_pipeline_config(sw->dpid, msg.get_flags(), msg.get_miss_send_len())){
		throw rofl::eSwitchConfigBadFlags();
	}
}



void
of13_endpoint::handle_packet_out(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_packet_out& msg)
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
	if (HAL_FAILURE == hal_driver_of1x_process_packet_out(sw->dpid,
							msg.get_buffer_id(),
							msg.get_in_port(),
							action_group,
							msg.get_packet().soframe(), msg.get_packet().length())){
		// log error
		//FIXME: send error
	}

	of1x_destroy_action_group(action_group);
}



void
of13_endpoint::handle_flow_mod(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_flow_mod& msg)
{
	switch (msg.get_flowmod().get_command()) {
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
		throw rofl::eFlowModBadCommand();
	}
}



void
of13_endpoint::flow_mod_add(
		rofl::crofctl& ctl,
		rofl::openflow::cofmsg_flow_mod& msg)
{
	uint8_t table_id = msg.get_flowmod().get_table_id();
	hal_result_t res;
	of1x_flow_entry_t *entry=NULL;

	// sanity check: table for table-id must exist
	if ( (table_id > sw->num_of_tables) && (table_id != openflow13::OFPTT_ALL) ){
		rofl::logging::error << "[xdpd][of13][flow-mod-add] unable to add flow-mod due to " <<
				"invalid table-id:" << msg.get_flowmod().get_table_id() << " on dpt:" << sw->dpname << std::endl;
		throw rofl::eFlowModBadTableId();
	}

	try{
		entry = of13_translation_utils::of13_map_flow_entry(&ctl, &msg, sw);
	}catch(...){
		rofl::logging::error << "[xdpd][of13][flow-mod-add] unable to map flow-mod entry to internal representation on dpt:" << sw->dpname << std::endl;
		throw rofl::eFlowModUnknown();
	}

	if(!entry){
		throw rofl::eFlowModUnknown();//Just for safety, but shall never reach this
	}

	if (HAL_SUCCESS != (res = hal_driver_of1x_process_flow_mod_add(sw->dpid,
								msg.get_flowmod().get_table_id(),
								&entry,
								msg.get_flowmod().get_buffer_id(),
								msg.get_flowmod().get_flags() & openflow13::OFPFF_CHECK_OVERLAP,
								msg.get_flowmod().get_flags() & openflow13::OFPFF_RESET_COUNTS))){
		// log error
		rofl::logging::error << "[xdpd][of13][flow-mod-add] error inserting flow-mod on dpt:" << sw->dpname << std::endl;
		of1x_destroy_flow_entry(entry);

		if(res == HAL_FM_OVERLAP_FAILURE){
			throw rofl::eFlowModOverlap();
		}else{
			throw rofl::eFlowModTableFull();
		}
	}
}



void
of13_endpoint::flow_mod_modify(
		rofl::crofctl& ctl,
		rofl::openflow::cofmsg_flow_mod& pack,
		bool strict)
{
	of1x_flow_entry_t *entry=NULL;

	// sanity check: table for table-id must exist
	if (pack.get_flowmod().get_table_id() > sw->num_of_tables)
	{
		rofl::logging::error << "[xdpd][of13][flow-mod-modify] unable to modify flow-mod due to " <<
				"invalid table-id:" << pack.get_flowmod().get_table_id() << " on dpt:" << sw->dpname << std::endl;
		throw rofl::eFlowModBadTableId();
	}

	try{
		entry = of13_translation_utils::of13_map_flow_entry(&ctl, &pack, sw);
	}catch(...){
		rofl::logging::error << "[xdpd][of13][flow-mod-modify] unable to map flow-mod entry to internal representation on dpt:" << sw->dpname << std::endl;
		throw rofl::eFlowModUnknown();
	}

	if(!entry){
		throw rofl::eFlowModUnknown();//Just for safety, but shall never reach this
	}


	of1x_flow_removal_strictness_t strictness = (strict) ? STRICT : NOT_STRICT;


	if(HAL_SUCCESS != hal_driver_of1x_process_flow_mod_modify(sw->dpid,
								pack.get_flowmod().get_table_id(),
								&entry,
								pack.get_flowmod().get_buffer_id(),
								strictness,
								pack.get_flowmod().get_flags() & openflow13::OFPFF_RESET_COUNTS)){
		rofl::logging::error << "[xdpd][of13][flow-mod-modify] error modifying flow-mod on dpt:" << sw->dpname << std::endl;
		of1x_destroy_flow_entry(entry);

		throw rofl::eFlowModBase();
	}
}



void
of13_endpoint::flow_mod_delete(
		rofl::crofctl& ctl,
		rofl::openflow::cofmsg_flow_mod& pack,
		bool strict)
{

	of1x_flow_entry_t *entry=NULL;

	try{
		entry = of13_translation_utils::of13_map_flow_entry(&ctl, &pack, sw);
	}catch(...){
		rofl::logging::error << "[xdpd][of13][flow-mod-delete] unable to map flow-mod entry to internal representation on dpt:" << sw->dpname << std::endl;
		throw rofl::eFlowModUnknown();
	}

	if(!entry)
		throw rofl::eFlowModUnknown();//Just for safety, but shall never reach this


	of1x_flow_removal_strictness_t strictness = (strict) ? STRICT : NOT_STRICT;

	if(HAL_SUCCESS != hal_driver_of1x_process_flow_mod_delete(sw->dpid,
								pack.get_flowmod().get_table_id(),
								entry,
								pack.get_flowmod().get_out_port(),
								pack.get_flowmod().get_out_group(),
								strictness)) {
		rofl::logging::error << "[xdpd][of13][flow-mod-delete] error deleting flow-mod on dpt:" << sw->dpname << std::endl;
		of1x_destroy_flow_entry(entry);
		throw rofl::eFlowModBase();
	}

	//Always delete entry
	of1x_destroy_flow_entry(entry);
}



void
of13_endpoint::handle_desc_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_desc_stats_request& msg)
{

	std::string mfr_desc(PACKAGE_NAME);
	std::string hw_desc(VERSION);
	std::string sw_desc(VERSION);

	rofl::openflow::cofdesc_stats_reply desc_stats(
			ctl.get_version_negotiated(),
			mfr_desc,
			hw_desc,
			sw_desc,
			system_manager::get_id(),
			system_manager::get_driver_description()
			);

	ctl.send_desc_stats_reply(auxid, msg.get_xid(), desc_stats);
}



void
of13_endpoint::handle_flow_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_flow_stats_request& msg)
{
	//Map the match structure from OpenFlow to packet_matches_t
	of1x_flow_entry_t* entry = of1x_init_flow_entry(false);

	try{
		of13_translation_utils::of13_map_flow_entry_matches(&ctl, msg.get_flow_stats().get_match(), sw, entry);
	}catch(...){
		of1x_destroy_flow_entry(entry);
		throw rofl::eBadRequestBadStat();
	}

	//Ask the Forwarding Plane to process stats
	of1x_stats_flow_msg_t* fp_msg = hal_driver_of1x_get_flow_stats(
													sw->dpid,
													msg.get_flow_stats().get_table_id(),
													msg.get_flow_stats().get_cookie(),
													msg.get_flow_stats().get_cookie_mask(),
													msg.get_flow_stats().get_out_port(),
													msg.get_flow_stats().get_out_group(),
													&entry->matches);

	if(!fp_msg){
		of1x_destroy_flow_entry(entry);
		throw rofl::eBadRequestBadStat();
	}

	//Construct OF message
	of1x_stats_single_flow_msg_t *elem = fp_msg->flows_head;

	rofl::openflow::cofflowstatsarray flowstatsarray(ctl.get_version_negotiated());

	uint32_t flow_id = 0;

	for(elem = fp_msg->flows_head; elem; elem = elem->next){

		rofl::openflow::cofmatch match(rofl::openflow13::OFP_VERSION);
		of13_translation_utils::of13_map_reverse_flow_entry_matches(elem->matches, match);

		rofl::openflow::cofinstructions instructions(ctl.get_version_negotiated());
		of13_translation_utils::of13_map_reverse_flow_entry_instructions((of1x_instruction_group_t*)(elem->inst_grp), instructions);


		flowstatsarray.set_flow_stats(flow_id).set_table_id(elem->table_id);
		flowstatsarray.set_flow_stats(flow_id).set_duration_sec(elem->duration_sec);
		flowstatsarray.set_flow_stats(flow_id).set_duration_nsec(elem->duration_nsec);
		flowstatsarray.set_flow_stats(flow_id).set_priority(elem->priority);
		flowstatsarray.set_flow_stats(flow_id).set_idle_timeout(elem->idle_timeout);
		flowstatsarray.set_flow_stats(flow_id).set_hard_timeout(elem->hard_timeout);
		flowstatsarray.set_flow_stats(flow_id).set_flags(elem->flags);
		flowstatsarray.set_flow_stats(flow_id).set_cookie(elem->cookie);
		flowstatsarray.set_flow_stats(flow_id).set_packet_count(elem->packet_count);
		flowstatsarray.set_flow_stats(flow_id).set_byte_count(elem->byte_count);
		flowstatsarray.set_flow_stats(flow_id).set_match() = match;
		flowstatsarray.set_flow_stats(flow_id).set_instructions() = instructions;

		flow_id++;
	}


	try{
		//Send message
		ctl.send_flow_stats_reply(auxid, msg.get_xid(), flowstatsarray);
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
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_aggr_stats_request& msg)
{
	//Map the match structure from OpenFlow to packet_matches_t
	 of1x_flow_entry_t* entry = of1x_init_flow_entry(false);

	if(!entry)
		throw rofl::eBadRequestBadStat();

	try{
		of13_translation_utils::of13_map_flow_entry_matches(&ctl, msg.get_aggr_stats().get_match(), sw, entry);
	}catch(...){
		of1x_destroy_flow_entry(entry);
		throw rofl::eBadRequestBadStat();
	}

	//TODO check error while mapping

	//Ask the Forwarding Plane to process stats
	of1x_stats_flow_aggregate_msg_t* fp_msg = hal_driver_of1x_get_flow_aggregate_stats(sw->dpid,
					msg.get_aggr_stats().get_table_id(),
					msg.get_aggr_stats().get_cookie(),
					msg.get_aggr_stats().get_cookie_mask(),
					msg.get_aggr_stats().get_out_port(),
					msg.get_aggr_stats().get_out_group(),
					&entry->matches);

	if(!fp_msg){
		of1x_destroy_flow_entry(entry);
		throw rofl::eBadRequestBadStat();
	}

	try{
		rofl::openflow::cofaggr_stats_reply aggr_stats_reply(
				ctl.get_version_negotiated(),
				fp_msg->packet_count,
				fp_msg->byte_count,
				fp_msg->flow_count);
		//Construct OF message
		ctl.send_aggr_stats_reply(auxid, msg.get_xid(), aggr_stats_reply);
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
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_table_stats_request& msg)
{
	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)hal_driver_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw rofl::eRofBase();
	
	unsigned int num_of_tables = of13switch->pipeline.num_of_tables;
	rofl::openflow::coftablestatsarray tablestatsarray(ctl.get_version_negotiated());

	for (unsigned int n = 0; n < num_of_tables; n++) {
	
		of1x_flow_table_t *table = &of13switch->pipeline.tables[n];
		uint8_t table_id = table->number;

		tablestatsarray.set_table_stats(table_id).set_table_id(table->number);
		tablestatsarray.set_table_stats(table_id).set_active_count(table->num_of_entries);
		tablestatsarray.set_table_stats(table_id).set_lookup_count(table->stats.s.counters.lookup_count);
		tablestatsarray.set_table_stats(table_id).set_matched_count(table->stats.s.counters.matched_count);
	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_table_stats_reply(auxid, msg.get_xid(), tablestatsarray, false);
}



void
of13_endpoint::handle_port_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_port_stats_request& msg)
{

	switch_port_snapshot_t* port;
	uint32_t port_no = msg.get_port_stats().get_portno();

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)hal_driver_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw rofl::eRofBase();

	rofl::openflow::cofportstatsarray portstatsarray(ctl.get_version_negotiated());

	/*
	 *  send statistics for all ports
	 */
	if (openflow13::OFPP_ANY == port_no){

		//we check all the positions in case there are empty slots
		for (unsigned int n = 1; n < of13switch->max_ports; n++){
	
			port = of13switch->logical_ports[n].port; 
	
			if((port != NULL) && (of13switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED)){

				portstatsarray.set_port_stats(port->of_port_num).set_port_no(port->of_port_num);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_packets(port->stats.rx_packets);
				portstatsarray.set_port_stats(port->of_port_num).set_tx_packets(port->stats.tx_packets);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_bytes(port->stats.rx_bytes);
				portstatsarray.set_port_stats(port->of_port_num).set_tx_bytes(port->stats.tx_bytes);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_dropped(port->stats.rx_dropped);
				portstatsarray.set_port_stats(port->of_port_num).set_tx_dropped(port->stats.tx_dropped);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_errors(port->stats.rx_errors);
				portstatsarray.set_port_stats(port->of_port_num).set_tx_errors(port->stats.tx_errors);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_frame_err(port->stats.rx_frame_err);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_over_err(port->stats.rx_over_err);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_crc_err(port->stats.rx_crc_err);
				portstatsarray.set_port_stats(port->of_port_num).set_collisions(port->stats.collisions);
				portstatsarray.set_port_stats(port->of_port_num).set_duration_sec(0); 	// TODO
				portstatsarray.set_port_stats(port->of_port_num).set_duration_nsec(0); 	// TODO
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
				portstatsarray.set_port_stats(port->of_port_num).set_port_no(port->of_port_num);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_packets(port->stats.rx_packets);
				portstatsarray.set_port_stats(port->of_port_num).set_tx_packets(port->stats.tx_packets);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_bytes(port->stats.rx_bytes);
				portstatsarray.set_port_stats(port->of_port_num).set_tx_bytes(port->stats.tx_bytes);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_dropped(port->stats.rx_dropped);
				portstatsarray.set_port_stats(port->of_port_num).set_tx_dropped(port->stats.tx_dropped);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_errors(port->stats.rx_errors);
				portstatsarray.set_port_stats(port->of_port_num).set_tx_errors(port->stats.tx_errors);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_frame_err(port->stats.rx_frame_err);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_over_err(port->stats.rx_over_err);
				portstatsarray.set_port_stats(port->of_port_num).set_rx_crc_err(port->stats.rx_crc_err);
				portstatsarray.set_port_stats(port->of_port_num).set_collisions(port->stats.collisions);
				portstatsarray.set_port_stats(port->of_port_num).set_duration_sec(0); 	// TODO
				portstatsarray.set_port_stats(port->of_port_num).set_duration_nsec(0); 	// TODO


				break;
			}
	 	}

		// if port_no was not found, body.memlen() is 0
	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);

	ctl.send_port_stats_reply(auxid, msg.get_xid(), portstatsarray, false);
}




void
of13_endpoint::handle_queue_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_queue_stats_request& pack)
{

	switch_port_snapshot_t* port = NULL;
	unsigned int portnum = pack.get_queue_stats().get_port_no();
	unsigned int queue_id = pack.get_queue_stats().get_queue_id();

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)hal_driver_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw rofl::eRofBase();


	if( ((portnum >= of13switch->max_ports) && (portnum != openflow13::OFPP_ALL)) || portnum == 0){
		//Destroy the snapshot
		of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);
		throw rofl::eBadRequestBadPort(); 	//Invalid port num
	}

	rofl::openflow::cofqueuestatsarray queuestatsarray(ctl.get_version_negotiated());

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
					queuestatsarray.set_queue_stats(port->of_port_num, i).set_port_no(port->of_port_num);
					queuestatsarray.set_queue_stats(port->of_port_num, i).set_queue_id(i);
					queuestatsarray.set_queue_stats(port->of_port_num, i).set_tx_bytes(port->queues[i].stats.tx_bytes);
					queuestatsarray.set_queue_stats(port->of_port_num, i).set_tx_packets(port->queues[i].stats.tx_packets);
					queuestatsarray.set_queue_stats(port->of_port_num, i).set_tx_errors(port->queues[i].stats.overrun);
					queuestatsarray.set_queue_stats(port->of_port_num, i).set_duration_sec(0); 	// TODO
					queuestatsarray.set_queue_stats(port->of_port_num, i).set_duration_nsec(0); // TODO
				}

			} else {

				if(queue_id >= port->max_queues){
					//Destroy the snapshot
					of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);
					throw rofl::eBadRequestBadPort(); 	//FIXME send a BadQueueId error
				}

				//Check if the queue is really in use
				if(port->queues[queue_id].set){
					//Set values

					queuestatsarray.set_queue_stats(portnum, queue_id).set_port_no(portnum);
					queuestatsarray.set_queue_stats(portnum, queue_id).set_queue_id(queue_id);
					queuestatsarray.set_queue_stats(portnum, queue_id).set_tx_bytes(port->queues[queue_id].stats.tx_bytes);
					queuestatsarray.set_queue_stats(portnum, queue_id).set_tx_packets(port->queues[queue_id].stats.tx_packets);
					queuestatsarray.set_queue_stats(portnum, queue_id).set_tx_errors(port->queues[queue_id].stats.overrun);
					queuestatsarray.set_queue_stats(portnum, queue_id).set_duration_sec(0); 	// TODO
					queuestatsarray.set_queue_stats(portnum, queue_id).set_duration_nsec(0); 	// TODO
				}
			}
		}
	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);
	
	ctl.send_queue_stats_reply(auxid, pack.get_xid(), queuestatsarray);
}



void
of13_endpoint::handle_group_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_group_stats_request& msg)
{
	// we need to get the statistics, build a packet and send it
	unsigned int i;
	cmemory body(0);
	unsigned int num_of_buckets;
	of1x_stats_group_msg_t *g_msg, *g_msg_all;

	uint32_t group_id = msg.get_group_stats().get_group_id();
	
	g_msg_all = hal_driver_of1x_get_group_stats(sw->dpid, group_id);
	
	if(g_msg_all==NULL){
		//TODO handle error
		logging::error << "[xdpd][of13][group-stats] unable to retrieve group statistics from pipeline" << std::endl;
	}
	
	rofl::openflow::cofgroupstatsarray groups(ctl.get_version_negotiated());
	
	for(g_msg = g_msg_all; g_msg; g_msg = g_msg->next){
		num_of_buckets = g_msg->num_of_buckets;

		groups.set_group_stats(g_msg->group_id).set_group_id(g_msg->group_id);
		groups.set_group_stats(g_msg->group_id).set_ref_count(g_msg->ref_count);
		groups.set_group_stats(g_msg->group_id).set_packet_count(g_msg->packet_count);
		groups.set_group_stats(g_msg->group_id).set_byte_count(g_msg->byte_count);
		groups.set_group_stats(g_msg->group_id).set_duration_sec(0);
		groups.set_group_stats(g_msg->group_id).set_duration_nsec(0);
		for(i=0;i<num_of_buckets;i++) {
			groups.set_group_stats(g_msg->group_id).set_bucket_counters().set_bucket_counter(i).set_packet_count(g_msg->bucket_stats[i].packet_count);
			groups.set_group_stats(g_msg->group_id).set_bucket_counters().set_bucket_counter(i).set_byte_count(g_msg->bucket_stats[i].byte_count);
		}
	}

	try{
		//Send the group stats
		ctl.send_group_stats_reply(auxid, msg.get_xid(), groups);
	}catch(...){
		of1x_destroy_stats_group_msg(g_msg_all);
		throw;
	}
	
	//Destroy the g_msg
	of1x_destroy_stats_group_msg(g_msg_all);
}



void
of13_endpoint::handle_group_desc_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_group_desc_stats_request& msg)
{
	rofl::openflow::cofgroupdescstatsarray groupdescs(ctl.get_version_negotiated());
	
	of1x_stats_group_desc_msg_t *grp_dsc, *group_it;

	grp_dsc = hal_driver_of1x_get_group_desc_stats(sw->dpid);
	if ( grp_dsc == NULL ){
		return;
		//TODO throw rofl::exeption
	}
	
	for(group_it=grp_dsc;group_it;group_it=group_it->next){
		rofl::openflow::cofbuckets bclist(ctl.get_version_negotiated());
		of13_translation_utils::of13_map_reverse_bucket_list(bclist,group_it->bucket);

		groupdescs.set_group_desc_stats(group_it->group_id).set_group_type(group_it->type);
		groupdescs.set_group_desc_stats(group_it->group_id).set_group_id(group_it->group_id);
		groupdescs.set_group_desc_stats(group_it->group_id).set_buckets(bclist);
	}

	of1x_destroy_group_desc_stats(grp_dsc);
	
	ctl.send_group_desc_stats_reply(auxid, msg.get_xid(), groupdescs);
}



void
of13_endpoint::handle_group_features_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_group_features_stats_request& msg)
{
	rofl::openflow::cofgroup_features_stats_reply group_features_reply(ctl.get_version_negotiated());

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

	ctl.send_group_features_stats_reply(auxid, msg.get_xid(), group_features_reply);
}



void
of13_endpoint::handle_meter_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_meter_stats_request& msg)
{
	rofl::openflow::cofmeterstatsarray meter_stats_reply(rofl::openflow13::OFP_VERSION);

	// TODO: adjust meter_stats_reply, when meters have been implemented in the pipeline/in the drivers

	ctl.send_meter_stats_reply(auxid, msg.get_xid(), meter_stats_reply);
}



void
of13_endpoint::handle_meter_config_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_meter_config_stats_request& msg)
{
	rofl::openflow::cofmeterconfigarray meter_config_reply(rofl::openflow13::OFP_VERSION);

	// TODO: adjust meter_config_reply, when meters have been implemented in the pipeline/in the drivers

	ctl.send_meter_config_stats_reply(auxid, msg.get_xid(), meter_config_reply);
}



void
of13_endpoint::handle_meter_features_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_meter_features_stats_request& msg)
{
	rofl::openflow::cofmeter_features_reply meter_features_reply(rofl::openflow13::OFP_VERSION);

	meter_features_reply.set_max_meter(0);	// no meters supported
	meter_features_reply.set_band_types(0);
	meter_features_reply.set_capabilities(0);
	meter_features_reply.set_max_bands(0);
	meter_features_reply.set_max_color(0);

	// TODO: adjust meter_features_reply, when meters have been implemented in the pipeline/in the drivers

	ctl.send_meter_features_stats_reply(auxid, msg.get_xid(), meter_features_reply);
}



void
of13_endpoint::handle_table_features_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_table_features_stats_request& msg)
{
	// TODO: check for pipeline definition within request and configure pipeline accordingly

	rofl::openflow::coftables tables(ctl.get_version_negotiated());

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)hal_driver_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw rofl::eRofBase();

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
			of13_translation_utils::of13_map_bitmap_set_fields(&tc->apply_actions, tables.set_table(table_id).set_properties().set_tfp_apply_setfield());
			of13_translation_utils::of13_map_bitmap_set_fields(&tc->apply_actions, tables.set_table(table_id).set_properties().set_tfp_apply_setfield_miss());
		}
		if(tc->instructions & ( 1 << OF1X_IT_WRITE_ACTIONS)) {
			of13_translation_utils::of13_map_bitmap_actions(&tc->write_actions, tables.set_table(table_id).set_properties().set_tfp_write_actions());
			of13_translation_utils::of13_map_bitmap_actions(&tc->write_actions, tables.set_table(table_id).set_properties().set_tfp_write_actions_miss());
			of13_translation_utils::of13_map_bitmap_set_fields(&tc->write_actions, tables.set_table(table_id).set_properties().set_tfp_write_setfield());
			of13_translation_utils::of13_map_bitmap_set_fields(&tc->write_actions, tables.set_table(table_id).set_properties().set_tfp_write_setfield_miss());
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

	ctl.send_table_features_stats_reply(auxid, msg.get_xid(), tables, 0);
}



void
of13_endpoint::handle_port_desc_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_port_desc_stats_request& msg)
{
	logical_switch_port_t* ls_port;
	switch_port_snapshot_t* _port;

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)hal_driver_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw rofl::eRofBase();

	// array of structures ofp_port
	rofl::openflow::cofports ports(ctl.get_version_negotiated());

	//we check all the positions in case there are empty slots
	for (unsigned int n = 1; n < of13switch->max_ports; n++){

		ls_port = &of13switch->logical_ports[n];
		_port = ls_port->port;

		if(_port!=NULL && ls_port->attachment_state!=LOGICAL_PORT_STATE_DETACHED){

			//Mapping of port state
			assert(n == _port->of_port_num);

			rofl::openflow::cofport port(ctl.get_version_negotiated());

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

	ctl.send_port_desc_stats_reply(auxid, msg.get_xid(), ports, 0);
}



void
of13_endpoint::handle_experimenter_stats_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_experimenter_stats_request& pack)
{
	//TODO: when exp are supported 
}







rofl_result_t
of13_endpoint::process_packet_in(
		uint8_t table_id,
		uint8_t reason,
		uint32_t in_port,
		uint32_t buffer_id,
		uint64_t cookie,
		uint8_t* pkt_buffer,
		uint32_t buf_len,
		uint16_t total_len,
		packet_matches_t* matches)
{
	try {
		//Transform matches 
		rofl::openflow::cofmatch match(rofl::openflow13::OFP_VERSION);
		of13_translation_utils::of13_map_reverse_packet_matches(matches, match);

		size_t len = (total_len < buf_len) ? total_len : buf_len;

		rofl::crofbase::send_packet_in_message(
				cauxid(0),
				buffer_id,
				total_len,
				reason,
				table_id,
				cookie,
				/*in_port=*/0, // OF1.0 only
				match,
				pkt_buffer, len);

		return ROFL_SUCCESS;

	} catch (...) {

#if 0
		if (buffer_id == OF1XP_NO_BUFFER) {
			rofl::logging::error << "[xdpd][of13][packet-in] unable to send Packet-In message" << std::endl;

			return HAL_FAILURE;
		}

		rofl::logging::error << "[xdpd][of13][packet-in] unable to send Packet-In message, dropping packet from occupied pkt slot" << std::endl;

		of1x_action_group_t* action_group = of1x_init_action_group(NULL);

		try{
			rofl::cofactions actions(rofl::openflow13::OFP_VERSION);
			of13_translation_utils::of13_map_flow_entry_actions(NULL, sw, actions, action_group, NULL);
		}catch(...){
			of1x_destroy_action_group(action_group);
			return HAL_FAILURE;
		}

		/* assumption: driver can handle all situations properly:
		 * - data and datalen both 0 and buffer_id != OFP_NO_BUFFER
		 * - buffer_id == OFP_NO_BUFFER and data and datalen both != 0
		 * - everything else is an error?
		 */
		if (HAL_FAILURE == hal_driver_of1x_process_packet_out(sw->dpid,
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

	try {
		uint32_t config=0x0;
	
		//Compose port config
		if(!port->up) config |= openflow13::OFPPC_PORT_DOWN;
		if(!port->of_generate_packet_in) config |= openflow13::OFPPC_NO_PACKET_IN;
		if(!port->forward_packets) config |= openflow13::OFPPC_NO_FWD;
		if(port->drop_received) config |= openflow13::OFPPC_NO_RECV;


		rofl::openflow::cofport ofport(openflow13::OFP_VERSION);
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
		rofl::crofbase::send_port_status_message(cauxid(0), openflow13::OFPPR_ADD, ofport);
	
		return ROFL_SUCCESS;


	} catch (...) {

		return ROFL_FAILURE;
	}

}

rofl_result_t of13_endpoint::notify_port_detached(const switch_port_snapshot_t* port){

	try {
		uint32_t config=0x0;
	
		//Compose port config
		if(!port->up) config |= openflow13::OFPPC_PORT_DOWN;
		if(!port->of_generate_packet_in) config |= openflow13::OFPPC_NO_PACKET_IN;
		if(!port->forward_packets) config |= openflow13::OFPPC_NO_FWD;
		if(port->drop_received) config |= openflow13::OFPPC_NO_RECV;

		rofl::openflow::cofport ofport(openflow13::OFP_VERSION);
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
		rofl::crofbase::send_port_status_message(cauxid(0), openflow13::OFPPR_DELETE, ofport);
	
		return ROFL_SUCCESS;

	} catch (...) {

		return ROFL_FAILURE;
	}

}

rofl_result_t of13_endpoint::notify_port_status_changed(const switch_port_snapshot_t* port){

	try {
		uint32_t config=0x0;

		//Compose port config
		if(!port->up) config |= openflow13::OFPPC_PORT_DOWN;
		if(!port->of_generate_packet_in) config |= openflow13::OFPPC_NO_PACKET_IN;
		if(!port->forward_packets) config |= openflow13::OFPPC_NO_FWD;
		if(port->drop_received) config |= openflow13::OFPPC_NO_RECV;

		//Notify OF controller
		rofl::openflow::cofport ofport(openflow13::OFP_VERSION);
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
		rofl::crofbase::send_port_status_message(cauxid(0), openflow13::OFPPR_MODIFY, ofport);
	
		return ROFL_SUCCESS; // ignore this notification

	
	} catch (...) {

		return ROFL_FAILURE;
	}

}





void
of13_endpoint::handle_barrier_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_barrier_request& pack)
{
	//Since we are not queuing messages currently
	ctl.send_barrier_reply(auxid, pack.get_xid());
}









rofl_result_t
of13_endpoint::process_flow_removed(
		uint8_t reason,
		of1x_flow_entry *entry)
{
	try {

		rofl::openflow::cofmatch match(rofl::openflow13::OFP_VERSION);
		uint32_t sec,nsec;

		of13_translation_utils::of13_map_reverse_flow_entry_matches(entry->matches.head, match);

		//get duration of the flow mod
		of1x_stats_flow_get_duration(entry, &sec, &nsec);

		rofl::crofbase::send_flow_removed_message(
				cauxid(0),
				match,
				entry->cookie,
				entry->priority,
				reason,
				entry->table->number,
				sec,
				nsec,
				entry->timer_info.idle_timeout,
				entry->timer_info.hard_timeout,
				entry->stats.s.counters.packet_count,
				entry->stats.s.counters.byte_count);

		return ROFL_SUCCESS;

	} catch (...) {

		return ROFL_FAILURE;
	}

}



void
of13_endpoint::handle_group_mod(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_group_mod& msg)
{
	//throw rofl::eNotImplemented(std::string("of13_endpoint::handle_group_mod()"));
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
					throw rofl::eBadActionBadOutPort();
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
			ret_val = hal_driver_of1x_group_mod_add(sw->dpid, (of1x_group_type_t)msg.get_group_type(), msg.get_group_id(), &bucket_list);
			break;
			
		case openflow13::OFPGC_MODIFY:
			of13_translation_utils::of13_map_bucket_list(&ctl, sw, msg.get_buckets(), bucket_list);
			ret_val = hal_driver_of1x_group_mod_modify(sw->dpid, (of1x_group_type_t)msg.get_group_type(), msg.get_group_id(), &bucket_list);
			break;
		
		case openflow13::OFPGC_DELETE:
			ret_val = hal_driver_of1x_group_mod_delete(sw->dpid, msg.get_group_id());
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
			throw rofl::eGroupModExists();
			break;
		case ROFL_OF1X_GM_INVAL:
			throw rofl::eGroupModInvalGroup();
			break;
		case ROFL_OF1X_GM_WEIGHT:
			throw rofl::eGroupModWeightUnsupported();
			break;
		case ROFL_OF1X_GM_OGRUPS:
			throw rofl::eGroupModOutOfGroups();
			break;
		case ROFL_OF1X_GM_OBUCKETS:
			throw rofl::eGroupModOutOfBuckets();
			break;
		case ROFL_OF1X_GM_CHAIN:
			throw rofl::eGroupModChainingUnsupported();
			break;
		case ROFL_OF1X_GM_WATCH:
			throw rofl::eGroupModWatchUnsupported();
			break;
		case ROFL_OF1X_GM_LOOP:
			throw rofl::eGroupModLoop();
			break;
		case ROFL_OF1X_GM_UNKGRP:
			throw rofl::eGroupModUnknownGroup();
			break;
		case ROFL_OF1X_GM_CHNGRP:
			throw rofl::eGroupModChainedGroup();
			break;
		case ROFL_OF1X_GM_BTYPE:
			throw rofl::eGroupModBadType();
			break;
		case ROFL_OF1X_GM_BCOMMAND:
			throw rofl::eGroupModBadCommand();
			break;
		case ROFL_OF1X_GM_BBUCKET:
			throw rofl::eGroupModBadBucket();
			break;
		case ROFL_OF1X_GM_BWATCH:
			throw rofl::eGroupModBadWatch();
			break;
		case ROFL_OF1X_GM_EPERM:
			throw rofl::eGroupModEperm();
			break;
		default:
			/*Not a valid value - Log error*/
			break;
	}
}



void
of13_endpoint::handle_table_mod(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_table_mod& msg)
{
	// table config is different in OF1.3
	// well, in fact, it's deprecated, but kept for backwards compatibility ;)
	// hence, we ignore incoming messages of type OFPT_TABLE_MOD
}



void
of13_endpoint::handle_port_mod(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_port_mod& msg)
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
	case rofl::openflow13::OFPP_LOCAL: {
		throw rofl::ePortModBadPort();
	}
	case rofl::openflow13::OFPP_ANY: // be liberal what you accept, and conservative, what you send
	case rofl::openflow13::OFPP_ALL: {

		of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)hal_driver_get_switch_snapshot_by_dpid(sw->dpid);
		if(!of13switch)
			throw rofl::eRofBase();
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
		if( HAL_FAILURE == hal_driver_of1x_set_port_drop_received_config(dpid, portno, config & openflow13::OFPPC_NO_RECV ) )
			throw rofl::ePortModBase();
	//No forward
	if( mask &  openflow13::OFPPC_NO_FWD )
		if( HAL_FAILURE == hal_driver_of1x_set_port_forward_config(dpid, portno, !(config & openflow13::OFPPC_NO_FWD) ) )
			throw rofl::ePortModBase();
	//No packet in
	if( mask &  openflow13::OFPPC_NO_PACKET_IN )
		if( HAL_FAILURE == hal_driver_of1x_set_port_generate_packet_in_config(dpid, portno, !(config & openflow13::OFPPC_NO_PACKET_IN) ) )
			throw rofl::ePortModBase();

	//Advertised
	if( advertise )
		if( HAL_FAILURE == hal_driver_of1x_set_port_advertise_config(dpid, portno, advertise)  )
			throw rofl::ePortModBase();

	//Port admin down //TODO: evaluate if we can directly call hal_driver_enable_port_by_num instead
	if( mask &  openflow13::OFPPC_PORT_DOWN ){
		if( (config & openflow13::OFPPC_PORT_DOWN)  ){
			//Disable port
			if( HAL_FAILURE == hal_driver_bring_port_down_by_num(dpid, portno) ){
				throw rofl::ePortModBase();
			}
		}else{
			if( HAL_FAILURE == hal_driver_bring_port_up_by_num(dpid, portno) ){
				throw rofl::ePortModBase();
			}
		}
	}
#if 0
	/*
	 * in case of an error, use one of these exceptions:
	 */
	throw rofl::ePortModBadAdvertise();
	throw rofl::ePortModBadConfig();
	throw rofl::ePortModBadHwAddr();
	throw rofl::ePortModBadPort();
#endif
}




void
of13_endpoint::handle_queue_get_config_request(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_queue_get_config_request& pack)
{
	switch_port_snapshot_t* port;
	unsigned int portnum = pack.get_port_no();

	//FIXME: send error? => yes, if portnum is unknown, just throw the appropriate exception
	if (0 /*add check for existence of port*/)
		throw rofl::eBadRequestBadPort();

	of1x_switch_snapshot_t* of13switch = (of1x_switch_snapshot_t*)hal_driver_get_switch_snapshot_by_dpid(sw->dpid);

	if(!of13switch)
		throw rofl::eRofBase();


	rofl::openflow::cofpacket_queues queues(ctl.get_version_negotiated());

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
			queues.set_pqueue(port->of_port_num, port->queues[i].id).set_queue_props().
					add_queue_prop_min_rate().set_min_rate(port->queues[i].min_rate);
			queues.set_pqueue(port->of_port_num, port->queues[i].id).set_queue_props().
					add_queue_prop_max_rate().set_max_rate(port->queues[i].max_rate);
		}
	}

	//Destroy the snapshot
	of_switch_destroy_snapshot((of_switch_snapshot_t*)of13switch);
		
	ctl.send_queue_get_config_reply(auxid, pack.get_xid(), pack.get_port_no(), queues);
}



void
of13_endpoint::handle_experimenter_message(
		rofl::crofctl& ctl,
		const rofl::cauxid& auxid,
		rofl::openflow::cofmsg_experimenter& pack)
{
	// TODO
}



void
of13_endpoint::handle_meter_mod(
		rofl::crofctl& ctl,
		const cauxid& auxid,
		rofl::openflow::cofmsg_meter_mod& msg)
{
	// TODO
}



void
of13_endpoint::handle_ctl_attached(crofctl *ctrl)
{
	std::stringstream sstr; sstr << ctrl->get_peer_addr(rofl::cauxid(0));
	ROFL_INFO("[sw: %s]Controller %s:%u is in CONNECTED state. \n", sw->dpname.c_str() , sstr.str().c_str()); //FIXME: add role
}



void
of13_endpoint::handle_ctl_detached(crofctl *ctrl)
{
	std::stringstream sstr; sstr << ctrl->get_peer_addr(rofl::cauxid(0));
	ROFL_INFO("[sw: %s] Controller %s:%u has DISCONNECTED. \n", sw->dpname.c_str() , sstr.str().c_str()); //FIXME: add role
}

