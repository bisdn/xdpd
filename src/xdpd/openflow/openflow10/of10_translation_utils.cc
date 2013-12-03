/*
 * of10_translation_utils.cc
 *
 *  Created on: 06.09.2013
 *      Author: andreas
 */


#define __STDC_CONSTANT_MACROS 1 // todo enable globally
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
		cofctl *ctl,
		cofmsg_flow_mod *msg,
		openflow_switch* sw)
{

	of1x_flow_entry_t *entry = of1x_init_flow_entry(NULL, NULL, msg->get_flags() & OFPFF_SEND_FLOW_REM);

	if(!entry)
		throw eFlowModUnknown();

	// store flow-mod fields in of1x_flow_entry
	entry->priority 		= msg->get_priority();
	entry->cookie 			= msg->get_cookie();
	entry->cookie_mask 		= 0xFFFFFFFFFFFFFFFFULL;
	entry->timer_info.idle_timeout	= msg->get_idle_timeout(); // these timers must be activated some time, when?
	entry->timer_info.hard_timeout	= msg->get_hard_timeout();

	try{
		// extract OXM fields from pack and store them in of1x_flow_entry
		of10_map_flow_entry_matches(ctl, msg->get_match(), sw, entry);
	}catch(...){
		of1x_destroy_flow_entry(entry);
		throw eFlowModUnknown();
	}

	// for OpenFlow 1.0 => add a single instruction APPLY-ACTIONS to instruction group
	of1x_action_group_t *apply_actions = of1x_init_action_group(0);

	try{
		of1x_map_flow_entry_actions(ctl, sw, msg->get_actions(), apply_actions, /*of1x_write_actions_t*/0);
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
		cofctl *ctl,
		cofmatch const& ofmatch,
		openflow_switch* sw,
		of1x_flow_entry *entry)
{
	try {
		of1x_match_t *match = of1x_init_port_in_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_in_port());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	// no in_phy_port in OF1.0

	try {
		uint64_t maddr = ofmatch.get_eth_dst_addr().get_mac();;
		uint64_t mmask = rofl::cmacaddr("FF:FF:FF:FF:FF:FF").get_mac(); // no mask in OF1.0

		of1x_match_t *match = of1x_init_eth_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								maddr,
								mmask);

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t maddr = ofmatch.get_eth_src_addr().get_mac();
		uint64_t mmask = rofl::cmacaddr("FF:FF:FF:FF:FF:FF").get_mac(); // no mask in OF1.0

		of1x_match_t *match = of1x_init_eth_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								maddr,
								mmask);

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_eth_type_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_eth_type());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_vlan_vid_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_vlan_vid_value()|OF1X_VLAN_PRESENT_MASK,
								0x1FFF); // no mask in OF1.0

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_vlan_pcp_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_vlan_pcp());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	//NW TOS
	try {
		of1x_match_t *match = of1x_init_ip_dscp_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ip_dscp()>>2);

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	//NW PROTO 
	try {
		of1x_match_t *match = of1x_init_nw_proto_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_nw_proto());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	//NW SRC
	try {

		of1x_match_t *match = NULL; 
		caddress value(ofmatch.get_nw_src_value());
		caddress mask(ofmatch.get_nw_src_mask());
	
		if(mask.ca_s4addr->sin_addr.s_addr){	
			match = of1x_init_nw_src_match(	/*prev*/NULL,
							/*next*/NULL,
							be32toh(value.ca_s4addr->sin_addr.s_addr),
							be32toh(mask.ca_s4addr->sin_addr.s_addr));

			of1x_add_match_to_entry(entry, match);
		}

	} catch (eOFmatchNotFound& e) {}

	//NW DST 
	try {

		of1x_match_t *match = NULL; 
		caddress value(ofmatch.get_nw_dst_value());
		caddress mask(ofmatch.get_nw_dst_mask());
		
		if(mask.ca_s4addr->sin_addr.s_addr){	
			match = of1x_init_nw_dst_match(	/*prev*/NULL,
							/*next*/NULL,
							be32toh(value.ca_s4addr->sin_addr.s_addr),
							be32toh(mask.ca_s4addr->sin_addr.s_addr));

			of1x_add_match_to_entry(entry, match);
		}
	} catch (eOFmatchNotFound& e) {}

	//TP SRC
	try {
		of1x_match_t *match = of1x_init_tp_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tp_src());
		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	//TP DST
	try {
		of1x_match_t *match = of1x_init_tp_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tp_dst());
		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

}



/**
* Maps a of1x_action from an OF1.2 Header
*/

//FIXME TODO XXX: cofaction should have appropiate getters and setters instead of having  to access internals of the class!
void
of10_translation_utils::of1x_map_flow_entry_actions(
		cofctl *ctl,
		openflow_switch* sw,
		cofaclist& actions,
		of1x_action_group_t *apply_actions,
		of1x_write_actions_t *write_actions)
{
	for (cofaclist::iterator
			jt = actions.begin(); jt != actions.end(); ++jt)
	{
		cofaction& raction = (*jt);

		of1x_packet_action_t *action = NULL;
		wrap_uint_t field;
		memset(&field,0,sizeof(wrap_uint_t));

		switch (raction.get_type()) {
		case OFP10AT_OUTPUT:
			//Translate special values to of1x
			field.u64 = get_out_port(be16toh(raction.oac_10output->port));
			action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, NULL, NULL);
			break;
		case OFP10AT_SET_VLAN_VID:
			field.u64 = be16toh(raction.oac_10vlanvid->vlan_vid);
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_VID, field, NULL, NULL);
			break;
		case OFP10AT_SET_VLAN_PCP:
			field.u64 = be16toh(raction.oac_10vlanpcp->vlan_pcp)>>8;
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_PCP, field, NULL, NULL);
			break;
		case OFP10AT_STRIP_VLAN:
			action = of1x_init_packet_action( OF1X_AT_POP_VLAN, field, NULL, NULL); 
			break;
		case OFP10AT_SET_DL_SRC: {
			cmacaddr mac(raction.oac_10dladdr->dl_addr, 6);
			field.u64 = mac.get_mac();
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_SRC, field, NULL, NULL);
			} break;
		case OFP10AT_SET_DL_DST: {
			cmacaddr mac(raction.oac_10dladdr->dl_addr, 6);
			field.u64 = mac.get_mac();
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_DST, field, NULL, NULL);
			} break;
		case OFP10AT_SET_NW_SRC:
			field.u32 = be32toh(raction.oac_10nwaddr->nw_addr);
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_NW_SRC, field, NULL, NULL);
			break;
		case OFP10AT_SET_NW_DST:
			field.u32 = be32toh(raction.oac_10nwaddr->nw_addr);
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_NW_DST, field, NULL, NULL);
			break;
		case OFP10AT_SET_NW_TOS:
			field.u64 = raction.oac_10nwtos->nw_tos>>2;
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_DSCP, field, NULL, NULL);
			break;
		case OFP10AT_SET_TP_SRC:
			field.u64 = be16toh(raction.oac_10tpport->tp_port);
			action = of1x_init_packet_action(OF1X_AT_SET_FIELD_TP_SRC, field, NULL, NULL);
			break;
		case OFP10AT_SET_TP_DST:
			field.u64 = be16toh(raction.oac_10tpport->tp_port);
			action = of1x_init_packet_action(OF1X_AT_SET_FIELD_TP_DST, field, NULL, NULL);
			break;
		case OFP10AT_ENQUEUE:
			field.u64 = be32toh(raction.oac_10enqueue->queue_id);
			action = of1x_init_packet_action( OF1X_AT_SET_QUEUE, field, NULL, NULL);
			if (NULL != apply_actions) of1x_push_packet_action_to_group(apply_actions, action);
			field.u64 = get_out_port(be16toh(raction.oac_10enqueue->port));
			action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, NULL, NULL);
			break;
		}

		if (NULL != apply_actions)
		{
			of1x_push_packet_action_to_group(apply_actions, action);
		}
	}

}



/*
* Maps a of1x_action TO an OF1.2 Header
*/
void
of10_translation_utils::of1x_map_reverse_flow_entry_matches(
		of1x_match_t* m,
		cofmatch& match)
{
	bool has_vlan=false;
	while (NULL != m)
	{
		switch (m->type) {
		case OF1X_MATCH_IN_PORT:
			match.set_in_port(m->value->value.u32);
			break;
		case OF1X_MATCH_ETH_DST:
		{
			cmacaddr maddr(m->value->value.u64);
			cmacaddr mmask(m->value->mask.u64);
			match.set_eth_dst(maddr, mmask);
		}
			break;
		case OF1X_MATCH_ETH_SRC:
		{
			cmacaddr maddr(m->value->value.u64);
			cmacaddr mmask(m->value->mask.u64);
			match.set_eth_src(maddr, mmask);
		}
			break;
		case OF1X_MATCH_ETH_TYPE:
			match.set_eth_type(m->value->value.u16);
			break;
		case OF1X_MATCH_VLAN_VID:
			has_vlan = true;
			match.set_vlan_vid(m->value->value.u16&OF1X_VLAN_ID_MASK);
			break;
		case OF1X_MATCH_VLAN_PCP:
			match.set_vlan_pcp(m->value->value.u8);
			break;
		case OF1X_MATCH_ARP_OP:
			match.set_arp_opcode(m->value->value.u16);
			break;
		case OF1X_MATCH_ARP_SHA:
		{
			cmacaddr maddr(m->value->value.u64);
			cmacaddr mmask(m->value->mask.u64);
			match.set_arp_sha(maddr, mmask);
		}
			break;
		case OF1X_MATCH_ARP_SPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(m->value->value.u32);
			match.set_arp_spa(addr);
		}
			break;
		case OF1X_MATCH_ARP_THA:
		{
			cmacaddr maddr(m->value->value.u64);
			cmacaddr mmask(m->value->mask.u64);
			match.set_arp_tha(maddr, mmask);
		}
			break;
		case OF1X_MATCH_ARP_TPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(m->value->value.u32);
			match.set_arp_tpa(addr);
		}
			break;
		case OF1X_MATCH_IP_DSCP:
			match.set_ip_dscp((m->value->value.u8));
			break;
		case OF1X_MATCH_IP_ECN:
			match.set_ip_ecn(m->value->value.u8);
			break;
		case OF1X_MATCH_NW_PROTO:
			match.set_nw_proto(m->value->value.u8);
			break;
		case OF1X_MATCH_NW_SRC:
		{
			caddress addr(AF_INET, "0.0.0.0");
			caddress mask(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(m->value->value.u32);
			mask.set_ipv4_addr(m->value->mask.u32);
			match.set_nw_src(addr, mask);

		}
			break;
		case OF1X_MATCH_NW_DST:
		{
			caddress addr(AF_INET, "0.0.0.0");
			caddress mask(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(m->value->value.u32);
			mask.set_ipv4_addr(m->value->mask.u32);
			match.set_nw_dst(addr, mask);
		}
			break;
		case OF1X_MATCH_TP_SRC:
			match.set_tp_src(m->value->value.u16);
			break;
		case OF1X_MATCH_TP_DST:
			match.set_tp_dst(m->value->value.u16);
			break;
		case OF1X_MATCH_MPLS_LABEL:
			match.set_mpls_label(m->value->value.u32);
			break;
		case OF1X_MATCH_MPLS_TC:
			match.set_mpls_tc(m->value->value.u8);
			break;
		case OF1X_MATCH_PPPOE_CODE:
			match.insert(coxmatch_ofx_pppoe_code(m->value->value.u8));
			break;
		case OF1X_MATCH_PPPOE_TYPE:
			match.insert(coxmatch_ofx_pppoe_type(m->value->value.u8));
			break;
		case OF1X_MATCH_PPPOE_SID:
			match.insert(coxmatch_ofx_pppoe_sid(m->value->value.u16));
			break;
		case OF1X_MATCH_PPP_PROT:
			match.insert(coxmatch_ofx_ppp_prot(m->value->value.u16));
			break;
		case OF1X_MATCH_GTP_MSG_TYPE:
			match.insert(coxmatch_ofx_gtp_msg_type(m->value->value.u8));
			break;
		case OF1X_MATCH_GTP_TEID:
			match.insert(coxmatch_ofx_gtp_teid(m->value->value.u32));
			break;
		default:
			break;
		}


		m = m->next;
	}

	//In 1.0 if there is no VLAN OFP10_VLAN_NONE has to be set...
	if(!has_vlan)
		match.set_vlan_vid(OFP10_VLAN_NONE);
}



/**
*
*/
void
of10_translation_utils::of1x_map_reverse_flow_entry_instructions(
		of1x_instruction_group_t* group,
		cofinlist& instructions,
		uint16_t pipeline_miss_send_len)
{
	for (unsigned int i = 0; i < (sizeof(group->instructions) / sizeof(of1x_instruction_t)); i++) {
		if (OF1X_IT_NO_INSTRUCTION == group->instructions[i].type)
			continue;
		cofinst instruction(OFP10_VERSION);;
		of1x_map_reverse_flow_entry_instruction(&(group->instructions[i]), instruction, pipeline_miss_send_len);
		instructions.next() = instruction;
	}
}


void
of10_translation_utils::of1x_map_reverse_flow_entry_instruction(
		of1x_instruction_t* inst,
		cofinst& instruction,
		uint16_t pipeline_miss_send_len)
{
	switch (inst->type) {
	case OF1X_IT_APPLY_ACTIONS: {
		instruction = cofinst_apply_actions(OFP10_VERSION);
		for (of1x_packet_action_t *of1x_action = inst->apply_actions->head; of1x_action != NULL; of1x_action = of1x_action->next) {
			if (OF1X_AT_NO_ACTION == of1x_action->type)
				continue;
			cofaction action(OFP10_VERSION);
			of1x_map_reverse_flow_entry_action(of1x_action, action, pipeline_miss_send_len);
			instruction.actions.next() = action;
				
			//Skip next action if action is set-queue (SET-QUEUE-OUTPUT)
			if(of1x_action->type == OF1X_AT_SET_QUEUE){
				if(of1x_action->next && !of1x_action->next->next)
					break;
				else
					of1x_action = of1x_action->next; //Skip output
			}
		}
	} break;
	case OF1X_IT_CLEAR_ACTIONS: {
		instruction = cofinst_clear_actions(OFP10_VERSION);
	} break;
	case OF1X_IT_WRITE_ACTIONS: {
		instruction = cofinst_write_actions(OFP10_VERSION);
		for (unsigned int i = 0; i < OF1X_IT_GOTO_TABLE; i++) {
			if (OF1X_AT_NO_ACTION == inst->write_actions->write_actions[i].type)
				continue;
			cofaction action(OFP10_VERSION);
			of1x_map_reverse_flow_entry_action(&(inst->write_actions->write_actions[i]), action, pipeline_miss_send_len);
			instruction.actions.next() = action;
		}
	} break;
	case OF1X_IT_WRITE_METADATA:
	case OF1X_IT_EXPERIMENTER: {
		// TODO: both are marked TODO in of1x_pipeline
	} break;
	case OF1X_IT_GOTO_TABLE: {
		instruction = cofinst_goto_table(OFP10_VERSION, inst->go_to_table);
	} break;
	default: {
		// do nothing
	} break;
	}
}


void
of10_translation_utils::of1x_map_reverse_flow_entry_action(
		of1x_packet_action_t* of1x_action,
		cofaction& action,
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
		//action = cofaction_pop_vlan(OFP10_VERSION);
		action = cofaction_strip_vlan(OFP10_VERSION);
	} break;
	case OF1X_AT_PUSH_VLAN: {
		action = cofaction_push_vlan(OFP10_VERSION, of1x_action->field.u16);
	} break;
	case OF1X_AT_SET_FIELD_ETH_DST: {
		action = cofaction_set_dl_dst(OFP10_VERSION, cmacaddr(of1x_action->field.u64));
	} break;
	case OF1X_AT_SET_FIELD_ETH_SRC: {
		action = cofaction_set_dl_src(OFP10_VERSION, cmacaddr(of1x_action->field.u64));
	} break;
	case OF1X_AT_SET_FIELD_VLAN_VID: {
		action = cofaction_set_vlan_vid(OFP10_VERSION, of1x_action->field.u16&OF1X_VLAN_ID_MASK);
	} break;
	case OF1X_AT_SET_FIELD_VLAN_PCP: {
		action = cofaction_set_vlan_pcp(OFP10_VERSION, of1x_action->field.u8);
	} break;
	case OF1X_AT_SET_FIELD_IP_DSCP: {
		action = cofaction_set_nw_tos(OFP10_VERSION, of1x_action->field.u8<<2);
	} break;
	case OF1X_AT_SET_FIELD_NW_SRC: {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(of1x_action->field.u32);
		action = cofaction_set_nw_src(OFP10_VERSION, addr);
	} break;
	case OF1X_AT_SET_FIELD_NW_DST: {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(of1x_action->field.u32);
		action = cofaction_set_nw_dst(OFP10_VERSION, addr);
	} break;
	case OF1X_AT_SET_FIELD_TP_SRC: {
		action = cofaction_set_tp_src(OFP10_VERSION, of1x_action->field.u16);
	} break;
	case OF1X_AT_SET_FIELD_TP_DST: {
		action = cofaction_set_tp_dst(OFP10_VERSION, of1x_action->field.u16);
	} break;
	case OF1X_AT_EXPERIMENTER: {
		// TODO
	} break;
	case OF1X_AT_SET_QUEUE: {
		//Right after queue we must have an output
		if(of1x_action->next)
			action = cofaction_enqueue(OFP10_VERSION, get_out_port_reverse(of1x_action->next->field.u64), of1x_action->field.u32);
		else{
			assert(0);
		}
	}break;
	case OF1X_AT_OUTPUT: {
		//Setting max_len to the switch max_len (we do not support per action max_len)
		action = cofaction_output(OFP10_VERSION, get_out_port_reverse(of1x_action->field.u64), pipeline_miss_send_len);
	} break;
	default: {
		// do nothing
	} break;
	}
	
}


/*
* Maps packet actions to cofmatches
*/

void of10_translation_utils::of1x_map_reverse_packet_matches(of1x_packet_matches_t* packet_matches, cofmatch& match){
	if(packet_matches->port_in)
		match.set_in_port(packet_matches->port_in);
	if(packet_matches->eth_dst){
		cmacaddr maddr(packet_matches->eth_dst);
		cmacaddr mmask(0x0000FFFFFFFFFFFFULL);
		match.set_eth_dst(maddr, mmask);
	}
	if(packet_matches->eth_src){
		cmacaddr maddr(packet_matches->eth_src);
		cmacaddr mmask(0x0000FFFFFFFFFFFFULL);
		match.set_eth_src(maddr, mmask);
	}
	if(packet_matches->eth_type)
		match.set_eth_type(packet_matches->eth_type);
	if(packet_matches->vlan_vid)
		match.set_vlan_vid(packet_matches->vlan_vid);
	if(packet_matches->vlan_pcp)
		match.set_vlan_pcp(packet_matches->vlan_pcp);
	if(packet_matches->arp_opcode)
		match.set_arp_opcode(packet_matches->arp_opcode);
	if(packet_matches->arp_sha)
		match.set_arp_sha(cmacaddr(packet_matches->arp_sha));
	if(packet_matches->arp_spa) {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(packet_matches->arp_spa);
		match.set_arp_spa(addr);
	}
	if(packet_matches->arp_tha)
		match.set_arp_tha(cmacaddr(packet_matches->arp_tha));
	if(packet_matches->arp_tpa) {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(packet_matches->arp_tpa);
		match.set_arp_tpa(addr);
	}
	if(packet_matches->ip_dscp)
		match.set_ip_dscp(packet_matches->ip_dscp);
	if(packet_matches->ip_ecn)
		match.set_ip_ecn(packet_matches->ip_ecn);
	if(packet_matches->ip_proto)
		match.set_ip_proto(packet_matches->ip_proto);
	if(packet_matches->ipv4_src){
			caddress addr(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(packet_matches->ipv4_src);
			match.set_ipv4_src(addr);

	}
	if(packet_matches->ipv4_dst){
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(packet_matches->ipv4_dst);
		match.set_ipv4_dst(addr);
	}
	if(packet_matches->tcp_src)
		match.set_tcp_src(packet_matches->tcp_src);
	if(packet_matches->tcp_dst)
		match.set_tcp_dst(packet_matches->tcp_dst);
	if(packet_matches->udp_src)
		match.set_udp_src(packet_matches->udp_src);
	if(packet_matches->udp_dst)
		match.set_udp_dst(packet_matches->udp_dst);
	if(packet_matches->icmpv4_type)
		match.set_icmpv4_type(packet_matches->icmpv4_type);
	if(packet_matches->icmpv4_code)
		match.set_icmpv4_code(packet_matches->icmpv4_code);

	//TODO IPv6
	if(packet_matches->mpls_label)
		match.set_mpls_label(packet_matches->mpls_label);
	if(packet_matches->mpls_tc)
		match.set_mpls_tc(packet_matches->mpls_tc);
	if(packet_matches->pppoe_code)
		match.insert(coxmatch_ofx_pppoe_code(packet_matches->pppoe_code));
	if(packet_matches->pppoe_type)
		match.insert(coxmatch_ofx_pppoe_type(packet_matches->pppoe_type));
	if(packet_matches->pppoe_sid)
		match.insert(coxmatch_ofx_pppoe_sid(packet_matches->pppoe_sid));
	if(packet_matches->ppp_proto)
		match.insert(coxmatch_ofx_ppp_prot(packet_matches->ppp_proto));
	if(packet_matches->gtp_msg_type)
		match.insert(coxmatch_ofx_gtp_msg_type(packet_matches->gtp_msg_type));
	if(packet_matches->gtp_teid)
		match.insert(coxmatch_ofx_gtp_teid(packet_matches->gtp_teid));
}

uint32_t of10_translation_utils::get_supported_actions(of1x_switch_t *lsw){
	uint32_t mask = 0;
	
	of1x_flow_table_config_t config = lsw->pipeline->tables[0].config;
		
	if (config.apply_actions&(1UL<<OF12PAT_OUTPUT))
		mask |= 1 << OFP10AT_OUTPUT;
	
	if (config.match&(1UL<<OF1X_MATCH_VLAN_VID))
		mask |= 1 << OFP10AT_SET_VLAN_VID;
	
	if (config.match&(1UL<<OF1X_MATCH_VLAN_PCP))
		mask |= 1 << OFP10AT_SET_VLAN_PCP;
	
	if (config.apply_actions&(1UL<<OF12PAT_POP_VLAN))
		mask |= 1 << OFP10AT_STRIP_VLAN;
	
	if (config.match&(1UL<<OF1X_MATCH_ETH_SRC))
		mask |= 1 << OFP10AT_SET_DL_SRC;
	
	if (config.match&(1UL<<OF1X_MATCH_ETH_DST))
		mask |= 1 << OFP10AT_SET_DL_DST;
	
	if (config.match&(1UL<<OF1X_MATCH_IPV4_SRC))
		mask |= 1 << OFP10AT_SET_NW_SRC;
	
	if (config.match&(1UL<<OF1X_MATCH_IPV4_DST))
		mask |= 1 << OFP10AT_SET_NW_DST;
	
	if (config.match&(1UL<<OF1X_MATCH_IP_DSCP))
		mask |= 1 << OFP10AT_SET_NW_TOS;
	
	if (config.match&(UINT64_C(1)<<OF1X_MATCH_TP_SRC))
		mask |= 1 << OFP10AT_SET_TP_SRC;
	
	if (config.match&(UINT64_C(1)<<OF1X_MATCH_TP_DST))
		mask |= 1 << OFP10AT_SET_TP_DST;
	
	if (config.apply_actions&(1UL<<OF12PAT_SET_QUEUE))
		mask |= 1 << OFP10AT_ENQUEUE;
		
	return mask;
}

uint64_t of10_translation_utils::get_out_port(uint16_t port){
	switch(port){
		case OFPP10_MAX:
			return OF1X_PORT_MAX;
			break;
		case OFPP10_IN_PORT:
			return OF1X_PORT_IN_PORT;
			break;
		case OFPP10_TABLE:
			return OF1X_PORT_TABLE;
			break;
		case OFPP10_NORMAL:
			return OF1X_PORT_NORMAL;
			break;
		case OFPP10_FLOOD:
			return OF1X_PORT_FLOOD;
			break;
		case OFPP10_ALL:
			return OF1X_PORT_ALL;
			break;
		case OFPP10_CONTROLLER:
			return OF1X_PORT_CONTROLLER;
			break;
		case OFPP10_LOCAL:
			return OF1X_PORT_LOCAL;
			break;
		case OFPP10_NONE:
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
			return OFPP10_MAX;
			break;
		case OF1X_PORT_IN_PORT:
			return OFPP10_IN_PORT;
			break;
		case OF1X_PORT_TABLE:
			return OFPP10_TABLE;
			break;
		case OF1X_PORT_NORMAL:
			return OFPP10_NORMAL;
			break;
		case OF1X_PORT_FLOOD:
			return OFPP10_FLOOD;
			break;
		case OF1X_PORT_ALL:
			return OFPP10_ALL;
			break;
		case OF1X_PORT_CONTROLLER:
			return OFPP10_CONTROLLER;
			break;
		case OF1X_PORT_LOCAL:
			return OFPP10_LOCAL;
			break;
		case OF1X_PORT_ANY:
			return OFPP10_NONE; //NOTE needed for deleting flows
			break;
		default:
			return port;
			break;
	}
}
