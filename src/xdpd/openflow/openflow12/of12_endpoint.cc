#include "of12_endpoint.h"

#include <rofl/datapath/afa/fwd_module.h>
#include "of12_translation_utils.h"

using namespace rofl;

/*
* Constructor and destructor
*/
of12_endpoint::of12_endpoint(openflow_switch* sw, caddress const& controller_addr, caddress const& binding_addr) throw (eOfSmErrorOnCreation){

	//Reference back to the sw
	this->sw = sw;
	of12switch = (of12_switch_t*)sw->get_fwd_module_sw_ref();


	//FIXME: make controller and binding optional somehow
	//Active connection
	//if(controller_addr.port)
	rpc_connect_to_ctl(controller_addr);

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
of12_endpoint::handle_features_request(
		cofctl *ctl,
		cofmsg_features_request *msg)
{
	logical_switch_port_t* ls_port;	
	switch_port_t* _port;	
	
	uint32_t num_of_tables 	= 0;
	uint32_t num_of_buffers = 0;
	uint32_t capabilities 	= 0;

	num_of_tables 	= of12switch->pipeline->num_of_tables;
	num_of_buffers 	= of12switch->pipeline->num_of_buffers;
	capabilities 	= of12switch->pipeline->capabilities;

	// array of structures ofp_port
	cofportlist portlist;

	//we check all the positions in case there are empty slots
	for (unsigned int n = 1; n < of12switch->max_ports; n++){

		ls_port = &of12switch->logical_ports[n];
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
				config |= OFPPC_PORT_DOWN;
			if(_port->drop_received)
				config |= OFPPC_NO_RECV;
			if(!_port->forward_packets)	
				config |= OFPPC_NO_FWD;
			if(!_port->of_generate_packet_in)
				config |= OFPPC_NO_PACKET_IN;

			port.set_config(config);
			port.set_state(_port->state);
			port.set_curr(_port->curr);
			port.set_advertised(_port->advertised);
			port.set_supported(_port->supported);
			port.set_peer(_port->peer);
			port.set_curr_speed(of12_translation_utils::get_port_speed_kb(_port->curr_speed));
			port.set_max_speed(of12_translation_utils::get_port_speed_kb(_port->curr_max_speed));

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
			0,
			0,
			portlist);

	delete msg;
}



void
of12_endpoint::handle_get_config_request(
		cofctl *ctl,
		cofmsg_get_config_request *msg)
{
	uint16_t flags = 0x0;
	uint16_t miss_send_len = 0;

	//FIXME: this should not be made like this!!!
	
	flags = of12switch->pipeline->capabilities;
	miss_send_len = of12switch->pipeline->miss_send_len;

	send_get_config_reply(ctl, msg->get_xid(), flags, miss_send_len);

	delete msg;
}



void
of12_endpoint::handle_desc_stats_request(
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
of12_endpoint::handle_table_stats_request(
		cofctl *ctl,
		cofmsg_table_stats_request *msg)
{
	unsigned int num_of_tables = of12switch->pipeline->num_of_tables;

	std::vector<coftable_stats_reply> table_stats;

	for (unsigned int n = 0; n < num_of_tables; n++) {

		table_stats.push_back(
				coftable_stats_reply(
					ctl->get_version(),
					of12switch->pipeline->tables[n].number,
					std::string(of12switch->pipeline->tables[n].name, OFP_MAX_TABLE_NAME_LEN),
					(of12switch->pipeline->tables[n].config.match),
					(of12switch->pipeline->tables[n].config.wildcards),
					(of12switch->pipeline->tables[n].config.write_actions),
					(of12switch->pipeline->tables[n].config.apply_actions),
					(of12switch->pipeline->tables[n].config.write_setfields),
					(of12switch->pipeline->tables[n].config.apply_setfields),
					(of12switch->pipeline->tables[n].config.metadata_match),
					(of12switch->pipeline->tables[n].config.metadata_write),
					(of12switch->pipeline->tables[n].config.instructions),
					(of12switch->pipeline->tables[n].config.table_miss_config),
					(of12switch->pipeline->tables[n].max_entries),
					(of12switch->pipeline->tables[n].num_of_entries),
					(of12switch->pipeline->tables[n].stats.lookup_count),
					(of12switch->pipeline->tables[n].stats.matched_count)
				));

		coftable_stats_reply a(
							ctl->get_version(),
							of12switch->pipeline->tables[n].number,
							std::string(of12switch->pipeline->tables[n].name, OFP_MAX_TABLE_NAME_LEN),
							(of12switch->pipeline->tables[n].config.match),
							(of12switch->pipeline->tables[n].config.wildcards),
							(of12switch->pipeline->tables[n].config.write_actions),
							(of12switch->pipeline->tables[n].config.apply_actions),
							(of12switch->pipeline->tables[n].config.write_setfields),
							(of12switch->pipeline->tables[n].config.apply_setfields),
							(of12switch->pipeline->tables[n].config.metadata_match),
							(of12switch->pipeline->tables[n].config.metadata_write),
							(of12switch->pipeline->tables[n].config.instructions),
							(of12switch->pipeline->tables[n].config.table_miss_config),
							(of12switch->pipeline->tables[n].max_entries),
							(of12switch->pipeline->tables[n].num_of_entries),
							(of12switch->pipeline->tables[n].stats.lookup_count),
							(of12switch->pipeline->tables[n].stats.matched_count)
						);
	}


	send_table_stats_reply(ctl, msg->get_xid(), table_stats, false);

	delete msg;
}



void
of12_endpoint::handle_port_stats_request(
		cofctl *ctl,
		cofmsg_port_stats_request *msg)
{

	switch_port_t* port;
	uint32_t port_no = msg->get_port_stats().get_portno();

	std::vector<cofport_stats_reply> port_stats;

	/*
	 *  send statistics for all ports
	 */
	if (OFPP_ANY == port_no){

		//we check all the positions in case there are empty slots
		for (unsigned int n = 1; n < of12switch->max_ports; n++){
	
			port = of12switch->logical_ports[n].port; 
	
			if((port != NULL) && (of12switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED)){

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

	}else{
		/*
		 * send statistics for only one port
		 */
		
		// search for the port with the specified port-number
		//we check all the positions in case there are empty slots
		for (unsigned int n = 1; n < of12switch->max_ports; n++){
			
			port = of12switch->logical_ports[n].port; 

			if( 	(port != NULL) &&
				(of12switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED) &&
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

				break;
			}
	 	}

		// if port_no was not found, body.memlen() is 0
	}

	send_port_stats_reply(ctl, msg->get_xid(), port_stats, false);

	delete msg;
}



void
of12_endpoint::handle_flow_stats_request(
		cofctl *ctl,
		cofmsg_flow_stats_request *msg)
{
	of12_stats_flow_msg_t* fp_msg = NULL;
	of12_flow_entry_t* entry = NULL;

	//Map the match structure from OpenFlow to of12_packet_matches_t
	entry = of12_init_flow_entry(NULL, NULL, false);
	of12_translation_utils::of12_map_flow_entry_matches(ctl, msg->get_flow_stats().get_match(), sw, entry);

	//TODO check error while mapping 

	//Ask the Forwarding Plane to process stats
	fp_msg = fwd_module_of12_get_flow_stats(sw->dpid,
			msg->get_flow_stats().get_table_id(),
			msg->get_flow_stats().get_cookie(),
			msg->get_flow_stats().get_cookie_mask(),
			msg->get_flow_stats().get_out_port(),
			msg->get_flow_stats().get_out_group(),
					entry->matchs);
	
	if(!fp_msg){
		//FIXME throw exception
		return;
	}

	//Construct OF message
	of12_stats_single_flow_msg_t *elem = fp_msg->flows_head;

	std::vector<cofflow_stats_reply> flow_stats;

	for(elem = fp_msg->flows_head; elem; elem = elem->next){

		cofmatch match;
		of12_translation_utils::of12_map_reverse_flow_entry_matches(elem->matches, match);

		cofinlist instructions;
		of12_translation_utils::of12_map_reverse_flow_entry_instructions((of12_instruction_group_t*)(elem->inst_grp), instructions);


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
						instructions));
	}

	//Send message
	send_flow_stats_reply(ctl, msg->get_xid(), flow_stats);

	//Destroy FP stats
	of12_destroy_stats_flow_msg(fp_msg);	
	of12_destroy_flow_entry(entry);	

	delete msg;
}



void
of12_endpoint::handle_aggregate_stats_request(
		cofctl *ctl,
		cofmsg_aggr_stats_request *msg)
{
	of12_stats_flow_aggregate_msg_t* fp_msg;
	of12_flow_entry_t* entry;

//	cmemory body(sizeof(struct ofp_flow_stats));
//	struct ofp_flow_stats *flow_stats = (struct ofp_flow_stats*)body.somem();

	//Map the match structure from OpenFlow to of12_packet_matches_t
	entry = of12_init_flow_entry(NULL, NULL, false);
	of12_translation_utils::of12_map_flow_entry_matches(ctl, msg->get_aggr_stats().get_match(), sw, entry);

	//TODO check error while mapping 

	//Ask the Forwarding Plane to process stats
	fp_msg = fwd_module_of12_get_flow_aggregate_stats(sw->dpid,
					msg->get_aggr_stats().get_table_id(),
					msg->get_aggr_stats().get_cookie(),
					msg->get_aggr_stats().get_cookie_mask(),
					msg->get_aggr_stats().get_out_port(),
					msg->get_aggr_stats().get_out_group(),
					entry->matchs);
	
	if(!fp_msg){
		//FIXME throw exception
		return;
	}

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

	//Destroy FP stats
	of12_destroy_stats_flow_aggregate_msg(fp_msg);	

	delete msg;
}



void
of12_endpoint::handle_queue_stats_request(
		cofctl *ctl,
		cofmsg_queue_stats_request *pack)
{

#if 0
	switch_port_t* port;
	unsigned int portnum = 0;//pack->get_port_no(); 
	unsigned int queue_id = 0;//pack->get_queue_id(); 

	cofmsg_queue_stats_reply reply(OFP12_VERSION); //, portnum, queue_id, 0, 0, 0);

	if( (portnum >= of12switch->max_ports && port_num != OFPP_ANY) || portnum == 0){
		//Invalid port num
		assert(0);
	
		delete pack;
		return;
	}

	/*
	* port num
	*/
	if (OFPP_ANY == port_no){
		//All ports
		unsigned int tx_bytes = 0;
		unsigned int tx_packets = 0;
		unsigned int tx_errors = 0;

		//we check all the positions in case there are empty slots
		for (unsigned int n = 1; n < of12switch->max_ports; n++){
	
			port = of12switch->logical_ports[n].port; 

			if((port != NULL) && (of12switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED) && (port->of_port_num == portnum)){

				//Check if the queue is really in use
				if(port->queues[queue_id].use){
					//Set values
					tx_bytes += port->queues[queue_id].stats.tx_bytes;
					tx_packets += port->queues[queue_id].stats.tx_packets;
					tx_errors += port->queues[queue_id].stats.tx_errors;
				}
			}
		}

		//Set values
		reply.set_tx_bytes(tx_bytes);
		reply.set_tx_packets(tx_packets);
		reply.set_tx_errors(tx_errors);

		//Send reply
		//send_queue_get_config_reply(ctl, reply);

		delete pack;
		return;

	}else{
		//Single port
		port = of12switch->logical_ports[n].port; 

		if((port != NULL) && (of12switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED) && (port->of_port_num == portnum)){

			//Check if the queue is really in use
			if(port->queues[queue_id].use){
				//Set values
				reply.set_tx_bytes(port->queues[queue_id].stats.tx_bytes);
				reply.set_tx_packets(port->queues[queue_id].stats.tx_packets);
				reply.set_tx_errors(port->queues[queue_id].stats.tx_errors);

				//Send reply
				//send_queue_get_config_reply(ctl, reply);

				delete pack;
				return;
			}
		}
	}
#endif
	//FIXME: send error?
	delete pack;
}



void
of12_endpoint::handle_group_stats_request(
		cofctl *ctl,
		cofmsg_group_stats_request *msg)
{
	// we need to get the statistics, build a packet and send it
	unsigned int i;
	cmemory body(0);
	unsigned int num_of_buckets;

	uint32_t group_id = msg->get_group_stats().get_group_id();
	
	 of12_stats_group_msg_t *g_msg = fwd_module_of12_get_group_stats(sw->dpid, group_id);
	
	if(g_msg==NULL){
		//TODO handle error
		WRITELOG(CDATAPATH, ERROR,"<%s:%d> ERROR MESSAGE NOT CREATED\n",__func__,__LINE__);
	}
	
	num_of_buckets = g_msg->num_of_buckets;
	body.resize(sizeof(of12_stats_group_msg_t)+num_of_buckets*sizeof(of12_stats_bucket_counter_t));
	
	struct ofp12_group_stats *stats = (struct ofp12_group_stats *)body.somem();
	
	//WARNING BYTE ORDER!!
	
	stats->group_id = htonl( g_msg->group_id);
	stats->ref_count = htonl(g_msg->ref_count);
	stats->packet_count = htobe64(g_msg->packet_count);
	stats->byte_count = htobe64(g_msg->byte_count);
	
	for(i=0;i<num_of_buckets;i++){
		stats->bucket_stats[i].byte_count = htobe64(g_msg->bucket_stats[i].byte_count);
		stats->bucket_stats[i].packet_count = htobe64(g_msg->bucket_stats[i].packet_count);
	}
	
	send_stats_reply(ctl, msg->get_xid(), OFPST_GROUP, body.somem(), body.memlen(), false);
	of12_destroy_stats_group_msg(g_msg);

	delete msg;
}



void
of12_endpoint::handle_group_desc_stats_request(
		cofctl *ctl,
		cofmsg_group_desc_stats_request *msg)
{
	std::vector<cofgroup_desc_stats_reply> group_desc_stats;

	//TODO: fill in std::vector<...> group_desc_stats, when groups are implemented
#if 0
	group_desc_stats.push_back(cofgroup_desc_stats_reply(ctl->get_version()));
#endif


	send_group_desc_stats_reply(
			ctl,
			msg->get_xid(),
			group_desc_stats);

	delete msg;
}



void
of12_endpoint::handle_group_features_stats_request(
		cofctl *ctl,
		cofmsg_group_features_stats_request *msg)
{
	cofgroup_features_stats_reply group_features_reply(ctl->get_version());

	//TODO: fill in group_features_reply, when groups are implemented

	send_group_features_stats_reply(
			ctl,
			msg->get_xid(),
			group_features_reply);

	delete msg;
}



void
of12_endpoint::handle_experimenter_stats_request(
		cofctl *ctl,
		cofmsg_stats_request *pack)
{

	//TODO: when exp are supported 

	delete pack;
}



void
of12_endpoint::handle_packet_out(
		cofctl *ctl,
		cofmsg_packet_out *msg)
{
	of12_action_group_t* action_group = of12_init_action_group(NULL);

	of12_translation_utils::of12_map_flow_entry_actions(ctl, sw, msg->get_actions(), action_group, NULL); //TODO: is this OK always NULL?

	/* assumption: driver can handle all situations properly:
	 * - data and datalen both 0 and buffer_id != OFP_NO_BUFFER
	 * - buffer_id == OFP_NO_BUFFER and data and datalen both != 0
	 * - everything else is an error?
	 */
	if (AFA_FAILURE == fwd_module_of12_process_packet_out(sw->dpid,
							msg->get_buffer_id(),
							msg->get_in_port(),
							action_group,
							msg->get_packet().soframe(), msg->get_packet().framelen())){
		// log error
		//FIXME: send error
	}

	of12_destroy_action_group(action_group);

	delete msg;
}







afa_result_t
of12_endpoint::process_packet_in(
		uint8_t table_id,
		uint8_t reason,
		uint32_t in_port,
		uint32_t buffer_id,
		uint8_t* pkt_buffer,
		uint32_t buf_len,
		uint16_t total_len,
		of12_packet_matches_t matches)
{
	try {
		// classify packet and extract matches as instance of cofmatch
		cpacket pack(pkt_buffer, buf_len, in_port, true);

		send_packet_in_message(
				NULL,
				buffer_id,
				total_len,
				reason,
				table_id,
				/*cookie=*/0,
				/*in_port=*/0, // OF1.0 only
				pack.get_match(),
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

afa_result_t of12_endpoint::notify_port_add(switch_port_t* port){

	uint32_t config=0x0;

	//Compose port config
	if(!port->up) config |= OFPPC_PORT_DOWN; 
	if(!port->of_generate_packet_in) config |= OFPPC_NO_PACKET_IN;
	if(!port->forward_packets) config |= OFPPC_NO_FWD;
	if(port->drop_received) config |= OFPPC_NO_RECV;
			
	
	cofport ofport(OFP12_VERSION);
	ofport.set_port_no(port->of_port_num);
	ofport.set_hwaddr(cmacaddr(port->hwaddr, OFP_ETH_ALEN));
	ofport.set_name(std::string(port->name));
	ofport.set_config(config);
	ofport.set_state(port->state);
	ofport.set_curr(port->curr);
	ofport.set_advertised(port->advertised);
	ofport.set_supported(port->supported);
	ofport.set_peer(port->peer);
	ofport.set_curr_speed(of12_translation_utils::get_port_speed_kb(port->curr_speed));
	ofport.set_max_speed(of12_translation_utils::get_port_speed_kb(port->curr_max_speed));
	
	//Send message
	send_port_status_message(NULL, OFPPR_ADD, ofport);

	return AFA_SUCCESS;
}

afa_result_t of12_endpoint::notify_port_delete(switch_port_t* port){

	uint32_t config=0x0;

	//Compose port config
	if(!port->up) config |= OFPPC_PORT_DOWN; 
	if(!port->of_generate_packet_in) config |= OFPPC_NO_PACKET_IN;
	if(!port->forward_packets) config |= OFPPC_NO_FWD;
	if(port->drop_received) config |= OFPPC_NO_RECV;
	
	cofport ofport(OFP12_VERSION);
	ofport.set_port_no(port->of_port_num);
	ofport.set_hwaddr(cmacaddr(port->hwaddr, OFP_ETH_ALEN));
	ofport.set_name(std::string(port->name));
	ofport.set_config(config);
	ofport.set_state(port->state);
	ofport.set_curr(port->curr);
	ofport.set_advertised(port->advertised);
	ofport.set_supported(port->supported);
	ofport.set_peer(port->peer);
	ofport.set_curr_speed(of12_translation_utils::get_port_speed_kb(port->curr_speed));
	ofport.set_max_speed(of12_translation_utils::get_port_speed_kb(port->curr_max_speed));
	
	//Send message
	send_port_status_message(NULL, OFPPR_DELETE, ofport);

	return AFA_SUCCESS;
}

afa_result_t of12_endpoint::notify_port_status_changed(switch_port_t* port){

	uint32_t config=0x0;

	//Compose port config
	if(!port->up) config |= OFPPC_PORT_DOWN; 
	if(!port->of_generate_packet_in) config |= OFPPC_NO_PACKET_IN;
	if(!port->forward_packets) config |= OFPPC_NO_FWD;
	if(port->drop_received) config |= OFPPC_NO_RECV;
	
	//Notify OF controller
	cofport ofport(OFP12_VERSION);
	ofport.set_port_no(port->of_port_num);
	ofport.set_hwaddr(cmacaddr(port->hwaddr, OFP_ETH_ALEN));
	ofport.set_name(std::string(port->name));
	ofport.set_config(config);
	ofport.set_state(port->state);
	ofport.set_curr(port->curr);
	ofport.set_advertised(port->advertised);
	ofport.set_supported(port->supported);
	ofport.set_peer(port->peer);
	ofport.set_curr_speed(of12_translation_utils::get_port_speed_kb(port->curr_speed));
	ofport.set_max_speed(of12_translation_utils::get_port_speed_kb(port->curr_max_speed));
	
	//Send message
	send_port_status_message(NULL, OFPPR_MODIFY, ofport);

	return AFA_SUCCESS; // ignore this notification
}





void
of12_endpoint::handle_barrier_request(
		cofctl *ctl,
		cofmsg_barrier_request *pack)
{

	//Since we are not queuing messages currently
	send_barrier_reply(ctl, pack->get_xid());		

	delete pack;
}



void
of12_endpoint::handle_flow_mod(
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
of12_endpoint::flow_mod_add(
		cofctl *ctl,
		cofmsg_flow_mod *msg) //throw (eOfSmPipelineBadTableId, eOfSmPipelineTableFull)
{
	uint8_t table_id = msg->get_table_id();
	// sanity check: table for table-id must exist
	if ( (table_id > of12switch->pipeline->num_of_tables) && (table_id != OFPTT_ALL) )
	{
		WRITELOG(CDATAPATH, ERROR, "of12_endpoint(%s)::flow_mod_add() "
				"invalid table-id:%d in flow-mod command",
				sw->dpname.c_str(), msg->get_table_id());

		throw eFlowModBadTableId();
	}

	of12_flow_entry_t *entry = of12_translation_utils::of12_map_flow_entry(ctl, msg,sw);

	if (NULL == entry)
	{
		WRITELOG(CDATAPATH, ERROR, "of12_endpoint(%s)::flow_mod_add() "
				"unable to create flow-entry", sw->dpname.c_str());

		throw eFlowModTableFull();
	}

	if (AFA_FAILURE == fwd_module_of12_process_flow_mod_add(sw->dpid,
								msg->get_table_id(),
								entry,
								msg->get_buffer_id(),
								msg->get_flags() & OFPFF_CHECK_OVERLAP,
								msg->get_flags() & OFPFF_RESET_COUNTS)){
		// log error
		WRITELOG(CDATAPATH, ERROR, "Error inserting the flowmod\n");
		of12_destroy_flow_entry(entry);
	}

}



void
of12_endpoint::flow_mod_modify(
		cofctl *ctl,
		cofmsg_flow_mod *pack,
		bool strict)
{

	// sanity check: table for table-id must exist
	if (pack->get_table_id() > of12switch->pipeline->num_of_tables)
	{
		WRITELOG(CDATAPATH, ERROR, "of12_endpoint(%s)::flow_mod_delete() "
				"invalid table-id:%d in flow-mod command",
				sw->dpname.c_str(), pack->get_table_id());

		throw eFlowModBadTableId();
	}

	of12_flow_entry_t *entry = of12_translation_utils::of12_map_flow_entry(ctl, pack,sw);

	if (NULL == entry){
		WRITELOG(CDATAPATH, ERROR, "of12_endpoint(%s)::flow_mod_delete() "
				"unable to attempt to remove flow-entry", sw->dpname.c_str());

		return;
	}

	of12_flow_removal_strictness_t strictness = (strict) ? STRICT : NOT_STRICT;


	if(AFA_FAILURE == fwd_module_of12_process_flow_mod_modify(sw->dpid,
								pack->get_table_id(),
								entry,
								strictness,
								pack->get_flags() & OFPFF_RESET_COUNTS)){
		//TODO: FIXME send exception
		WRITELOG(CDATAPATH, ERROR, "Error modiying flowmod\n");
		of12_destroy_flow_entry(entry);
	} 

}



void
of12_endpoint::flow_mod_delete(
		cofctl *ctl,
		cofmsg_flow_mod *pack,
		bool strict) //throw (eOfSmPipelineBadTableId)
{

	of12_flow_entry_t *entry = of12_translation_utils::of12_map_flow_entry(ctl, pack, sw);

	if (NULL == entry) {
		WRITELOG(CDATAPATH, ERROR, "of12_endpoint(%s)::flow_mod_delete() "
				"unable to attempt to remove flow-entry", sw->dpname.c_str());

		return;
	}

	of12_flow_removal_strictness_t strictness = (strict) ? STRICT : NOT_STRICT;


	if(AFA_FAILURE == fwd_module_of12_process_flow_mod_delete(sw->dpid,
								pack->get_table_id(),
								entry,
								pack->get_buffer_id(),
								pack->get_out_port(),
								pack->get_out_group(),
								strictness)) {
		WRITELOG(CDATAPATH, ERROR, "Error deleting flowmod\n");
		//TODO: treat exception
	} 
	of12_destroy_flow_entry(entry);

}






afa_result_t
of12_endpoint::process_flow_removed(
		uint8_t reason,
		of12_flow_entry *entry)
{
	cofmatch match;
	uint32_t sec,nsec;

	of12_translation_utils::of12_map_reverse_flow_entry_matches(entry->matchs, match);

	//get duration of the flow mod
	of12_stats_flow_get_duration(entry, &sec, &nsec);


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
of12_endpoint::handle_group_mod(
		cofctl *ctl,
		cofmsg_group_mod *msg)
{
	//throw eNotImplemented(std::string("of12_endpoint::handle_group_mod()"));
	//steps:
	/* 1- map the packet
	 * 2- check for errors
	 * 3- call driver function?
	 */

	WRITELOG(CDATAPATH, DBG, "of12_endpoint::handle_group_mod() => buckets: %s\n", msg->get_buckets().c_str());

#if 0
	// sanity check: check for invalid actions => FIXME: fake for oftest12, there are numerous
	// combinations, where an action list may be invalid, especially when heterogeneous tables
	// in terms of capabilities exist!
	for (cofbclist::iterator it = msg->get_buckets().begin(); it != msg->get_buckets().end(); ++it) {
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

	rofl_of12_gm_result_t ret_val;
 	of12_bucket_list_t* bucket_list=of12_init_bucket_list();
	
	switch(msg->get_command()){
		case OFPGC_ADD:
			of12_translation_utils::of12_map_bucket_list(ctl, sw, msg->get_buckets(), bucket_list);
			ret_val = fwd_module_of12_group_mod_add(sw->dpid, (of12_group_type_t)msg->get_group_type(), msg->get_group_id(), bucket_list);
			break;
			
		case OFPGC_MODIFY:
			of12_translation_utils::of12_map_bucket_list(ctl, sw, msg->get_buckets(), bucket_list);
			ret_val = fwd_module_of12_group_mod_modify(sw->dpid, (of12_group_type_t)msg->get_group_type(), msg->get_group_id(), bucket_list);
			break;
		
		case OFPGC_DELETE:
			ret_val = fwd_module_of12_group_mod_delete(sw->dpid, msg->get_group_id());
			break;
		
		default:
			ret_val = ROFL_OF12_GM_BCOMMAND;
			break;
	}

	switch(ret_val){
		case ROFL_OF12_GM_OK:
			break;
		case ROFL_OF12_GM_EXISTS:
			throw eGroupModExists();
			break;
		case ROFL_OF12_GM_INVAL:
			throw eGroupModInvalGroup();
			break;
		case ROFL_OF12_GM_WEIGHT:
			throw eGroupModWeightUnsupported();
			break;
		case ROFL_OF12_GM_OGRUPS:
			throw eGroupModOutOfGroups();
			break;
		case ROFL_OF12_GM_OBUCKETS:
			throw eGroupModOutOfBuckets();
			break;
		case ROFL_OF12_GM_CHAIN:
			throw eGroupModChainingUnsupported();
			break;
		case ROFL_OF12_GM_WATCH:
			throw eGroupModWatchUnsupported();
			break;
		case ROFL_OF12_GM_LOOP:
			throw eGroupModLoop();
			break;
		case ROFL_OF12_GM_UNKGRP:
			throw eGroupModUnknownGroup();
			break;
		case ROFL_OF12_GM_CHNGRP:
			throw eGroupModChainedGroup();
			break;
		case ROFL_OF12_GM_BTYPE:
			throw eGroupModBadType();
			break;
		case ROFL_OF12_GM_BCOMMAND:
			throw eGroupModBadCommand();
			break;
		case ROFL_OF12_GM_BBUCKET:
			throw eGroupModBadBucket();
			break;
		case ROFL_OF12_GM_BWATCH:
			throw eGroupModBadWatch();
			break;
		case ROFL_OF12_GM_EPERM:
			throw eGroupModEperm();
			break;
		default:
			/*Not a valid value - Log error*/
			break;
	}

	delete msg;
}



void
of12_endpoint::handle_table_mod(
		cofctl *ctl,
		cofmsg_table_mod *msg)
{

	/*
	 * the parameters defined in the pipeline OF12_TABLE_...
	 * match those defined by the OF1.2 specification.
	 * This may change in the future for other versions, so map
	 * the OF official numbers to the ones used by the pipeline.
	 *
	 * at least we map network byte order to host byte order here ...
	 */
	of12_flow_table_miss_config_t config = OF12_TABLE_MISS_CONTROLLER; //Default 

	if (msg->get_config() == OFPTC_TABLE_MISS_CONTINUE){
		config = OF12_TABLE_MISS_CONTINUE;
	}else if (msg->get_config() == OFPTC_TABLE_MISS_CONTROLLER){
		config = OF12_TABLE_MISS_CONTROLLER;
	}else if (msg->get_config() == OFPTC_TABLE_MISS_DROP){
		config = OF12_TABLE_MISS_DROP;
	}

	if( AFA_FAILURE == fwd_module_of12_set_table_config(sw->dpid, msg->get_table_id(), config) ){
		//TODO: treat exception
	} 

	delete msg;
}



void
of12_endpoint::handle_port_mod(
		cofctl *ctl,
		cofmsg_port_mod *msg)
{
	uint32_t config, mask, advertise, port_num;

	config 		= msg->get_config();
	mask 		= msg->get_mask();
	advertise 	= msg->get_advertise();
	port_num 	= msg->get_port_no();

	//Check if port_num FLOOD
	//TODO: Inspect if this is right. Spec does not clearly define if this should be supported or not
	if( port_num == OFPP_ANY )
		throw ePortModBadPort(); 
		
	//Drop received
	if( mask &  OFPPC_NO_RECV ){
	
		if( AFA_FAILURE == fwd_module_of12_set_port_drop_received_config(sw->dpid, port_num, config & OFPPC_NO_RECV ) ){
			//TODO: treat exception
		}
	}
	//No forward
	if( mask &  OFPPC_NO_FWD ){
	
		if( AFA_FAILURE == fwd_module_of12_set_port_forward_config(sw->dpid, port_num, !(config & OFPPC_NO_FWD) ) ){
			//TODO: treat exception
		}
	}

	//No packet in
	if( mask &  OFPPC_NO_PACKET_IN ){
	
		if( AFA_FAILURE == fwd_module_of12_set_port_generate_packet_in_config(sw->dpid, port_num, !(config & OFPPC_NO_PACKET_IN) ) ){
			//TODO: treat exception
		}
	}

	//Advertised
	if( advertise ){ 
	
		if( AFA_FAILURE == fwd_module_of12_set_port_advertise_config(sw->dpid, port_num, advertise)  ){
			//TODO: treat exception
		}
	}

	//Port admin down //TODO: evaluate if we can directly call fwd_module_enable_port_by_num instead
	if( mask &  OFPPC_PORT_DOWN ){
	
		if( (config & OFPPC_PORT_DOWN)  ){
			//Disable port
			if( AFA_FAILURE == fwd_module_disable_port_by_num(sw->dpid, port_num) ){
				//TODO: treat exception
			}
		}else{
			if( AFA_FAILURE == fwd_module_enable_port_by_num(sw->dpid, port_num) ){
				//TODO: treat exception
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
of12_endpoint::handle_set_config(
		cofctl *ctl,
		cofmsg_set_config *msg)
{

	//Instruct the driver to process the set config	
	if(AFA_FAILURE == fwd_module_of12_set_pipeline_config(sw->dpid, msg->get_flags(), msg->get_miss_send_len())){
		//TODO: what to do
	}
		
	delete msg;
}



void
of12_endpoint::handle_queue_get_config_request(
		cofctl *ctl,
		cofmsg_queue_get_config_request *pack)
{
	switch_port_t* port;
	unsigned int portnum = pack->get_port_no(); 
	cofmsg_queue_get_config_reply reply(OFP12_VERSION);

	//we check all the positions in case there are empty slots
	for(unsigned int n = 1; n < of12switch->max_ports; n++){

		port = of12switch->logical_ports[n].port; 

		if((port != NULL) && (of12switch->logical_ports[n].attachment_state == LOGICAL_PORT_STATE_ATTACHED) && (port->of_port_num == portnum)){

			for(unsigned int i=0; i<port->max_queues; i++){
				if(port->queues[i].set){	
/*					reply.push_back(
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
*/
				}
			
				//Send reply
				//XXX
				//send_queue_get_config_reply(ctl, reply);

				delete pack;
				return;
			}
		}
	}

	//FIXME: send error?
	delete pack;
}



void
of12_endpoint::handle_experimenter_message(
		cofctl *ctl,
		cofmsg_features_request *pack)
{
	// TODO

	delete pack;
}



void
of12_endpoint::handle_ctrl_open(cofctl *ctrl)
{

}



void
of12_endpoint::handle_ctrl_close(cofctl *ctrl)
{

}

