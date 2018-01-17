/*
 * of10_translation_utils.cc
 *
 *  Created on: 06.09.2013
 *      Author: andreas
 */

#include "of10_translation_utils.h"
#include <stdint.h>
#include <inttypes.h>

using namespace xdpd;

/*
* Port utils
*/
#define HAS_CAPABILITY(bitmap,cap) (bitmap&cap) > 0
uint32_t of10_translation_utils::get_port_speed_kb(port_features_t features){

	if(HAS_CAPABILITY(features, PORT_FEATURE_1TB_FD))
		return 1000000000;
	if(HAS_CAPABILITY(features, PORT_FEATURE_100GB_FD))
		return 100000000;
	if(HAS_CAPABILITY(features, PORT_FEATURE_40GB_FD))
		return 40000000;
	if(HAS_CAPABILITY(features, PORT_FEATURE_1GB_FD) || HAS_CAPABILITY(features, PORT_FEATURE_1GB_HD))
		return 1000000;
	if(HAS_CAPABILITY(features, PORT_FEATURE_100MB_FD) || HAS_CAPABILITY(features, PORT_FEATURE_100MB_HD))
		return 100000;

	if(HAS_CAPABILITY(features, PORT_FEATURE_10MB_FD) || HAS_CAPABILITY(features, PORT_FEATURE_10MB_HD))
		return 10000;

	return 0;
}

/**
* Maps a of1x_flow_entry from an OF1.2 Header
*/
of1x_flow_entry_t*
of10_translation_utils::of1x_map_flow_entry(
		crofctl *ctl,
		rofl::openflow::cofmsg_flow_mod *msg,
		openflow_switch* sw)
{

	of1x_flow_entry_t *entry = of1x_init_flow_entry(msg->get_flowmod().get_flags() & openflow10::OFPFF_SEND_FLOW_REM, /*builtin=*/false);

	if(!entry)
		throw eFlowModUnknown();

	// store flow-mod fields in of1x_flow_entry
	entry->priority 		= msg->get_flowmod().get_priority();
	entry->cookie 			= msg->get_flowmod().get_cookie();
	entry->cookie_mask 		= 0xFFFFFFFFFFFFFFFFULL;
	entry->timer_info.idle_timeout	= msg->get_flowmod().get_idle_timeout();
	entry->timer_info.hard_timeout	= msg->get_flowmod().get_hard_timeout();

	try{
		// extract OXM fields from pack and store them in of1x_flow_entry
		of10_map_flow_entry_matches(ctl, msg->get_flowmod().get_match(), sw, entry);
	}catch(...){
		of1x_destroy_flow_entry(entry);
		throw eFlowModUnknown();
	}

	// for OpenFlow 1.0 => add a single instruction APPLY-ACTIONS to instruction group
	of1x_action_group_t *apply_actions = of1x_init_action_group(0);

	try{
		of1x_map_flow_entry_actions(ctl, sw, msg->set_flowmod().set_actions(), apply_actions, /*of1x_write_actions_t*/0);
	}catch(...){
		of1x_destroy_flow_entry(entry);
		throw eFlowModUnknown();
	}

	of1x_add_instruction_to_group(
			&(entry->inst_grp),
			OF1X_IT_APPLY_ACTIONS,
			(of1x_action_group_t*)apply_actions,
			NULL,
			NULL,
			/*go_to_table*/0);

	return entry;
}



/**
* Maps a of1x_match from an OF1.0 Header
*/
void
of10_translation_utils::of10_map_flow_entry_matches(
		crofctl *ctl,
		rofl::openflow::cofmatch const& ofmatch,
		openflow_switch* sw,
		of1x_flow_entry *entry)
{
	of1x_match_t *match;	

	try {
		match = of1x_init_port_in_match(ofmatch.get_in_port());
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	// no in_phy_port in OF1.0
	try {
		uint64_t maddr = ofmatch.get_eth_src_addr().get_mac();
		uint64_t mmask = rofl::cmacaddr("FF:FF:FF:FF:FF:FF").get_mac(); // no mask in OF1.0
		match = of1x_init_eth_src_match(maddr,mmask);
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}
	
	try {
		uint64_t maddr = ofmatch.get_eth_dst_addr().get_mac();
		uint64_t mmask = rofl::cmacaddr("FF:FF:FF:FF:FF:FF").get_mac(); // no mask in OF1.0
		match = of1x_init_eth_dst_match(maddr,mmask);
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}


	try {
		match = of1x_init_eth_type_match(ofmatch.get_eth_type());
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	try {
		enum of1x_vlan_present vlan_present;
		uint16_t value = ofmatch.get_vlan_vid_value();
		/*
		 * clear bit 12 in value, even if this does not exist in OF10,
		 * as the pipeline may get interprete this bit otherwise
		 */
		if(value == rofl::openflow10::OFP_VLAN_NONE )
			vlan_present = OF1X_MATCH_VLAN_NONE;
		else
			vlan_present = OF1X_MATCH_VLAN_SPECIFIC;
		match = of1x_init_vlan_vid_match(value & ~openflow::OFPVID_PRESENT, NTOHB16(OF1X_VLAN_ID_MASK), vlan_present); // no mask in OF1.0
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	try {
		match = of1x_init_vlan_pcp_match(ofmatch.get_vlan_pcp());
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//NW TOS
	try {
		uint8_t dscp = OF1X_IP_DSCP_VALUE(ofmatch.get_nw_tos()); //Align to get DSCP value from TOS
		match = of1x_init_ip_dscp_match(dscp);
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//NW PROTO 
	try {
		match = of1x_init_nw_proto_match(ofmatch.get_nw_proto());
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//NW SRC
	try {
		uint32_t value = ofmatch.get_nw_src_value().get_addr_hbo();
		uint32_t mask = ofmatch.get_nw_src_mask().get_addr_hbo();
		if(value != 0x0){
			match = of1x_init_nw_src_match(value, mask);
			of1x_add_match_to_entry(entry, match);
		}
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//NW DST 
	try {
		uint32_t value = ofmatch.get_nw_dst_value().get_addr_hbo();
		uint32_t mask = ofmatch.get_nw_dst_mask().get_addr_hbo();
		if(value != 0x0){
			match = of1x_init_nw_dst_match(value, mask);
			of1x_add_match_to_entry(entry, match);
		}
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//TP SRC
	try {
		match = of1x_init_tp_src_match(ofmatch.get_tp_src());
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//TP DST
	try {
		match = of1x_init_tp_dst_match(ofmatch.get_tp_dst());
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

}



/**
* Maps a of1x_action from an OF1.0 Header
*/
void
of10_translation_utils::of1x_map_flow_entry_actions(
		crofctl *ctl,
		openflow_switch* sw,
		const rofl::openflow::cofactions& actions,
		of1x_action_group_t *apply_actions,
		of1x_write_actions_t *write_actions)
{
	for (std::map<rofl::cindex, unsigned int>::const_iterator
			jt = actions.get_actions_index().begin();
					jt != actions.get_actions_index().end(); ++jt) {

		const rofl::cindex& index 	= jt->first;
		const unsigned int& type	= jt->second;

		of1x_packet_action_t *action = NULL;
		wrap_uint_t field;
		memset(&field,0,sizeof(wrap_uint_t));

		switch (type) {
			case rofl::openflow10::OFPAT_OUTPUT:
				//Translate special values to of1x
				field.u32 = get_out_port(actions.get_action_output(index).get_port_no());
				action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, actions.get_action_output(index).get_max_len());
				break;
			case rofl::openflow10::OFPAT_SET_VLAN_VID:
				field.u16 = actions.get_action_set_vlan_vid(index).get_vlan_vid();
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_VID, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_VLAN_PCP:
				field.u8 = actions.get_action_set_vlan_pcp(index).get_vlan_pcp();
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_PCP, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_STRIP_VLAN:
				action = of1x_init_packet_action( OF1X_AT_POP_VLAN, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_DL_SRC: {
				field.u64 = actions.get_action_set_dl_src(index).get_dl_src().get_mac();
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_SRC, field, 0x0);
				} break;
			case rofl::openflow10::OFPAT_SET_DL_DST: {
				field.u64 = actions.get_action_set_dl_dst(index).get_dl_dst().get_mac();
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_DST, field, 0x0);
				} break;
			case rofl::openflow10::OFPAT_SET_NW_SRC:
				field.u32 = actions.get_action_set_nw_src(index).get_nw_src().get_addr_hbo();
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_NW_SRC, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_NW_DST:
				field.u32 = actions.get_action_set_nw_dst(index).get_nw_dst().get_addr_hbo();
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_NW_DST, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_NW_TOS:
				field.u8 = OF1X_IP_DSCP_VALUE(actions.get_action_set_nw_tos(index).get_nw_tos()); //Align to get DSCP value from TOS
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_DSCP, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_TP_SRC:
				field.u16 = actions.get_action_set_tp_src(index).get_tp_src();
				action = of1x_init_packet_action(OF1X_AT_SET_FIELD_TP_SRC, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_TP_DST:
				field.u16 = actions.get_action_set_tp_dst(index).get_tp_dst();
				action = of1x_init_packet_action(OF1X_AT_SET_FIELD_TP_DST, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_ENQUEUE:
				field.u32 = actions.get_action_enqueue(index).get_queue_id();
				action = of1x_init_packet_action( OF1X_AT_SET_QUEUE, field, 0x0);
				if (NULL != apply_actions) of1x_push_packet_action_to_group(apply_actions, action);
				field.u64 = get_out_port(actions.get_action_enqueue(index).get_port_no());
				action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, 0x0);
				break;
			}

			if (NULL != apply_actions)
			{
				of1x_push_packet_action_to_group(apply_actions, action);
			}
	}
#if 0
	for (std::list<rofl::openflow::cofaction*>::iterator
			jt = actions.begin(); jt != actions.end(); ++jt)
	{
		rofl::openflow::cofaction& raction = *(*jt);

		of1x_packet_action_t *action = NULL;
		wrap_uint_t field;
		memset(&field,0,sizeof(wrap_uint_t));

		switch (raction.get_type()) {
			case rofl::openflow10::OFPAT_OUTPUT:
				//Translate special values to of1x
				field.u32 = get_out_port(NTOHB16(raction.oac_10output->port));
				action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, NTOHB16(raction.oac_10output->max_len));
				break;
			case rofl::openflow10::OFPAT_SET_VLAN_VID:
				field.u16 = NTOHB16(raction.oac_10vlanvid->vlan_vid);
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_VID, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_VLAN_PCP:
				field.u8 = raction.oac_10vlanpcp->vlan_pcp;
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_PCP, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_STRIP_VLAN:
				action = of1x_init_packet_action( OF1X_AT_POP_VLAN, field, 0x0); 
				break;
			case rofl::openflow10::OFPAT_SET_DL_SRC: {
				cmacaddr mac(raction.oac_10dladdr->dl_addr, 6);
				field.u64 = mac.get_mac();
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_SRC, field, 0x0);
				} break;
			case rofl::openflow10::OFPAT_SET_DL_DST: {
				cmacaddr mac(raction.oac_10dladdr->dl_addr, 6);
				field.u64 = mac.get_mac();
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_DST, field, 0x0);
				} break;
			case rofl::openflow10::OFPAT_SET_NW_SRC:
				field.u32 = NTOHB32(raction.oac_10nwaddr->nw_addr);
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_NW_SRC, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_NW_DST:
				field.u32 = NTOHB32(raction.oac_10nwaddr->nw_addr);
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_NW_DST, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_NW_TOS:
				field.u8 = OF1X_IP_DSCP_VALUE(raction.oac_10nwtos->nw_tos); //Align to get DSCP value from TOS
				action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_DSCP, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_TP_SRC:
				field.u16 = NTOHB16(raction.oac_10tpport->tp_port);
				action = of1x_init_packet_action(OF1X_AT_SET_FIELD_TP_SRC, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_SET_TP_DST:
				field.u16 = NTOHB16(raction.oac_10tpport->tp_port);
				action = of1x_init_packet_action(OF1X_AT_SET_FIELD_TP_DST, field, 0x0);
				break;
			case rofl::openflow10::OFPAT_ENQUEUE:
				field.u32 = NTOHB32(raction.oac_10enqueue->queue_id);
				action = of1x_init_packet_action( OF1X_AT_SET_QUEUE, field, 0x0);
				if (NULL != apply_actions) of1x_push_packet_action_to_group(apply_actions, action);
				field.u64 = get_out_port(NTOHB16(raction.oac_10enqueue->port));
				action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, 0x0);
				break;
			}

			if (NULL != apply_actions)
			{
				of1x_push_packet_action_to_group(apply_actions, action);
			}
	}
#endif
}



/*
* Maps a of1x_action TO an OF1.0 Header
*/
void
of10_translation_utils::of1x_map_reverse_flow_entry_matches(
		of1x_match_t* m,
		rofl::openflow::cofmatch& match)
{
	//bool has_vlan=false;
	while (NULL != m)
	{
		switch (m->type) {
		case OF1X_MATCH_IN_PORT:
			match.set_in_port(of1x_get_match_value32(m));
			break;
		case OF1X_MATCH_ETH_DST:
		{
			match.set_eth_dst(cmacaddr(of1x_get_match_value64(m)), cmacaddr(of1x_get_match_mask64(m)));
		}
			break;
		case OF1X_MATCH_ETH_SRC:
		{
			match.set_eth_src(cmacaddr(of1x_get_match_value64(m)), cmacaddr(of1x_get_match_mask64(m)));
		}
			break;
		case OF1X_MATCH_ETH_TYPE:
			match.set_eth_type(of1x_get_match_value16(m));
			break;
		case OF1X_MATCH_VLAN_VID:
			//has_vlan = true;
			if(m->vlan_present == OF1X_MATCH_VLAN_NONE){
				match.set_vlan_vid(rofl::openflow10::OFP_VLAN_NONE);
					
				//Acording to spec 1.0.2 we should set pcp to 0 to avoid having wildcard flag for PCP
				match.set_vlan_pcp(0x0);
			}else
				match.set_vlan_vid(of1x_get_match_value16(m));
			break;
		case OF1X_MATCH_VLAN_PCP:
			match.set_vlan_pcp(of1x_get_match_value8(m));
			break;
		case OF1X_MATCH_ARP_OP:
			match.set_nw_proto(of1x_get_match_value16(m));
			break;
		case OF1X_MATCH_ARP_SPA:
		{
			caddress_in4 addr; addr.set_addr_hbo(of1x_get_match_value32(m));
			caddress_in4 mask; mask.set_addr_hbo(of1x_get_match_mask32(m));
			match.set_nw_src(addr, mask);
		}
			break;
		case OF1X_MATCH_ARP_TPA:
		{
			caddress_in4 addr; addr.set_addr_hbo(of1x_get_match_value32(m));
			caddress_in4 mask; mask.set_addr_hbo(of1x_get_match_mask32(m));
			match.set_nw_dst(addr, mask);
		}
			break;
		case OF1X_MATCH_IP_DSCP:
			match.set_nw_tos(OF1X_IP_DSCP_ALIGN(of1x_get_match_value8(m))); //getting TOS value from DSCP
			break;
		case OF1X_MATCH_NW_PROTO:
			match.set_nw_proto(of1x_get_match_value8(m));
			break;
		case OF1X_MATCH_NW_SRC:
		{
			caddress_in4 addr; addr.set_addr_hbo(of1x_get_match_value32(m));
			caddress_in4 mask; mask.set_addr_hbo(of1x_get_match_mask32(m));
			match.set_nw_src(addr, mask);
		}
			break;
		case OF1X_MATCH_NW_DST:
		{
			caddress_in4 addr; addr.set_addr_hbo(of1x_get_match_value32(m));
			caddress_in4 mask; mask.set_addr_hbo(of1x_get_match_mask32(m));
			match.set_nw_dst(addr, mask);
		}
			break;
		case OF1X_MATCH_TP_SRC:
			match.set_tp_src(of1x_get_match_value16(m));
			break;
		case OF1X_MATCH_TP_DST:
			match.set_tp_dst(of1x_get_match_value16(m));
			break;
		default:
			break;
		}

		m = m->next;
	}

	//In 1.0 if there is no VLAN OFP10_VLAN_NONE has to be set...
	//if(!has_vlan)
	//	match.set_vlan_untagged();
}



/**
*
*/
void
of10_translation_utils::of1x_map_reverse_flow_entry_actions(
		of1x_instruction_group_t* group,
		rofl::openflow::cofactions& actions,
		uint16_t pipeline_miss_send_len)
{
	for (unsigned int i = 0; i < (sizeof(group->instructions) / sizeof(of1x_instruction_t)); i++) {

		if (OF1X_IT_APPLY_ACTIONS != group->instructions[i].type)
			continue;

		if(!group->instructions[i].apply_actions)
			continue;

		rofl::cindex index;

		for (of1x_packet_action_t *of1x_action = group->instructions[i].apply_actions->head; of1x_action != NULL; of1x_action = of1x_action->next) {
			if (OF1X_AT_NO_ACTION == of1x_action->type)
				continue;
			rofl::openflow::cofaction action(OFP10_VERSION);
			of1x_map_reverse_flow_entry_action(of1x_action, index++, actions, pipeline_miss_send_len);
			
			//Skip next action if action is set-queue (SET-QUEUE-OUTPUT)
			if(of1x_action->type == OF1X_AT_SET_QUEUE){
				if(of1x_action->next && !of1x_action->next->next)
					break;
				else
					of1x_action = of1x_action->next; //Skip output
			}
		}

		break;
	}
}




void
of10_translation_utils::of1x_map_reverse_flow_entry_action(
		of1x_packet_action_t* of1x_action,
		const rofl::cindex& index,
		rofl::openflow::cofactions& actions,
		uint16_t pipeline_miss_send_len)
{
	/*
	 * FIXME: add masks for those fields defining masked values in the specification
	 */


	switch (of1x_action->type) {
	case OF1X_AT_NO_ACTION: {
		// do nothing
	} break;
	case OF1X_AT_POP_VLAN: {
		actions.add_action_strip_vlan(index);
	} break;
	case OF1X_AT_PUSH_VLAN: {
		actions.add_action_push_vlan(index).set_eth_type(of1x_get_packet_action_field16(of1x_action));
	} break;
	case OF1X_AT_SET_FIELD_ETH_DST: {
		uint64_t mac = of1x_get_packet_action_field64(of1x_action);
		actions.add_action_set_dl_dst(index).set_dl_dst(cmacaddr(mac));
	} break;
	case OF1X_AT_SET_FIELD_ETH_SRC: {
		uint64_t mac = of1x_get_packet_action_field64(of1x_action);
		actions.add_action_set_dl_src(index).set_dl_src(cmacaddr(mac));
	} break;
	case OF1X_AT_SET_FIELD_VLAN_VID: {
		actions.add_action_set_vlan_vid(index).set_vlan_vid(of1x_get_packet_action_field16(of1x_action));
	} break;
	case OF1X_AT_SET_FIELD_VLAN_PCP: {
		actions.add_action_set_vlan_pcp(index).set_vlan_pcp(of1x_get_packet_action_field8(of1x_action));
	} break;
	case OF1X_AT_SET_FIELD_IP_DSCP: {
		actions.add_action_set_nw_tos(index).set_nw_tos(OF1X_IP_DSCP_ALIGN(of1x_get_packet_action_field8(of1x_action))); //We need to get the TOS value from the DSCP
	} break;
	case OF1X_AT_SET_FIELD_NW_SRC: {
		actions.add_action_set_nw_src(index).set_nw_src().set_addr_hbo(of1x_get_packet_action_field32(of1x_action));
	} break;
	case OF1X_AT_SET_FIELD_NW_DST: {
		actions.add_action_set_nw_dst(index).set_nw_dst().set_addr_hbo(of1x_get_packet_action_field32(of1x_action));
	} break;
	case OF1X_AT_SET_FIELD_TP_SRC: {
		actions.add_action_set_tp_src(index).set_tp_src(of1x_get_packet_action_field16(of1x_action));
	} break;
	case OF1X_AT_SET_FIELD_TP_DST: {
		actions.add_action_set_tp_dst(index).set_tp_dst(of1x_get_packet_action_field16(of1x_action));
	} break;
	case OF1X_AT_EXPERIMENTER: {
		// TODO
		//actions.add_action_vendor(index).set_exp_id();
		//actions.add_action_vendor(index).set_exp_body();
	} break;
	case OF1X_AT_SET_QUEUE: {
		//Right after queue we must have an output
		if(of1x_action->next) {
			actions.add_action_enqueue(index).set_port_no(get_out_port_reverse(of1x_get_packet_action_field32(of1x_action->next)));
			actions.set_action_enqueue(index).set_queue_id(of1x_get_packet_action_field32(of1x_action));
		}else{
			assert(0);
		}
	}break;
	case OF1X_AT_OUTPUT: {
		//Setting max_len to the switch max_len (we do not support per action max_len)
		actions.add_action_output(index).set_port_no(get_out_port_reverse(of1x_get_packet_action_field32(of1x_action)));
		actions.set_action_output(index).set_max_len(of1x_action->send_len);
	} break;
	default: {
		// do nothing
	} break;
	}
	
}


/*
* Maps packet actions to cofmatches
*/
void of10_translation_utils::of1x_map_reverse_packet_matches(packet_matches_t* pm, rofl::openflow::cofmatch& match){
	if(packet_matches_get_port_in_value(pm))
		match.set_in_port(packet_matches_get_port_in_value(pm));
	if(packet_matches_get_eth_dst_value(pm)){
		uint64_t mac = packet_matches_get_eth_dst_value(pm);
		match.set_eth_dst(cmacaddr(mac));
	}
	if(packet_matches_get_eth_src_value(pm)){
		uint64_t mac = packet_matches_get_eth_src_value(pm);
		match.set_eth_src(cmacaddr(mac));
	}
	if(packet_matches_get_eth_type_value(pm))
		match.set_eth_type(packet_matches_get_eth_type_value(pm));
	if(packet_matches_has_vlan(pm)){
		if(packet_matches_get_vlan_vid_value(pm))
			match.set_vlan_vid(packet_matches_get_vlan_vid_value(pm));
		if(packet_matches_get_vlan_pcp_value(pm))
			match.set_vlan_pcp(packet_matches_get_vlan_pcp_value(pm));
	}
	if(packet_matches_get_arp_opcode_value(pm))
		match.set_nw_proto(packet_matches_get_arp_opcode_value(pm));
	if(packet_matches_get_arp_spa_value(pm)) {
		caddress_in4 addr;
		addr.set_addr_hbo(packet_matches_get_arp_spa_value(pm));
		match.set_nw_src(addr);
	}
	if(packet_matches_get_arp_tpa_value(pm)) {
		caddress_in4 addr;
		addr.set_addr_hbo(packet_matches_get_arp_tpa_value(pm));
		match.set_nw_dst(addr);
	}
	if(packet_matches_get_ip_dscp_value(pm))
		match.set_nw_tos(OF1X_IP_DSCP_ALIGN(packet_matches_get_ip_dscp_value(pm))); //We need to get the TOS value from the DSCP
	if(packet_matches_get_ip_proto_value(pm))
		match.set_ip_proto(packet_matches_get_ip_proto_value(pm));
	if(packet_matches_get_ipv4_src_value(pm)){
		caddress_in4 addr;
		addr.set_addr_hbo(packet_matches_get_ipv4_src_value(pm));
		match.set_nw_src(addr);
	}
	if(packet_matches_get_ipv4_dst_value(pm)){
		caddress_in4 addr;
		addr.set_addr_hbo(packet_matches_get_ipv4_dst_value(pm));
		match.set_nw_dst(addr);
	}
	if(packet_matches_get_tcp_src_value(pm))
		match.set_tp_src(packet_matches_get_tcp_src_value(pm));
	if(packet_matches_get_tcp_dst_value(pm))
		match.set_tp_dst(packet_matches_get_tcp_dst_value(pm));
	if(packet_matches_get_udp_src_value(pm))
		match.set_tp_src(packet_matches_get_udp_src_value(pm));
	if(packet_matches_get_udp_dst_value(pm))
		match.set_tp_dst(packet_matches_get_udp_dst_value(pm));
	if(packet_matches_get_icmpv4_type_value(pm))
		match.set_tp_src(packet_matches_get_icmpv4_type_value(pm));
	if(packet_matches_get_icmpv4_code_value(pm))
		match.set_tp_dst(packet_matches_get_icmpv4_code_value(pm));
}

uint32_t of10_translation_utils::get_supported_actions(of1x_switch_snapshot_t *lsw){
	uint32_t mask = 0;
	
	of1x_flow_table_config_t* config = &lsw->pipeline.tables[0].config;
		
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_OUTPUT))
		mask |= 1 << rofl::openflow10::OFPAT_OUTPUT;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_FIELD_VLAN_VID))
		mask |= 1 << rofl::openflow10::OFPAT_SET_VLAN_VID;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_FIELD_VLAN_PCP))
		mask |= 1 << rofl::openflow10::OFPAT_SET_VLAN_PCP;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_POP_VLAN))
		mask |= 1 << rofl::openflow10::OFPAT_STRIP_VLAN;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_FIELD_ETH_SRC))
		mask |= 1 << rofl::openflow10::OFPAT_SET_DL_SRC;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_FIELD_ETH_DST))
		mask |= 1 << rofl::openflow10::OFPAT_SET_DL_DST;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_FIELD_IPV4_SRC))
		mask |= 1 << rofl::openflow10::OFPAT_SET_NW_SRC;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_FIELD_IPV4_DST))
		mask |= 1 << rofl::openflow10::OFPAT_SET_NW_DST;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_FIELD_IP_DSCP))
		mask |= 1 << rofl::openflow10::OFPAT_SET_NW_TOS;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_FIELD_TP_SRC))
		mask |= 1 << rofl::openflow10::OFPAT_SET_TP_SRC;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_FIELD_TP_DST))
		mask |= 1 << rofl::openflow10::OFPAT_SET_TP_DST;
	
	if (bitmap128_is_bit_set(&config->apply_actions, OF1X_AT_SET_QUEUE))
		mask |= 1 << rofl::openflow10::OFPAT_ENQUEUE;
		
	return mask;
}

uint32_t of10_translation_utils::get_supported_wildcards(of1x_switch_snapshot_t *lsw){
	
	uint32_t mask = 0;
	of1x_flow_table_config_t* config = &lsw->pipeline.tables[0].config;

	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_IN_PORT ))
		mask |= rofl::openflow10::OFPFW_IN_PORT;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_ETH_DST ))
		mask |=  rofl::openflow10::OFPFW_DL_DST;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_ETH_SRC ))
		mask |=  rofl::openflow10::OFPFW_DL_SRC;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_ETH_TYPE ))
		mask |=  rofl::openflow10::OFPFW_DL_TYPE;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_VLAN_VID ))
		mask |=  rofl::openflow10::OFPFW_DL_VLAN;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_VLAN_PCP ))
		mask |=  rofl::openflow10::OFPFW_DL_VLAN_PCP;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_IP_DSCP ))
		mask |=  rofl::openflow10::OFPFW_NW_TOS;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_NW_PROTO ))
		mask |=  rofl::openflow10::OFPFW_NW_PROTO;

	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_NW_SRC ))
		mask |= rofl::openflow10::OFPFW_NW_SRC_ALL;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_NW_DST ))
		mask |= rofl::openflow10::OFPFW_NW_DST_ALL;

	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_TP_SRC ))
		mask |=  rofl::openflow10::OFPFW_TP_SRC;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_TP_DST ))
		mask |=  rofl::openflow10::OFPFW_TP_DST;

	return mask;
}
	
uint64_t of10_translation_utils::get_out_port(uint16_t port){
	switch(port){
		case rofl::openflow10::OFPP_MAX:
			return OF1X_PORT_MAX;
			break;
		case rofl::openflow10::OFPP_IN_PORT:
			return OF1X_PORT_IN_PORT;
			break;
		case rofl::openflow10::OFPP_TABLE:
			return OF1X_PORT_TABLE;
			break;
		case rofl::openflow10::OFPP_NORMAL:
			return OF1X_PORT_NORMAL;
			break;
		case rofl::openflow10::OFPP_FLOOD:
			return OF1X_PORT_FLOOD;
			break;
		case rofl::openflow10::OFPP_ALL:
			return OF1X_PORT_ALL;
			break;
		case rofl::openflow10::OFPP_CONTROLLER:
			return OF1X_PORT_CONTROLLER;
			break;
		case rofl::openflow10::OFPP_LOCAL:
			return OF1X_PORT_LOCAL;
			break;
		case rofl::openflow10::OFPP_NONE:
			return OF1X_PORT_ANY; //NOTE needed for deleting flows
			break;
		default:
			return port;
			break;
	}
}

uint32_t of10_translation_utils::get_out_port_reverse(uint64_t port){
	switch(port){
		case OF1X_PORT_MAX:
			return rofl::openflow10::OFPP_MAX;
			break;
		case OF1X_PORT_IN_PORT:
			return rofl::openflow10::OFPP_IN_PORT;
			break;
		case OF1X_PORT_TABLE:
			return rofl::openflow10::OFPP_TABLE;
			break;
		case OF1X_PORT_NORMAL:
			return rofl::openflow10::OFPP_NORMAL;
			break;
		case OF1X_PORT_FLOOD:
			return rofl::openflow10::OFPP_FLOOD;
			break;
		case OF1X_PORT_ALL:
			return rofl::openflow10::OFPP_ALL;
			break;
		case OF1X_PORT_CONTROLLER:
			return rofl::openflow10::OFPP_CONTROLLER;
			break;
		case OF1X_PORT_LOCAL:
			return rofl::openflow10::OFPP_LOCAL;
			break;
		case OF1X_PORT_ANY:
			return rofl::openflow10::OFPP_NONE; //NOTE needed for deleting flows
			break;
		default:
			return port;
			break;
	}
}
