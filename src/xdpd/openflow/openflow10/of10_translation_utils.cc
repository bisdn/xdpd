/*
 * of10_translation_utils.cc
 *
 *  Created on: 06.09.2013
 *      Author: andreas
 */


#include "of10_translation_utils.h"

#include <inttypes.h>

using namespace rofl;

//Byte masks
#define OF12_AT_6_BYTE_MASK 0x0000FFFFFFFFFFFF
#define OF12_AT_4_BYTE_MASK 0x00000000FFFFFFFF
#define OF12_AT_2_BYTE_MASK 0x000000000000FFFF
#define OF12_AT_1_BYTE_MASK 0x00000000000000FF

//Non-multiple of byte masks
#define OF12_AT_20_BITS_MASK 0x00000000000FFFFF
#define OF12_AT_13_BITS_MASK 0x0000000000001FFF
#define OF12_AT_6_BITS_MASK 0x000000000000003F
#define OF12_AT_3_BITS_MASK 0x0000000000000007
#define OF12_AT_2_BITS_MASK 0x0000000000000003


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
* Maps a of12_flow_entry from an OF1.2 Header
*/
of12_flow_entry_t*
of10_translation_utils::of12_map_flow_entry(
		cofctl *ctl,
		cofmsg_flow_mod *msg,
		openflow_switch* sw)
{

	of12_flow_entry_t *entry = of12_init_flow_entry(NULL, NULL, msg->get_flags() & OFPFF_SEND_FLOW_REM);

	if(!entry)
		throw eFlowModUnknown();

	// store flow-mod fields in of12_flow_entry
	entry->priority 		= msg->get_priority();
	entry->cookie 			= msg->get_cookie();
	entry->cookie_mask 		= 0xFFFFFFFFFFFFFFFF;
	entry->timer_info.idle_timeout	= msg->get_idle_timeout(); // these timers must be activated some time, when?
	entry->timer_info.hard_timeout	= msg->get_hard_timeout();

	try{
		// extract OXM fields from pack and store them in of12_flow_entry
		of10_map_flow_entry_matches(ctl, msg->get_match(), sw, entry);
	}catch(...){
		of12_destroy_flow_entry(entry);
		throw eFlowModUnknown();
	}

	// for OpenFlow 1.0 => add a single instruction APPLY-ACTIONS to instruction group
	of12_action_group_t *apply_actions = of12_init_action_group(0);

	try{
		of12_map_flow_entry_actions(ctl, sw, msg->get_actions(), apply_actions, /*of12_write_actions_t*/0);
	}catch(...){
		of12_destroy_flow_entry(entry);
		throw eFlowModUnknown();
	}

	of12_add_instruction_to_group(
			&(entry->inst_grp),
			OF12_IT_APPLY_ACTIONS,
			(of12_action_group_t*)apply_actions,
			NULL,
			/*go_to_table*/0);

	return entry;
}



/**
* Maps a of12_match from an OF1.0 Header
*/
void
of10_translation_utils::of10_map_flow_entry_matches(
		cofctl *ctl,
		cofmatch const& ofmatch,
		openflow_switch* sw,
		of12_flow_entry *entry)
{
	try {
		of12_match_t *match = of12_init_port_in_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_in_port());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	// no in_phy_port in OF1.0

	try {
		uint64_t maddr = ofmatch.get_eth_dst_addr().get_mac();;
		uint64_t mmask = rofl::cmacaddr("FF:FF:FF:FF:FF:FF").get_mac(); // no mask in OF1.0

		of12_match_t *match = of12_init_eth_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								maddr,
								mmask);

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t maddr = ofmatch.get_eth_src_addr().get_mac();
		uint64_t mmask = rofl::cmacaddr("FF:FF:FF:FF:FF:FF").get_mac(); // no mask in OF1.0

		of12_match_t *match = of12_init_eth_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								maddr,
								mmask);

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_eth_type_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_eth_type());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_vlan_vid_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_vlan_vid_value(),
								0xFFFF); // no mask in OF1.0

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_vlan_pcp_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_vlan_pcp());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_ip_dscp_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ip_dscp());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	// no ip_ecn in OF1.0

	try {
		of12_match_t *match = of12_init_ip_proto_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ip_proto());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		caddress value(ofmatch.get_ipv4_src_value());
		caddress mask (ofmatch.get_ipv4_src_mask());

		of12_match_t *match = of12_init_ip4_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								be32toh(value.ca_s4addr->sin_addr.s_addr),
								be32toh( mask.ca_s4addr->sin_addr.s_addr));

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		caddress value(ofmatch.get_ipv4_dst_value());
		caddress mask (ofmatch.get_ipv4_dst_mask());

		of12_match_t *match = of12_init_ip4_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								be32toh(value.ca_s4addr->sin_addr.s_addr),
								be32toh( mask.ca_s4addr->sin_addr.s_addr));

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_tcp_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_src());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_tcp_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_dst());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_udp_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_udp_src());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	// no UDP-dst/src in OF1.0 (TCP-dst/src is used for all transport protocols)

	// no SCTP-dst/src in OF1.0

	/* FIXME: OF 1.0 allows the following situation:
	 * all matches wildcard, except:
	 * - transport protocol source and destination
	 *
	 * this flow-mod matches TCP, UDP and ICMP packets
	 * This behaviour cannot be reproduced in OF1.2.
	 *
	 * We make an assumption here:
	 * The control plane designed has to define the
	 * ip_proto field as a match, thus indicating which
	 * protocol he assumes and we use ip_proto to
	 * set the correct match valies in structure of12_match.
	 *
	 */

	try {
		uint8_t ip_proto = ofmatch.get_ip_proto();

		of12_match_t *match = (of12_match_t*)0;

		switch (ip_proto) {
		case ftcpframe::TCP_IP_PROTO: {
			match = of12_init_tcp_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_src());
		} break;
		case fudpframe::UDP_IP_PROTO: {
			match = of12_init_udp_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_src());
		} break;
		case ficmpv4frame::ICMPV4_IP_PROTO: {
			match = of12_init_icmpv4_type_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_src());
		} break;
		}

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		uint8_t ip_proto = ofmatch.get_ip_proto();

		of12_match_t *match = (of12_match_t*)0;

		switch (ip_proto) {
		case ftcpframe::TCP_IP_PROTO: {
			match = of12_init_tcp_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_dst());
		} break;
		case fudpframe::UDP_IP_PROTO: {
			match = of12_init_udp_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_dst());
		} break;
		case ficmpv4frame::ICMPV4_IP_PROTO: {
			match = of12_init_icmpv4_code_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_dst());
		} break;
		}

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	/* FIXME: same situation as above: OF1.0 defines
	 * IP-src/dst and IP-proto with ambiguities, as the same fields
	 * may be used for multiple protocols.
	 *
	 * Same strategy as above: control plane has to specify,
	 * which protocol is assumed by setting the ethernet type
	 * match field.
	 *
	 */

	try {
		uint16_t ethtype = ofmatch.get_eth_type();

		of12_match_t *match = (of12_match_t*)0;

		switch (ethtype) {
		case fipv4frame::IPV4_ETHER: {
			match = of12_init_ip_proto_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ip_proto());
		} break;
		case farpv4frame::ARPV4_ETHER: {
			match = of12_init_arp_opcode_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ip_proto());
		} break;
		}

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	// no ARP-SHA in OF1.0

	try {
		caddress value(ofmatch.get_ipv4_src_value());
		caddress mask (ofmatch.get_ipv4_src_mask());

		of12_match_t *match = of12_init_arp_spa_match(
								/*prev*/NULL,
								/*next*/NULL,
								be32toh(value.ca_s4addr->sin_addr.s_addr),
								be32toh( mask.ca_s4addr->sin_addr.s_addr));

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	// no ARP_THA in OF1.0

	try {
		caddress value(ofmatch.get_ipv4_dst_value());
		caddress mask (ofmatch.get_ipv4_dst_mask());

		of12_match_t *match = of12_init_arp_tpa_match(
								/*prev*/NULL,
								/*next*/NULL,
								be32toh(value.ca_s4addr->sin_addr.s_addr),
								be32toh( mask.ca_s4addr->sin_addr.s_addr));

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	// no IPv6 support in OF1.0

	// no ICMPv6 support in OF1.0

	// no MPLS support in OF1.0
}



/**
* Maps a of12_action from an OF1.2 Header
*/
void
of10_translation_utils::of12_map_flow_entry_actions(
		cofctl *ctl,
		openflow_switch* sw,
		cofaclist& actions,
		of12_action_group_t *apply_actions,
		of12_write_actions_t *write_actions)
{
	for (cofaclist::iterator
			jt = actions.begin(); jt != actions.end(); ++jt)
	{
		cofaction& raction = (*jt);

		of12_packet_action_t *action = NULL;

		switch (raction.get_type()) {
		case OFPAT_OUTPUT:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_OUTPUT, be32toh(raction.oac_output->port), NULL, NULL);
			break;
		case OFP10AT_SET_VLAN_VID:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_VLAN_VID, be16toh(raction.oac_vlanvid->vlan_vid), NULL, NULL);
			break;
		case OFP10AT_SET_VLAN_PCP:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_VLAN_PCP, be16toh(raction.oac_vlanpcp->vlan_pcp), NULL, NULL);
			break;
		case OFP10AT_STRIP_VLAN:
			// FIXME: we need the ethertype here for the OF1.2 pipeline, but this field does not exist in OF1.0 and must be drawn from the removed VLAN tag
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_POP_VLAN, be16toh(0), NULL, NULL);
			break;
		case OFP10AT_SET_DL_SRC: {
			cmacaddr mac(raction.oac_dladdr->dl_addr, 6);
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_ETH_SRC, mac.get_mac(), NULL, NULL);
			} break;
		case OFP10AT_SET_DL_DST: {
			cmacaddr mac(raction.oac_dladdr->dl_addr, 6);
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_ETH_DST, mac.get_mac(), NULL, NULL);
			} break;
		case OFP10AT_SET_NW_SRC:
#if 0
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_ARP_SPA, oxm.uint32_value(), NULL, NULL);
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_IPV4_SRC, oxm.uint32_value(), NULL, NULL);
#endif
			break;
		case OFP10AT_SET_NW_DST:
			break;
		case OFP10AT_SET_NW_TOS:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_IP_DSCP, raction.oac_nwtos->nw_tos, NULL, NULL);
			break;
		case OFP10AT_SET_TP_SRC:
			break;
		case OFP10AT_SET_TP_DST:
			break;
		case OFP10AT_ENQUEUE:
			// FIXME: what to do with the port field in struct ofp10_action_enqueue?
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_QUEUE, be32toh(raction.oac_enqueue->queue_id), NULL, NULL);
			break;
		}

		if (NULL != apply_actions)
		{
			of12_push_packet_action_to_group(apply_actions, action);
		}

		if (NULL != write_actions)
		{
			of12_set_packet_action_on_write_actions(write_actions, action);
		}
	}

}



/*
* Maps a of12_action TO an OF1.2 Header
*/
void
of10_translation_utils::of12_map_reverse_flow_entry_matches(
		of12_match_t* m,
		cofmatch& match)
{
	while (NULL != m)
	{
		switch (m->type) {
		case OF12_MATCH_IN_PORT:
			match.set_in_port(((utern32_t*)(m->value))->value);
			break;
		case OF12_MATCH_IN_PHY_PORT:
			match.set_in_phy_port(((utern32_t*)(m->value))->value);
			break;
		case OF12_MATCH_METADATA:
			match.set_metadata(((utern64_t*)(m->value))->value);
			break;
		case OF12_MATCH_ETH_DST:
		{
			cmacaddr maddr(((utern64_t*)m->value)->value);
			cmacaddr mmask(((utern64_t*)m->value)->mask);
			match.set_eth_dst(maddr, mmask);
		}
			break;
		case OF12_MATCH_ETH_SRC:
		{
			cmacaddr maddr(((utern64_t*)m->value)->value);
			cmacaddr mmask(((utern64_t*)m->value)->mask);
			match.set_eth_src(maddr, mmask);
		}
			break;
		case OF12_MATCH_ETH_TYPE:
			match.set_eth_type(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_VLAN_VID:
			match.set_vlan_vid(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_VLAN_PCP:
			match.set_vlan_pcp(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_ARP_OP:
			match.set_arp_opcode(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_ARP_SHA:
		{
			cmacaddr maddr(((utern64_t*)m->value)->value);
			cmacaddr mmask(((utern64_t*)m->value)->mask);
			match.set_arp_sha(maddr, mmask);
		}
			break;
		case OF12_MATCH_ARP_SPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = htonl(((utern32_t*)(m->value))->value);
			match.set_arp_spa(addr);
		}
			break;
		case OF12_MATCH_ARP_THA:
		{
			cmacaddr maddr(((utern64_t*)m->value)->value);
			cmacaddr mmask(((utern64_t*)m->value)->mask);
			match.set_arp_tha(maddr, mmask);
		}
			break;
		case OF12_MATCH_ARP_TPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = htonl(((utern32_t*)(m->value))->value);
			match.set_arp_tpa(addr);
		}
			break;
		case OF12_MATCH_IP_DSCP:
			match.set_ip_dscp(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_IP_ECN:
			match.set_ip_ecn(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_IP_PROTO:
			match.set_ip_proto(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_IPV4_SRC:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = htonl(((utern32_t*)(m->value))->value);
			match.set_ipv4_src(addr);

		}
			break;
		case OF12_MATCH_IPV4_DST:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = htonl(((utern32_t*)(m->value))->value);
			match.set_ipv4_dst(addr);
		}
			break;
		case OF12_MATCH_TCP_SRC:
			match.set_tcp_src(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_TCP_DST:
			match.set_tcp_dst(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_UDP_SRC:
			match.set_udp_src(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_UDP_DST:
			match.set_udp_dst(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_SCTP_SRC:
			match.set_sctp_src(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_SCTP_DST:
			match.set_sctp_dst(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_ICMPV4_TYPE:
			match.set_icmpv4_type(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_ICMPV4_CODE:
			match.set_icmpv4_code(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_IPV6_SRC:
			throw eNotImplemented(std::string("of10_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_SRC"));
		case OF12_MATCH_IPV6_DST:
			throw eNotImplemented(std::string("of10_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_DST"));
		case OF12_MATCH_IPV6_FLABEL:
			throw eNotImplemented(std::string("of10_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_FLABEL"));
		case OF12_MATCH_ICMPV6_TYPE:
			throw eNotImplemented(std::string("of10_translation_utils::of12_map_reverse_flow_entry_matches() IPCMPV6_TYPE"));
		case OF12_MATCH_ICMPV6_CODE:
			throw eNotImplemented(std::string("of10_translation_utils::of12_map_reverse_flow_entry_matches() IPCMPV6_CODE"));
		case OF12_MATCH_IPV6_ND_TARGET:
			throw eNotImplemented(std::string("of10_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_ND_TARGET"));
		case OF12_MATCH_IPV6_ND_SLL:
			throw eNotImplemented(std::string("of10_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_ND_SLL"));
		case OF12_MATCH_IPV6_ND_TLL:
			throw eNotImplemented(std::string("of10_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_ND_TLL"));
		case OF12_MATCH_MPLS_LABEL:
			match.set_mpls_label(((utern32_t*)(m->value))->value);
			break;
		case OF12_MATCH_MPLS_TC:
			match.set_mpls_tc(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_PPPOE_CODE:
			match.insert(coxmatch_ofx_pppoe_code(((utern8_t*)(m->value))->value));
			break;
		case OF12_MATCH_PPPOE_TYPE:
			match.insert(coxmatch_ofx_pppoe_type(((utern8_t*)(m->value))->value));
			break;
		case OF12_MATCH_PPPOE_SID:
			match.insert(coxmatch_ofx_pppoe_sid(((utern16_t*)(m->value))->value));
			break;
		case OF12_MATCH_PPP_PROT:
			match.insert(coxmatch_ofx_ppp_prot(((utern16_t*)(m->value))->value));
			break;
		case OF12_MATCH_GTP_MSG_TYPE:
			match.insert(coxmatch_ofx_gtp_msg_type(((utern8_t*)(m->value))->value));
			break;
		case OF12_MATCH_GTP_TEID:
			match.insert(coxmatch_ofx_gtp_teid(((utern32_t*)(m->value))->value));
			break;
		default:
			break;
		}


		m = m->next;
	}
}



/**
*
*/
void
of10_translation_utils::of12_map_reverse_flow_entry_instructions(
		of12_instruction_group_t* group,
		cofinlist& instructions)
{
	for (unsigned int i = 0; i < (sizeof(group->instructions) / sizeof(of12_instruction_t)); i++) {
		if (OF12_IT_NO_INSTRUCTION == group->instructions[i].type)
			continue;
		cofinst instruction;
		of12_map_reverse_flow_entry_instruction(&(group->instructions[i]), instruction);
		instructions.next() = instruction;
	}
}


void
of10_translation_utils::of12_map_reverse_flow_entry_instruction(
		of12_instruction_t* inst,
		cofinst& instruction)
{
	switch (inst->type) {
	case OF12_IT_APPLY_ACTIONS: {
		instruction = cofinst_apply_actions();
		for (of12_packet_action_t *of12_action = inst->apply_actions->head; of12_action != NULL; of12_action = of12_action->next) {
			if (OF12_AT_NO_ACTION == of12_action->type)
				continue;
			cofaction action;
			of12_map_reverse_flow_entry_action(of12_action, action);
			instruction.actions.next() = action;
		}
	} break;
	case OF12_IT_CLEAR_ACTIONS: {
		instruction = cofinst_clear_actions();
	} break;
	case OF12_IT_WRITE_ACTIONS: {
		instruction = cofinst_write_actions();
		for (unsigned int i = 0; i < OF12_IT_GOTO_TABLE; i++) {
			if (OF12_AT_NO_ACTION == inst->write_actions->write_actions[i].type)
				continue;
			cofaction action;
			of12_map_reverse_flow_entry_action(&(inst->write_actions->write_actions[i]), action);
			instruction.actions.next() = action;
		}
	} break;
	case OF12_IT_WRITE_METADATA:
	case OF12_IT_EXPERIMENTER: {
		// TODO: both are marked TODO in of12_pipeline
	} break;
	case OF12_IT_GOTO_TABLE: {
		instruction = cofinst_goto_table(inst->go_to_table);
	} break;
	default: {
		// do nothing
	} break;
	}
}


void
of10_translation_utils::of12_map_reverse_flow_entry_action(
		of12_packet_action_t* of12_action,
		cofaction& action)
{
	/*
	 * FIXME: add masks for those fields defining masked values in the specification
	 */


	switch (of12_action->type) {
	case OF12_AT_NO_ACTION: {
		// do nothing
	} break;
	case OF12_AT_COPY_TTL_IN: {
		action = cofaction_copy_ttl_in();
	} break;
	case OF12_AT_POP_VLAN: {
		action = cofaction_pop_vlan();
	} break;
	case OF12_AT_POP_MPLS: {
		action = cofaction_pop_mpls((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK));
	} break;
	case OF12_AT_POP_PPPOE: {
		action = cofaction_pop_pppoe((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK));
	} break;
	case OF12_AT_PUSH_PPPOE: {
		action = cofaction_push_pppoe((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK));
	} break;
	case OF12_AT_PUSH_MPLS: {
		action = cofaction_push_mpls((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK));
	} break;
	case OF12_AT_PUSH_VLAN: {
		action = cofaction_push_vlan((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK));
	} break;
	case OF12_AT_COPY_TTL_OUT: {
		action = cofaction_copy_ttl_out();
	} break;
	case OF12_AT_DEC_NW_TTL: {
		action = cofaction_dec_nw_ttl();
	} break;
	case OF12_AT_DEC_MPLS_TTL: {
		action = cofaction_dec_mpls_ttl();
	} break;
	case OF12_AT_SET_MPLS_TTL: {
		action = cofaction_set_mpls_ttl((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK));
	} break;
	case OF12_AT_SET_NW_TTL: {
		action = cofaction_set_nw_ttl((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK));
	} break;
	case OF12_AT_SET_QUEUE: {
		action = cofaction_set_queue((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK));
	} break;
	//case OF12_AT_SET_FIELD_METADATA:
	case OF12_AT_SET_FIELD_ETH_DST: {
		cmacaddr maddr(of12_action->field);
		action = cofaction_set_field(coxmatch_ofb_eth_dst(maddr));
	} break;
	case OF12_AT_SET_FIELD_ETH_SRC: {
		cmacaddr maddr(of12_action->field);
		action = cofaction_set_field(coxmatch_ofb_eth_src(maddr));
	} break;
	case OF12_AT_SET_FIELD_ETH_TYPE: {
		action = cofaction_set_field(coxmatch_ofb_eth_type((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_VLAN_VID: {
		action = cofaction_set_field(coxmatch_ofb_vlan_vid((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_VLAN_PCP: {
		action = cofaction_set_field(coxmatch_ofb_vlan_pcp((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_ARP_OPCODE: {
		action = cofaction_set_field(coxmatch_ofb_arp_opcode((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_ARP_SHA: {
		cmacaddr maddr(of12_action->field);
		action = cofaction_set_field(coxmatch_ofb_arp_sha(maddr));
	} break;
	case OF12_AT_SET_FIELD_ARP_SPA: {
		action = cofaction_set_field(coxmatch_ofb_arp_spa((uint32_t)(of12_action->field & OF12_AT_4_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_ARP_THA: {
		cmacaddr maddr(of12_action->field);
		action = cofaction_set_field(coxmatch_ofb_arp_tha(maddr));
	} break;
	case OF12_AT_SET_FIELD_ARP_TPA: {
		action = cofaction_set_field(coxmatch_ofb_arp_tpa((uint32_t)(of12_action->field & OF12_AT_4_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_IP_DSCP: {
		action = cofaction_set_field(coxmatch_ofb_ip_dscp((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_IP_ECN: {
		action = cofaction_set_field(coxmatch_ofb_ip_ecn((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_IP_PROTO: {
		action = cofaction_set_field(coxmatch_ofb_ip_proto((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_IPV4_SRC: {
		action = cofaction_set_field(coxmatch_ofb_ipv4_src((uint32_t)(of12_action->field & OF12_AT_4_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_IPV4_DST: {
		action = cofaction_set_field(coxmatch_ofb_ipv4_dst((uint32_t)(of12_action->field & OF12_AT_4_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_TCP_SRC: {
		action = cofaction_set_field(coxmatch_ofb_tcp_src((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_TCP_DST: {
		action = cofaction_set_field(coxmatch_ofb_tcp_dst((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_UDP_SRC: {
		action = cofaction_set_field(coxmatch_ofb_udp_src((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_UDP_DST: {
		action = cofaction_set_field(coxmatch_ofb_udp_dst((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_ICMPV4_TYPE: {
		action = cofaction_set_field(coxmatch_ofb_icmpv4_type((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_ICMPV4_CODE: {
		action = cofaction_set_field(coxmatch_ofb_icmpv4_code((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_MPLS_LABEL: {
		action = cofaction_set_field(coxmatch_ofb_mpls_label((uint32_t)(of12_action->field & OF12_AT_4_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_MPLS_TC: {
		action = cofaction_set_field(coxmatch_ofb_mpls_tc((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_PPPOE_CODE: {
		action = cofaction_set_field(coxmatch_ofx_pppoe_code((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_PPPOE_TYPE: {
		action = cofaction_set_field(coxmatch_ofx_pppoe_type((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_PPPOE_SID: {
		action = cofaction_set_field(coxmatch_ofx_pppoe_sid((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_PPP_PROT: {
		action = cofaction_set_field(coxmatch_ofx_ppp_prot((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_GTP_MSG_TYPE: {
		action = cofaction_set_field(coxmatch_ofx_gtp_msg_type((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_GTP_TEID: {
		action = cofaction_set_field(coxmatch_ofx_gtp_teid((uint32_t)(of12_action->field & OF12_AT_4_BYTE_MASK)));
	} break;
	case OF12_AT_GROUP: {
		action = cofaction_group((uint32_t)(of12_action->field & OF12_AT_4_BYTE_MASK));
	} break;
	case OF12_AT_EXPERIMENTER: {
		// TODO
	} break;
	case OF12_AT_OUTPUT: {
		action = cofaction_output((uint32_t)(of12_action->field & OF12_AT_4_BYTE_MASK));
	} break;
	default: {
		// do nothing
	} break;
	}
}


/*
* Maps packet actions to cofmatches
*/

void of10_translation_utils::of12_map_reverse_packet_matches(of12_packet_matches_t* packet_matches, cofmatch& match){
	if(packet_matches->port_in)
		match.set_in_port(packet_matches->port_in);
	if(packet_matches->phy_port_in)
		match.set_in_phy_port(packet_matches->phy_port_in);
	//if(packet_matches->metadata)
	//	match.set_metadata(packet_matches->metadata);
	if(packet_matches->eth_dst){
		cmacaddr maddr(packet_matches->eth_dst);
		cmacaddr mmask(0x0000FFFFFFFFFFFF);
		match.set_eth_dst(maddr, mmask);
	}
	if(packet_matches->eth_src){
		cmacaddr maddr(packet_matches->eth_src);
		cmacaddr mmask(0x0000FFFFFFFFFFFF);
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
		addr.ca_s4addr->sin_addr.s_addr = htonl(packet_matches->arp_spa);
		match.set_arp_spa(addr);
	}
	if(packet_matches->arp_tha)
		match.set_arp_tha(cmacaddr(packet_matches->arp_tha));
	if(packet_matches->arp_tpa) {
		caddress addr(AF_INET, "0.0.0.0");
		addr.ca_s4addr->sin_addr.s_addr = htonl(packet_matches->arp_tpa);
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
			addr.ca_s4addr->sin_addr.s_addr = htonl(packet_matches->ipv4_src);
			match.set_ipv4_src(addr);

	}
	if(packet_matches->ipv4_dst){
		caddress addr(AF_INET, "0.0.0.0");
		addr.ca_s4addr->sin_addr.s_addr = htonl(packet_matches->ipv4_dst);
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

