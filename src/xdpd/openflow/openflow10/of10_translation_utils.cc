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

#include "../endianness_translation_utils.h"

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

	of1x_flow_entry_t *entry = of1x_init_flow_entry(msg->get_flags() & openflow10::OFPFF_SEND_FLOW_REM);

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
		crofctl *ctl,
		rofl::openflow::cofmatch const& ofmatch,
		openflow_switch* sw,
		of1x_flow_entry *entry)
{
	try {
		of1x_match_t *match = of1x_init_port_in_match(ofmatch.get_in_port());

		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	// no in_phy_port in OF1.0

	try {
		uint64_t maddr = ofmatch.get_eth_dst_addr().get_mac();
		MACTOBE(maddr);
		uint64_t mmask = rofl::cmacaddr("FF:FF:FF:FF:FF:FF").get_mac(); // no mask in OF1.0
		MACTOBE(mmask);
		of1x_match_t *match = of1x_init_eth_dst_match(maddr,mmask);

		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	try {
		uint64_t maddr = ofmatch.get_eth_src_addr().get_mac();
		MACTOBE(maddr);
		uint64_t mmask = rofl::cmacaddr("FF:FF:FF:FF:FF:FF").get_mac(); // no mask in OF1.0
		MACTOBE(mmask);
		of1x_match_t *match = of1x_init_eth_src_match(maddr,mmask);

		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_eth_type_match(htobe16(ofmatch.get_eth_type()));

		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	try {
		bool vlan_present=true;
		uint16_t value = ofmatch.get_vlan_vid_value();
		/*
		 * clear bit 12 in value, even if this does not exist in OF10,
		 * as the pipeline may get interprete this bit otherwise
		 */
		uint16_t vid = htobe16(value & ~openflow::OFPVID_PRESENT);

		of1x_match_t *match = of1x_init_vlan_vid_match(vid, OF1X_VLAN_ID_MASK, vlan_present); // no mask in OF1.0

		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_vlan_pcp_match(ofmatch.get_vlan_pcp());

		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//NW TOS
	try {
		of1x_match_t *match = of1x_init_ip_dscp_match(ofmatch.get_nw_tos()>>2);

		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//NW PROTO 
	try {
		of1x_match_t *match = of1x_init_nw_proto_match(ofmatch.get_nw_proto());
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//NW SRC
	try {

		of1x_match_t *match = NULL; 
		caddress value(ofmatch.get_nw_src_value());
		caddress mask(ofmatch.get_nw_src_mask());
		if(mask.ca_s4addr->sin_addr.s_addr){	
			match = of1x_init_nw_src_match(
							value.ca_s4addr->sin_addr.s_addr,
							mask.ca_s4addr->sin_addr.s_addr);
			of1x_add_match_to_entry(entry, match);
		}

	} catch (rofl::openflow::eOxmNotFound& e) {}

	//NW DST 
	try {

		of1x_match_t *match = NULL; 
		caddress value(ofmatch.get_nw_dst_value());
		caddress mask(ofmatch.get_nw_dst_mask());
		if(mask.ca_s4addr->sin_addr.s_addr){	
			match = of1x_init_nw_dst_match(
							value.ca_s4addr->sin_addr.s_addr,
							mask.ca_s4addr->sin_addr.s_addr);

			of1x_add_match_to_entry(entry, match);
		}
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//TP SRC
	try {
		of1x_match_t *match = of1x_init_tp_src_match(htobe16(ofmatch.get_tp_src()));
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

	//TP DST
	try {
		of1x_match_t *match = of1x_init_tp_dst_match(htobe16(ofmatch.get_tp_dst()));
		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOxmNotFound& e) {}

}



/**
* Maps a of1x_action from an OF1.2 Header
*/

//FIXME TODO XXX: cofaction should have appropiate getters and setters instead of having  to access internals of the class!
void
of10_translation_utils::of1x_map_flow_entry_actions(
		crofctl *ctl,
		openflow_switch* sw,
		rofl::openflow::cofactions& actions,
		of1x_action_group_t *apply_actions,
		of1x_write_actions_t *write_actions)
{
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
			field.u64 = get_out_port(be16toh(raction.oac_10output->port));
			action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, be16toh(raction.oac_10output->max_len));
			break;
		case rofl::openflow10::OFPAT_SET_VLAN_VID:
			field.u64 = raction.oac_10vlanvid->vlan_vid;
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_VID, field, 0x0);
			break;
		case rofl::openflow10::OFPAT_SET_VLAN_PCP:
			field.u64 = be16toh(raction.oac_10vlanpcp->vlan_pcp)>>8;
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_PCP, field, 0x0);
			break;
		case rofl::openflow10::OFPAT_STRIP_VLAN:
			action = of1x_init_packet_action( OF1X_AT_POP_VLAN, field, 0x0); 
			break;
		case rofl::openflow10::OFPAT_SET_DL_SRC: {
			cmacaddr mac(raction.oac_10dladdr->dl_addr, 6);
			field.u64 = mac.get_mac();
			MACTOBE(field.u64);
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_SRC, field, 0x0);
			} break;
		case rofl::openflow10::OFPAT_SET_DL_DST: {
			cmacaddr mac(raction.oac_10dladdr->dl_addr, 6);
			field.u64 = mac.get_mac();
			MACTOBE(field.u64);
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_DST, field, 0x0);
			} break;
		case rofl::openflow10::OFPAT_SET_NW_SRC:
			field.u32 = raction.oac_10nwaddr->nw_addr;
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_NW_SRC, field, 0x0);
			break;
		case rofl::openflow10::OFPAT_SET_NW_DST:
			field.u32 = raction.oac_10nwaddr->nw_addr;
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_NW_DST, field, 0x0);
			break;
		case rofl::openflow10::OFPAT_SET_NW_TOS:
			field.u64 = raction.oac_10nwtos->nw_tos>>2;
			action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_DSCP, field, 0x0);
			break;
		case rofl::openflow10::OFPAT_SET_TP_SRC:
			field.u64 = raction.oac_10tpport->tp_port;
			action = of1x_init_packet_action(OF1X_AT_SET_FIELD_TP_SRC, field, 0x0);
			break;
		case rofl::openflow10::OFPAT_SET_TP_DST:
			field.u64 = raction.oac_10tpport->tp_port;
			action = of1x_init_packet_action(OF1X_AT_SET_FIELD_TP_DST, field, 0x0);
			break;
		case rofl::openflow10::OFPAT_ENQUEUE:
			field.u64 = be32toh(raction.oac_10enqueue->queue_id);
			action = of1x_init_packet_action( OF1X_AT_SET_QUEUE, field, 0x0);
			if (NULL != apply_actions) of1x_push_packet_action_to_group(apply_actions, action);
			field.u64 = get_out_port(be16toh(raction.oac_10enqueue->port));
			action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, 0x0);
			break;
		}

		if (NULL != apply_actions)
		{
			of1x_push_packet_action_to_group(apply_actions, action);
		}
	}

}



/*
* Maps a of1x_action TO an OF1.0 Header
*/
void
of10_translation_utils::of1x_map_reverse_flow_entry_matches(
		of1x_match_t* m,
		rofl::openflow::cofmatch& match)
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
			uint64_t mac = m->value->value.u64;
			BETOHMAC(mac);
			uint64_t msk = m->value->mask.u64;
			BETOHMAC(msk);
			match.set_eth_dst(cmacaddr(mac), cmacaddr(msk));
		}
			break;
		case OF1X_MATCH_ETH_SRC:
		{
			uint64_t mac = m->value->value.u64;
			BETOHMAC(mac);
			uint64_t msk = m->value->mask.u64;
			BETOHMAC(msk);
			match.set_eth_src(cmacaddr(mac), cmacaddr(msk));
		}
			break;
		case OF1X_MATCH_ETH_TYPE:
			match.set_eth_type(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_VLAN_VID:
			has_vlan = true;
			match.set_vlan_vid(be16toh(m->value->value.u16&OF1X_VLAN_ID_MASK));
			break;
		case OF1X_MATCH_VLAN_PCP:
			match.set_vlan_pcp(m->value->value.u8);
			break;
		case OF1X_MATCH_ARP_OP:
			match.set_nw_proto(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_ARP_SPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(be32toh(m->value->value.u32));
			match.set_nw_src(addr);	// TODO: the same for ARP request and ARP reply?
		}
			break;
		case OF1X_MATCH_ARP_TPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(be32toh(m->value->value.u32));
			match.set_arp_tpa(addr);
			match.set_nw_dst(addr);	// TODO: the same for ARP request and ARP reply?
		}
			break;
		case OF1X_MATCH_IP_DSCP:
			match.set_nw_tos((m->value->value.u8));
			break;
		case OF1X_MATCH_NW_PROTO:
			match.set_nw_proto(m->value->value.u8);
			break;
		case OF1X_MATCH_NW_SRC:
		{
			caddress addr(AF_INET, "0.0.0.0");
			caddress mask(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(be32toh(m->value->value.u32));
			mask.set_ipv4_addr(be32toh(m->value->mask.u32));
			match.set_nw_src(addr, mask);

		}
			break;
		case OF1X_MATCH_NW_DST:
		{
			caddress addr(AF_INET, "0.0.0.0");
			caddress mask(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(be32toh(m->value->value.u32));
			mask.set_ipv4_addr(be32toh(m->value->mask.u32));
			match.set_nw_dst(addr, mask);
		}
			break;
		case OF1X_MATCH_TP_SRC:
			match.set_tp_src(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_TP_DST:
			match.set_tp_dst(be16toh(m->value->value.u16));
			break;
		default:
			break;
		}


		m = m->next;
	}

	//In 1.0 if there is no VLAN OFP10_VLAN_NONE has to be set...
	if(!has_vlan)
		match.set_vlan_untagged();
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

		for (of1x_packet_action_t *of1x_action = group->instructions[i].apply_actions->head; of1x_action != NULL; of1x_action = of1x_action->next) {
			if (OF1X_AT_NO_ACTION == of1x_action->type)
				continue;
			rofl::openflow::cofaction action(OFP10_VERSION);
			of1x_map_reverse_flow_entry_action(of1x_action, action, pipeline_miss_send_len);
			actions.append_action(action);
			
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
		rofl::openflow::cofaction& action,
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
		//action = rofl::openflow::cofaction_pop_vlan(OFP10_VERSION);
		action = rofl::openflow::cofaction_strip_vlan(OFP10_VERSION);
	} break;
	case OF1X_AT_PUSH_VLAN: {
		action = rofl::openflow::cofaction_push_vlan(OFP10_VERSION, of1x_action->field.u16);
	} break;
	case OF1X_AT_SET_FIELD_ETH_DST: {
		action = rofl::openflow::cofaction_set_dl_dst(OFP10_VERSION, cmacaddr(of1x_action->field.u64));
	} break;
	case OF1X_AT_SET_FIELD_ETH_SRC: {
		action = rofl::openflow::cofaction_set_dl_src(OFP10_VERSION, cmacaddr(of1x_action->field.u64));
	} break;
	case OF1X_AT_SET_FIELD_VLAN_VID: {
		action = rofl::openflow::cofaction_set_vlan_vid(OFP10_VERSION, of1x_action->field.u16&OF1X_VLAN_ID_MASK);
	} break;
	case OF1X_AT_SET_FIELD_VLAN_PCP: {
		action = rofl::openflow::cofaction_set_vlan_pcp(OFP10_VERSION, of1x_action->field.u8);
	} break;
	case OF1X_AT_SET_FIELD_IP_DSCP: {
		action = rofl::openflow::cofaction_set_nw_tos(OFP10_VERSION, of1x_action->field.u8<<2);
	} break;
	case OF1X_AT_SET_FIELD_NW_SRC: {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(be32toh(of1x_action->field.u32));
		action = rofl::openflow::cofaction_set_nw_src(OFP10_VERSION, addr);
	} break;
	case OF1X_AT_SET_FIELD_NW_DST: {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(be32toh(of1x_action->field.u32));
		action = rofl::openflow::cofaction_set_nw_dst(OFP10_VERSION, addr);
	} break;
	case OF1X_AT_SET_FIELD_TP_SRC: {
		action = rofl::openflow::cofaction_set_tp_src(OFP10_VERSION, be16toh(of1x_action->field.u16));
	} break;
	case OF1X_AT_SET_FIELD_TP_DST: {
		action = rofl::openflow::cofaction_set_tp_dst(OFP10_VERSION, be16toh(of1x_action->field.u16));
	} break;
	case OF1X_AT_EXPERIMENTER: {
		// TODO
	} break;
	case OF1X_AT_SET_QUEUE: {
		//Right after queue we must have an output
		if(of1x_action->next)
			action = rofl::openflow::cofaction_enqueue(OFP10_VERSION, get_out_port_reverse(of1x_action->next->field.u64), of1x_action->field.u32);
		else{
			assert(0);
		}
	}break;
	case OF1X_AT_OUTPUT: {
		//Setting max_len to the switch max_len (we do not support per action max_len)
		action = rofl::openflow::cofaction_output(OFP10_VERSION, get_out_port_reverse(of1x_action->field.u64), of1x_action->send_len);
	} break;
	default: {
		// do nothing
	} break;
	}
	
}


/*
* Maps packet actions to cofmatches
*/

void of10_translation_utils::of1x_map_reverse_packet_matches(packet_matches_t* packet_matches, rofl::openflow::cofmatch& match){
	if(packet_matches->port_in)
		match.set_in_port(packet_matches->port_in);
	if(packet_matches->eth_dst){
		uint64_t mac = packet_matches->eth_dst;
		BETOHMAC(mac);
		uint64_t msk = 0x0000FFFFFFFFFFFFULL;
		BETOHMAC(msk);
		match.set_eth_dst(cmacaddr(mac), cmacaddr(msk));
	}
	if(packet_matches->eth_src){
		uint64_t mac = packet_matches->eth_src;
		BETOHMAC(mac);
		uint64_t msk = 0x0000FFFFFFFFFFFFULL;
		BETOHMAC(msk);
		match.set_eth_src(cmacaddr(mac), cmacaddr(msk));
	}
	if(packet_matches->eth_type)
		match.set_eth_type(be16toh(packet_matches->eth_type));
	if(packet_matches->vlan_vid)
		match.set_vlan_vid(be16toh(packet_matches->vlan_vid));
	if(packet_matches->vlan_pcp)
		match.set_vlan_pcp(packet_matches->vlan_pcp);
	if(packet_matches->arp_opcode)
		match.set_nw_proto(be16toh(packet_matches->arp_opcode));
	if(packet_matches->arp_spa) {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(be32toh(packet_matches->arp_spa));
		match.set_nw_src(addr);
	}
	if(packet_matches->arp_tpa) {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(be32toh(packet_matches->arp_tpa));
		match.set_nw_dst(addr);
	}
	if(packet_matches->ip_dscp)
		match.set_ip_dscp(packet_matches->ip_dscp);
	if(packet_matches->ip_ecn)
		match.set_ip_ecn(packet_matches->ip_ecn);
	if(packet_matches->ip_proto)
		match.set_ip_proto(packet_matches->ip_proto);
	if(packet_matches->ipv4_src){
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(be32toh(packet_matches->ipv4_src));
		match.set_nw_src(addr);
	}
	if(packet_matches->ipv4_dst){
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(be32toh(packet_matches->ipv4_dst));
		match.set_nw_dst(addr);
	}
	if(packet_matches->tcp_src)
		match.set_tp_src(be16toh(packet_matches->tcp_src));
	if(packet_matches->tcp_dst)
		match.set_tp_dst(be16toh(packet_matches->tcp_dst));
	if(packet_matches->udp_src)
		match.set_tp_src(be16toh(packet_matches->udp_src));
	if(packet_matches->udp_dst)
		match.set_tp_dst(be16toh(packet_matches->udp_dst));
	if(packet_matches->icmpv4_type)
		match.set_tp_src(packet_matches->icmpv4_type);
	if(packet_matches->icmpv4_code)
		match.set_tp_dst(packet_matches->icmpv4_code);
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

	//TODO
#if 0	
	of1x_flow_table_config_t* config = &lsw->pipeline.tables[0].config;

	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_ETH_DST)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_ETH_SRC)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_ETH_TYPE)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_VLAN_VID)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_VLAN_PCP)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_ARP_OP)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_ARP_SPA)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_ARP_TPA)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_IP_DSCP)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_IP_ECN)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_NW_PROTO)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_NW_SRC)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_NW_DST)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_TP_SRC)
		mask |= 1 <<  ;
	if( bitmap128_is_bit_set(&config->match, OF1X_MATCH_TP_DST)
		mask |= 1 <<  ;
#endif
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
