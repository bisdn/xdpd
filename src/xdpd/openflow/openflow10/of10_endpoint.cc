/*
 * of10_endpoint.cc
 *
 *  Created on: 06.09.2013
 *      Author: andreas
 */

#include "of10_endpoint.h"

#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include "of10_translation_utils.h"

using namespace xdpd;

/*
* Constructor and destructor
*/
of10_endpoint::of10_endpoint(
		openflow_switch* sw,
		int reconnect_start_timeout,
		caddress const& controller_addr,
		caddress const& binding_addr) throw (eOfSmErrorOnCreation) :
	of_endpoint(1 << OFP10_VERSION) {

	//Reference back to the sw
	this->sw = sw;
	of10switch = (of1x_switch_t*)sw->get_fwd_module_sw_ref();


	//FIXME: make controller and binding optional somehow
	//Active connection
	//if(controller_addr.port)
	rpc_connect_to_ctl(OFP10_VERSION, reconnect_start_timeout, controller_addr);

	//Passive connection
	//if(binding_addr.port)
	//rpc_listen_for_ctls(binding_addr);
}

/*
*
* Handling endpoint messages routines
*
*/

void
of10_endpoint::handle_features_request(
		cofctl *ctl,
		cofmsg_features_request *msg)
{
	logical_switch_port_t* ls_port;
	switch_port_t* _port;

	uint32_t num_of_tables 	= 0;
	uint32_t num_of_buffers = 0;
	uint32_t capabilities 	= 0;

	num_of_tables 	= of10switch->pipeline->num_of_tables;
	num_of_buffers 	= of10switch->pipeline->num_of_buffers;
	capabilities 	= of10switch->pipeline->capabilities;

	// array of structures ofp_port
	cofportlist portlist;

	//we check all the positions in case there are empty slots
	for (unsigned int n = 1; n < of10switch->max_ports; n++){

		ls_port = &of10switch->logical_ports[n];
		_port = ls_port->port;

		if(_port!=NULL && ls_port->attachment_state!=LOGICAL_PORT_STATE_DETACHED){

			//Mapping of port state
			assert(n == _port->of_port_num);

			cofport port(ctl->get_version());

			port.set_port_no(_port->of_port_num);
			port.set_hwaddr(cmacaddr(_port->hwaddr, OFP_ETH_ALEN));
			port.set_name(std::string(_port->name));

			uint32_t config = 0;
			if(!_port->up)
				config |= OFP10PC_PORT_DOWN;
			if(_port->drop_received)
				config |= OFP10PC_NO_RECV;
			if(_port->no_flood)
				config |= OFP10PC_NO_FLOOD;
			if(!_port->forward_packets)
				config |= OFP10PC_NO_FWD;
			if(!_port->of_generate_packet_in)
				config |= OFP10PC_NO_PACKET_IN;

			port.set_config(config);
			port.set_state(_port->state);
			port.set_curr(_port->curr);
			port.set_advertised(_port->advertised);
			port.set_supported(_port->supported);
			port.set_peer(_port->peer);

			portlist.next() = port;
		}
 	}

	send_features_reply(
			ctl,
			msg->get_xid(),
			sw->dpid,
			num_of_buffers,	// n_buffers
			num_of_tables,	// n_tables
			capabilities,	// capabilities
			0, //of13_aux_id
			of10_translation_utils::get_supported_actions(of10switch),
			portlist);

	delete msg;
}



void
of10_endpoint::handle_get_config_request(
		cofctl *ctl,
		cofmsg_get_config_request *msg)
{
	uint16_t flags = 0x0;
	uint16_t miss_send_len = 0;

	//FIXME: this should not be made like this!!!

	flags = of10switch->pipeline->capabilities;
	miss_send_len = of10switch->pipeline->miss_send_len;

	send_get_config_reply(ctl, msg->get_xid(), flags, miss_send_len);

	delete msg;
}



void
of10_endpoint::handle_desc_stats_request(
		cofctl *ctl,
		cofmsg_desc_stats_request *msg)
{
	std::string mfr_desc("eXtensible Data Path");
	std::string hw_desc("v0.3.0");
	std::string sw_desc("v0.3.0");
	std::string serial_num("0");
	std::string dp_desc("xDP");

	cofdesc_stats_reply desc_stats(
			ctl->get_version(),
			mfr_desc,
			hw_desc,
			sw_desc,
			serial_num,
			dp_desc);

	send_desc_stats_reply(ctl, msg->get_xid(), desc_stats);

	delete msg;
}



void
of10_endpoint::handle_table_stats_request(
		cofctl *ctl,
		cofmsg_table_stats_request *msg)
{
	unsigned int num_of_tables = of10switch->pipeline->num_of_tables;

	std::vector<coftable_stats_reply> table_stats;

	for (unsigned int n = 0; n < num_of_tables; n++) {

		table_stats.push_back(
				coftable_stats_reply(
					ctl->get_version(),
					of10switch->pipeline->tables[n].number,
					std::string(of10switch->pipeline->tables[n].name, OFP_MAX_TABLE_NAME_LEN),
					(of10switch->pipeline->tables[n].config.match),
					(of10switch->pipeline->tables[n].config.wildcards),
					(of10switch->pipeline->tables[n].config.write_actions),
					(of10switch->pipeline->tables[n].config.apply_actions),
					(of10switch->pipeline->tables[n].config.write_setfields),
					(of10switch->pipeline->tables[n].config.apply_setfields),
					(of10switch->pipeline->tables[n].config.metadata_match),
					(of10switch->pipeline->tables[n].config.metadata_write),
					(of10switch->pipeline->tables[n].config.instructions),
					(of10switch->pipeline->tables[n].config.table_miss_config),
					(of10switch->pipeline->tables[n].max_entries),
					(of10switch->pipeline->tables[n].num_of_entries),
					(of10switch->pipeline->tables[n].stats.lookup_count),
					(of10switch->pipeline->tables[n].stats.matched_count)
				));

		coftable_stats_reply a(
							ctl->get_version(),
							of10switch->pipeline->tables[n].number,
							std::string(of10switch->pipeline->tables[n].name, OFP_MAX_TABLE_NAME_LEN),
							(of10switch->pipeline->tables[n].config.match),
							(of10switch->pipeline->tables[n].config.wildcards),
							(of10switch->pipeline->tables[n].config.write_actions),
							(of10switch->pipeline->tables[n].config.apply_actions),
							(of10switch->pipeline->tables[n].config.write_setfields),
							(of10switch->pipeline->tables[n].config.apply_setfields),
							(of10switch->pipeline->tables[n].config.metadata_match),
							(of10switch->pipeline->tables[n].config.metadata_write),
							(of10switch->pipeline->tables[n].config.instructions),
							(of10switch->pipeline->tables[n].config.table_miss_config),
							(of10switch->pipeline->tables[n].max_entries),
							(of10switch->pipeline->tables[n].num_of_entries),
							(of10switch->pipeline->tables[n].stats.lookup_count),
							(of10switch->pipeline->tables[n].stats.matched_count)
						);
	}


	send_table_stats_reply(ctl, msg->get_xid(), table_stats, false);

	delete msg;
}



void
of10_endpoint::handle_port_stats_request(
		cofctl *ctl,
		cofmsg_port_stats_request *msg)
{

	switch_port_t* port;
	uint32_t port_no = msg->get_port_stats().get_portno();

	std::vector<cofport_stats_reply> port_stats;

	/*
	 *  send statistics for all ports
	 */
	if (OFPP10_ALL == port_no || OFPP10_NONE == port_no){

		//we check all the positions in case there are empty slots
		for (unsigned int n = 1; n < of10switch->max_ports; n++){

			port = of10switch->logical_ports[n].port;

			if((port != NULL) && (of10switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED)){

				port_stats.push_back(
						cofport_stats_reply(
								ctl->get_version(),
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
								port->stats.collisions));
			}
	 	}

	}else if(port_no < of10switch->max_ports){
		/*
		 * send statistics for only one port
		 */

		// search for the port with the specified port-number
		//we check all the positions in case there are empty slots
		port = of10switch->logical_ports[port_no].port;
		if( 	(port != NULL) &&
			(of10switch->logical_ports[port_no].attachment_state == LOGICAL_PORT_STATE_ATTACHED) &&
			(port->of_port_num == port_no)
		){
			//Mapping of port state
			port_stats.push_back(
					cofport_stats_reply(
							ctl->get_version(),
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
							port->stats.collisions));

		}

		// if port_no was not found, body.memlen() is 0
	}else{
		//Unknown port
		ROFL_ERR("Got a port stats request for an unknown port: %u. Ignoring...\n",port_no);
	}

	send_port_stats_reply(ctl, msg->get_xid(), port_stats, false);

	delete msg;
}



void
of10_endpoint::handle_flow_stats_request(
		cofctl *ctl,
		cofmsg_flow_stats_request *msg)
{
	of1x_stats_flow_msg_t* fp_msg = NULL;
	of1x_flow_entry_t* entry = NULL;

	//Map the match structure from OpenFlow to of1x_packet_matches_t
	entry = of1x_init_flow_entry(NULL, NULL, false);

	try{
		of10_translation_utils::of10_map_flow_entry_matches(ctl, msg->get_flow_stats().get_match(), sw, entry);
	}catch(...){
		of1x_destroy_flow_entry(entry);
		throw eBadRequestBadStat();
	}

	//Ask the Forwarding Plane to process stats
	fp_msg = fwd_module_of1x_get_flow_stats(sw->dpid,
			msg->get_flow_stats().get_table_id(),
			0,
			0,
			of10_translation_utils::get_out_port(msg->get_flow_stats().get_out_port()),
			OF1X_GROUP_ANY,
			&entry->matches);

	if(!fp_msg){
		of1x_destroy_flow_entry(entry);
		throw eBadRequestBadStat();
	}

	//Construct OF message
	of1x_stats_single_flow_msg_t *elem = fp_msg->flows_head;

	std::vector<cofflow_stats_reply> flow_stats;

	for(elem = fp_msg->flows_head; elem; elem = elem->next){

		cofmatch match(OFP10_VERSION);
		of10_translation_utils::of1x_map_reverse_flow_entry_matches(elem->matches, match);

		cofinlist instructions(ctl->get_version());
		of10_translation_utils::of1x_map_reverse_flow_entry_instructions((of1x_instruction_group_t*)(elem->inst_grp), instructions, of10switch->pipeline->miss_send_len);

		if (0 == instructions.size())
			continue;

		flow_stats.push_back(
				cofflow_stats_reply(
						ctl->get_version(),
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
						instructions[0].actions));
	}


	try{
		//Send message
		send_flow_stats_reply(ctl, msg->get_xid(), flow_stats);
	}catch(...){
		of1x_destroy_stats_flow_msg(fp_msg);
		of1x_destroy_flow_entry(entry);
		throw;
	}
	//Destroy FP stats
	of1x_destroy_stats_flow_msg(fp_msg);
	of1x_destroy_flow_entry(entry);

	delete msg;
}



void
of10_endpoint::handle_aggregate_stats_request(
		cofctl *ctl,
		cofmsg_aggr_stats_request *msg)
{
	of1x_stats_flow_aggregate_msg_t* fp_msg;
	of1x_flow_entry_t* entry;

//	cmemory body(sizeof(struct ofp_flow_stats));
//	struct ofp_flow_stats *flow_stats = (struct ofp_flow_stats*)body.somem();

	//Map the match structure from OpenFlow to of1x_packet_matches_t
	entry = of1x_init_flow_entry(NULL, NULL, false);

	if(!entry)
		throw eBadRequestBadStat();

	try{
		of10_translation_utils::of10_map_flow_entry_matches(ctl, msg->get_aggr_stats().get_match(), sw, entry);
	}catch(...){
		of1x_destroy_flow_entry(entry);
		throw eBadRequestBadStat();
	}

	//TODO check error while mapping

	//Ask the Forwarding Plane to process stats
	fp_msg = fwd_module_of1x_get_flow_aggregate_stats(sw->dpid,
					msg->get_aggr_stats().get_table_id(),
					0,
					0,
					of10_translation_utils::get_out_port(msg->get_aggr_stats().get_out_port()),
					OF1X_GROUP_ANY,
					&entry->matches);

	if(!fp_msg){
		of1x_destroy_flow_entry(entry);
		throw eBadRequestBadStat();
	}

	try{
		//Construct OF message
		send_aggr_stats_reply(
				ctl,
				msg->get_xid(),
				cofaggr_stats_reply(
					ctl->get_version(),
					fp_msg->packet_count,
					fp_msg->byte_count,
					fp_msg->flow_count),
				false);
	}catch(...){
		of1x_destroy_stats_flow_aggregate_msg(fp_msg);
		of1x_destroy_flow_entry(entry);
		throw;
	}

	//Destroy FP stats
	of1x_destroy_stats_flow_aggregate_msg(fp_msg);
	of1x_destroy_flow_entry(entry);

	delete msg;
}



void
of10_endpoint::handle_queue_stats_request(
		cofctl *ctl,
		cofmsg_queue_stats_request *pack)
{

	switch_port_t* port = NULL;
	unsigned int portnum = pack->get_queue_stats().get_port_no();
	unsigned int queue_id = pack->get_queue_stats().get_queue_id();

	if( ((portnum >= of10switch->max_ports) && (portnum != OFPP10_ALL)) || portnum == 0){
		throw eBadRequestBadPort(); 	//Invalid port num
	}

	std::vector<cofqueue_stats_reply> stats;

	/*
	* port num
	*/

	//we check all the positions in case there are empty slots
	for (unsigned int n = 1; n < of10switch->max_ports; n++){

		port = of10switch->logical_ports[n].port;

		if ( port == NULL || ( (OFPP10_ALL != portnum) && (port->of_port_num != portnum) ) )
			continue;


		if( of10switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED /* && (port->of_port_num == portnum)*/){

			if (OFPQ_ALL == queue_id){

				// TODO: iterate over all queues

				for(unsigned int i=0; i<port->max_queues; i++){
					if(!port->queues[i].set)
						continue;

					//Set values
					stats.push_back(
							cofqueue_stats_reply(
									ctl->get_version(),
									port->of_port_num,
									i,
									port->queues[i].stats.tx_bytes,
									port->queues[i].stats.tx_packets,
									port->queues[i].stats.overrun));
				}

			} else {

				if(queue_id >= port->max_queues)
					throw eBadRequestBadPort(); 	//FIXME send a BadQueueId error


				//Check if the queue is really in use
				if(port->queues[queue_id].set){
					//Set values
					stats.push_back(
							cofqueue_stats_reply(
									ctl->get_version(),
									portnum,
									queue_id,
									port->queues[queue_id].stats.tx_bytes,
									port->queues[queue_id].stats.tx_packets,
									port->queues[queue_id].stats.overrun));

				}
			}
		}
	}


	send_queue_stats_reply(
			ctl,
			pack->get_xid(),
			stats,
			false);

	delete pack;
}





void
of10_endpoint::handle_experimenter_stats_request(
		cofctl *ctl,
		cofmsg_stats_request *pack)
{

	//TODO: when exp are supported

	delete pack;
}



void
of10_endpoint::handle_packet_out(
		cofctl *ctl,
		cofmsg_packet_out *msg)
{
	of1x_action_group_t* action_group = of1x_init_action_group(NULL);

	try{
		of10_translation_utils::of1x_map_flow_entry_actions(ctl, sw, msg->get_actions(), action_group, NULL); //TODO: is this OK always NULL?
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
							msg->get_buffer_id(),
							msg->get_in_port(),
							action_group,
							msg->get_packet().soframe(), msg->get_packet().framelen())){
		// log error
		//FIXME: send error
	}

	of1x_destroy_action_group(action_group);

	delete msg;
}







afa_result_t
of10_endpoint::process_packet_in(
		uint8_t table_id,
		uint8_t reason,
		uint32_t in_port,
		uint32_t buffer_id,
		uint8_t* pkt_buffer,
		uint32_t buf_len,
		uint16_t total_len,
		of1x_packet_matches_t matches)
{
	try {
		//Transform matches
		cofmatch match(OFP10_VERSION);
		of10_translation_utils::of1x_map_reverse_packet_matches(&matches, match);


		send_packet_in_message(
				NULL,
				buffer_id,
				total_len,
				reason,
				table_id,
				/*cookie=*/0,
				in_port, // OF1.0 only
				match,
				pkt_buffer, buf_len);

		return AFA_SUCCESS;

	} catch (eRofBaseNotConnected& e) {

		return AFA_FAILURE;

	} catch (...) {

	}

	return AFA_FAILURE;
}

/*
* Port async notifications processing
*/

afa_result_t of10_endpoint::notify_port_add(switch_port_t* port){

	uint32_t config=0x0;

	//Compose port config
	if(!port->up) config |= OFP10PC_PORT_DOWN;
	if(!port->of_generate_packet_in) config |= OFP10PC_NO_PACKET_IN;
	if(!port->forward_packets) config |= OFP10PC_NO_FWD;
	if(port->drop_received) config |= OFP10PC_NO_RECV;


	cofport ofport(OFP10_VERSION);
	ofport.set_port_no(port->of_port_num);
	ofport.set_hwaddr(cmacaddr(port->hwaddr, OFP_ETH_ALEN));
	ofport.set_name(std::string(port->name));
	ofport.set_config(config);
	ofport.set_state(port->state&0x1); //Only first bit is relevant
	ofport.set_curr(port->curr);
	ofport.set_advertised(port->advertised);
	ofport.set_supported(port->supported);
	ofport.set_peer(port->peer);
	//ofport.set_curr_speed(of10_translation_utils::get_port_speed_kb(port->curr_speed));
	//ofport.set_max_speed(of10_translation_utils::get_port_speed_kb(port->curr_max_speed));

	//Send message
	send_port_status_message(NULL, OFPPR_ADD, ofport);

	return AFA_SUCCESS;
}

afa_result_t of10_endpoint::notify_port_delete(switch_port_t* port){

	uint32_t config=0x0;

	//Compose port config
	if(!port->up) config |= OFP10PC_PORT_DOWN;
	if(!port->of_generate_packet_in) config |= OFP10PC_NO_PACKET_IN;
	if(!port->forward_packets) config |= OFP10PC_NO_FWD;
	if(port->drop_received) config |= OFP10PC_NO_RECV;

	cofport ofport(OFP10_VERSION);
	ofport.set_port_no(port->of_port_num);
	ofport.set_hwaddr(cmacaddr(port->hwaddr, OFP_ETH_ALEN));
	ofport.set_name(std::string(port->name));
	ofport.set_config(config);
	ofport.set_state(port->state&0x1); //Only first bit is relevant
	ofport.set_curr(port->curr);
	ofport.set_advertised(port->advertised);
	ofport.set_supported(port->supported);
	ofport.set_peer(port->peer);
	//ofport.set_curr_speed(of10_translation_utils::get_port_speed_kb(port->curr_speed));
	//ofport.set_max_speed(of10_translation_utils::get_port_speed_kb(port->curr_max_speed));

	//Send message
	send_port_status_message(NULL, OFPPR_DELETE, ofport);

	return AFA_SUCCESS;
}

afa_result_t of10_endpoint::notify_port_status_changed(switch_port_t* port){

	uint32_t config=0x0;

	//Compose port config
	if(!port->up) config |= OFP10PC_PORT_DOWN;
	if(!port->of_generate_packet_in) config |= OFP10PC_NO_PACKET_IN;
	if(!port->forward_packets) config |= OFP10PC_NO_FWD;
	if(port->drop_received) config |= OFP10PC_NO_RECV;

	//Notify OF controller
	cofport ofport(OFP10_VERSION);
	ofport.set_port_no(port->of_port_num);
	ofport.set_hwaddr(cmacaddr(port->hwaddr, OFP_ETH_ALEN));
	ofport.set_name(std::string(port->name));
	ofport.set_config(config);
	ofport.set_state(port->state&0x1); //Only first bit is relevant
	ofport.set_curr(port->curr);
	ofport.set_advertised(port->advertised);
	ofport.set_supported(port->supported);
	ofport.set_peer(port->peer);
	//ofport.set_curr_speed(of10_translation_utils::get_port_speed_kb(port->curr_speed));
	//ofport.set_max_speed(of10_translation_utils::get_port_speed_kb(port->curr_max_speed));

	//Send message
	send_port_status_message(NULL, OFPPR_MODIFY, ofport);

	return AFA_SUCCESS; // ignore this notification
}





void
of10_endpoint::handle_barrier_request(
		cofctl *ctl,
		cofmsg_barrier_request *pack)
{

	//Since we are not queuing messages currently
	send_barrier_reply(ctl, pack->get_xid());

	delete pack;
}



void
of10_endpoint::handle_flow_mod(
		cofctl *ctl,
		cofmsg_flow_mod *msg)
{
	switch (msg->get_command()) {
		case OFPFC_ADD: {
				flow_mod_add(ctl, msg);
			} break;

		case OFPFC_MODIFY: {
				flow_mod_modify(ctl, msg, false);
			} break;

		case OFPFC_MODIFY_STRICT: {
				flow_mod_modify(ctl, msg, true);
			} break;

		case OFPFC_DELETE: {
				flow_mod_delete(ctl, msg, false);
			} break;

		case OFPFC_DELETE_STRICT: {
				flow_mod_delete(ctl, msg, true);
			} break;

		default:
			throw eFlowModBadCommand();
	}
	delete msg;
}



void
of10_endpoint::flow_mod_add(
		cofctl *ctl,
		cofmsg_flow_mod *msg) //throw (eOfSmPipelineBadTableId, eOfSmPipelineTableFull)
{
	uint8_t table_id = msg->get_table_id();
	afa_result_t res;
	of1x_flow_entry_t *entry=NULL;

	// sanity check: table for table-id must exist
	if ( (table_id > of10switch->pipeline->num_of_tables) && (table_id != OFPTT_ALL) )
	{
		ROFL_DEBUG("of10_endpoint(%s)::flow_mod_add() "
				"invalid table-id:%d in flow-mod command",
				sw->dpname.c_str(), msg->get_table_id());

		throw eFlowModBadTableId();
	}

	try{
		entry = of10_translation_utils::of1x_map_flow_entry(ctl, msg,sw);
	}catch(...){
		ROFL_DEBUG("of10_endpoint(%s)::flow_mod_add() "
				"unable to create flow-entry", sw->dpname.c_str());
		throw eFlowModUnknown();
	}

	if(!entry)
		throw eFlowModUnknown();//Just for safety, but shall never reach this

	if (AFA_SUCCESS != (res = fwd_module_of1x_process_flow_mod_add(sw->dpid,
								msg->get_table_id(),
								entry,
								msg->get_buffer_id(),
								msg->get_flags() & OFPFF_CHECK_OVERLAP,
								msg->get_flags() & OFPFF_RESET_COUNTS))){
		// log error
		ROFL_DEBUG("Error inserting the flowmod\n");
		of1x_destroy_flow_entry(entry);

		if(res == AFA_FM_OVERLAP_FAILURE)
			throw eFlowModOverlap();
		else
			throw eFlowModTableFull();
	}

}



void
of10_endpoint::flow_mod_modify(
		cofctl *ctl,
		cofmsg_flow_mod *pack,
		bool strict)
{
	of1x_flow_entry_t *entry=NULL;

	// sanity check: table for table-id must exist
	if (pack->get_table_id() > of10switch->pipeline->num_of_tables)
	{
		ROFL_DEBUG("of10_endpoint(%s)::flow_mod_delete() "
				"invalid table-id:%d in flow-mod command",
				sw->dpname.c_str(), pack->get_table_id());

		throw eFlowModBadTableId();
	}

	try{
		entry = of10_translation_utils::of1x_map_flow_entry(ctl, pack, sw);
	}catch(...){
		ROFL_DEBUG("of10_endpoint(%s)::flow_mod_modify() "
				"unable to attempt to modify flow-entry", sw->dpname.c_str());
		throw eFlowModUnknown();
	}

	if(!entry)
		throw eFlowModUnknown();//Just for safety, but shall never reach this


	of1x_flow_removal_strictness_t strictness = (strict) ? STRICT : NOT_STRICT;


	if(AFA_SUCCESS != fwd_module_of1x_process_flow_mod_modify(sw->dpid,
								pack->get_table_id(),
								entry,
								pack->get_buffer_id(),
								strictness,
								pack->get_flags() & OFPFF_RESET_COUNTS)){
		ROFL_DEBUG("Error modiying flowmod\n");
		of1x_destroy_flow_entry(entry);
		throw eFlowModBase();
	}

}



void
of10_endpoint::flow_mod_delete(
		cofctl *ctl,
		cofmsg_flow_mod *pack,
		bool strict) //throw (eOfSmPipelineBadTableId)
{

	of1x_flow_entry_t *entry=NULL;

	try{
		entry = of10_translation_utils::of1x_map_flow_entry(ctl, pack, sw);
	}catch(...){
		ROFL_DEBUG("of10_endpoint(%s)::flow_mod_delete() "
				"unable to attempt to remove flow-entry", sw->dpname.c_str());
		throw eFlowModUnknown();
	}

	if(!entry)
		throw eFlowModUnknown();//Just for safety, but shall never reach this


	of1x_flow_removal_strictness_t strictness = (strict) ? STRICT : NOT_STRICT;

	if(AFA_SUCCESS != fwd_module_of1x_process_flow_mod_delete(sw->dpid,
								pack->get_table_id(),
								entry,
								of10_translation_utils::get_out_port(pack->get_out_port()),
								OF1X_GROUP_ANY,
								strictness)) {
		ROFL_DEBUG("Error deleting flowmod\n");
		of1x_destroy_flow_entry(entry);
		throw eFlowModBase();
	}

	//Always delete entry
	of1x_destroy_flow_entry(entry);

}






afa_result_t
of10_endpoint::process_flow_removed(
		uint8_t reason,
		of1x_flow_entry *entry)
{
	cofmatch match(OFP10_VERSION);
	uint32_t sec,nsec;

	of10_translation_utils::of1x_map_reverse_flow_entry_matches(entry->matches.head, match);

	//get duration of the flow mod
	of1x_stats_flow_get_duration(entry, &sec, &nsec);


	send_flow_removed_message(
			NULL, //broadcast to all controllers
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

	return AFA_SUCCESS;
}





void
of10_endpoint::handle_table_mod(
		cofctl *ctl,
		cofmsg_table_mod *msg)
{

	/*
	 * the parameters defined in the pipeline OF1X_TABLE_...
	 * match those defined by the OF1.2 specification.
	 * This may change in the future for other versions, so map
	 * the OF official numbers to the ones used by the pipeline.
	 *
	 * at least we map network byte order to host byte order here ...
	 */
	of1x_flow_table_miss_config_t config = OF1X_TABLE_MISS_CONTROLLER; //Default

	if (msg->get_config() == OFPTC_TABLE_MISS_CONTINUE){
		config = OF1X_TABLE_MISS_CONTINUE;
	}else if (msg->get_config() == OFPTC_TABLE_MISS_CONTROLLER){
		config = OF1X_TABLE_MISS_CONTROLLER;
	}else if (msg->get_config() == OFPTC_TABLE_MISS_DROP){
		config = OF1X_TABLE_MISS_DROP;
	}

	if( AFA_FAILURE == fwd_module_of1x_set_table_config(sw->dpid, msg->get_table_id(), config) ){
		//TODO: treat exception
	}

	delete msg;
}



void
of10_endpoint::handle_port_mod(
		cofctl *ctl,
		cofmsg_port_mod *msg)
{
	uint32_t config, mask, advertise;
	uint16_t port_num;

	config 		= msg->get_config();
	mask 		= msg->get_mask();
	advertise 	= msg->get_advertise();
	port_num 	= (uint16_t)msg->get_port_no();

	//Check if port_num FLOOD
	//TODO: Inspect if this is right. Spec does not clearly define if this should be supported or not
	if( (port_num != OFPP10_ALL) && (port_num > OFPP10_MAX) )
		throw ePortModBadPort();

	// check for existence of port with id port_num
	switch_port_t* port = (switch_port_t*)0;
	bool port_found = false;
	for(unsigned int n = 1; n < of10switch->max_ports; n++){
		port = of10switch->logical_ports[n].port;
		if ((0 != port) && (port->of_port_num == (uint32_t)port_num)) {
			port_found = true;
			break;
		}
	}
	if (not port_found)
		throw eBadRequestBadPort();


	//Drop received
	if( mask &  OFP10PC_NO_RECV )
		if( AFA_FAILURE == fwd_module_of1x_set_port_drop_received_config(sw->dpid, port_num, config & OFP10PC_NO_RECV ) )
			throw ePortModBase();
	//No forward
	if( mask &  OFP10PC_NO_FWD )
		if( AFA_FAILURE == fwd_module_of1x_set_port_forward_config(sw->dpid, port_num, !(config & OFP10PC_NO_FWD) ) )
			throw ePortModBase();

	//No flood
	if( mask &  OFP10PC_NO_FLOOD )
	{
		if( AFA_FAILURE == fwd_module_of1x_set_port_no_flood_config(sw->dpid, port_num, config & OFP10PC_NO_FLOOD ) )
			throw ePortModBase();
	}

	//No packet in
	if( mask &  OFP10PC_NO_PACKET_IN )
		if( AFA_FAILURE == fwd_module_of1x_set_port_generate_packet_in_config(sw->dpid, port_num, !(config & OFP10PC_NO_PACKET_IN) ) )
			throw ePortModBase();

	//Advertised
	if( advertise )
		if( AFA_FAILURE == fwd_module_of1x_set_port_advertise_config(sw->dpid, port_num, advertise)  )
			throw ePortModBase();

	//Port admin down //TODO: evaluate if we can directly call fwd_module_enable_port_by_num instead
	if( mask &  OFP10PC_PORT_DOWN ){
		if( (config & OFP10PC_PORT_DOWN)  ){
			//Disable port
			if( AFA_FAILURE == fwd_module_disable_port_by_num(sw->dpid, port_num) ){
				throw ePortModBase();
			}
		}else{
			if( AFA_FAILURE == fwd_module_enable_port_by_num(sw->dpid, port_num) ){
				throw ePortModBase();
			}
		}
	}


	delete msg;

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
of10_endpoint::handle_set_config(
		cofctl *ctl,
		cofmsg_set_config *msg)
{

	//Instruct the driver to process the set config
	if(AFA_FAILURE == fwd_module_of1x_set_pipeline_config(sw->dpid, msg->get_flags(), msg->get_miss_send_len())){
		throw eTableModBadConfig();
	}

	delete msg;
}



void
of10_endpoint::handle_queue_get_config_request(
		cofctl *ctl,
		cofmsg_queue_get_config_request *pack)
{
	switch_port_t* port;
	unsigned int portnum = pack->get_port_no();

	//FIXME: send error? => yes, if portnum is unknown, just throw the appropriate exception
	if (0 /*add check for existence of port*/)
		throw eBadRequestBadPort();


	cofpacket_queue_list pql(ctl->get_version());

	//we check all the positions in case there are empty slots
	for(unsigned int n = 1; n < of10switch->max_ports; n++){

		port = of10switch->logical_ports[n].port;

		if(port == NULL)
			continue;

		if (of10switch->logical_ports[n].attachment_state != LOGICAL_PORT_STATE_ATTACHED)
			continue;

		if ((OFPP10_ALL != portnum) && (port->of_port_num != portnum))
			continue;

		for(unsigned int i=0; i<port->max_queues; i++){
			if(!port->queues[i].set)
				continue;

			cofpacket_queue pq(ctl->get_version());
			pq.set_queue_id(port->queues[i].id);
			pq.set_port(port->of_port_num);
			pq.get_queue_prop_list().next() = cofqueue_prop_min_rate(ctl->get_version(), port->queues[i].min_rate);
			pq.get_queue_prop_list().next() = cofqueue_prop_max_rate(ctl->get_version(), port->queues[i].max_rate);
			//fprintf(stderr, "min_rate: %d\n", port->queues[i].min_rate);
			//fprintf(stderr, "max_rate: %d\n", port->queues[i].max_rate);

			pql.next() = pq;
		}
	}

	//Send reply
	send_queue_get_config_reply(
			ctl,
			pack->get_xid(),
			pack->get_port_no(),
			pql);

	// do not forget to remove pack from heap
	delete pack;
}



void
of10_endpoint::handle_experimenter_message(
		cofctl *ctl,
		cofmsg_features_request *pack)
{
	// TODO

	delete pack;
}



void
of10_endpoint::handle_ctrl_open(cofctl *ctrl)
{
	ROFL_INFO("[sw: %s]Controller %s:%u is in CONNECTED state. \n", sw->dpname.c_str() , ctrl->get_peer_addr().c_str()); //FIXME: add role
}



void
of10_endpoint::handle_ctrl_close(cofctl *ctrl)
{
	ROFL_INFO("[sw: %s] Controller %s:%u has DISCONNECTED. \n", sw->dpname.c_str() ,ctrl->get_peer_addr().c_str()); //FIXME: add role

}
