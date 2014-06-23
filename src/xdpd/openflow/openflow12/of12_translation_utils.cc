#include "of12_translation_utils.h"

#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>

#include <rofl/common/utils/c_logger.h>

using namespace xdpd;

/*
* Port utils
*/
#define HAS_CAPABILITY(bitmap,cap) (bitmap&cap) > 0
uint32_t of12_translation_utils::get_port_speed_kb(port_features_t features){

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
of12_translation_utils::of12_map_flow_entry(
		crofctl *ctl, 
		rofl::openflow::cofmsg_flow_mod *msg,
		openflow_switch* sw)
{

	of1x_flow_entry_t *entry = of1x_init_flow_entry(msg->get_flags() & openflow12::OFPFF_SEND_FLOW_REM);

	if(!entry)
		throw eFlowModUnknown();

	// store flow-mod fields in of1x_flow_entry
	entry->priority 		= msg->get_priority();
	entry->cookie 			= msg->get_cookie();
	entry->cookie_mask 		= msg->get_cookie_mask();
	entry->timer_info.idle_timeout	= msg->get_idle_timeout(); // these timers must be activated some time, when?
	entry->timer_info.hard_timeout	= msg->get_hard_timeout();

	try{
		// extract OXM fields from pack and store them in of1x_flow_entry
		of12_map_flow_entry_matches(ctl, msg->get_match(), sw, entry);
	}catch(...){
		of1x_destroy_flow_entry(entry);	
		throw eFlowModUnknown();
	}
	

	/*
	 * Inst-Apply-Actions
	 */
	if (msg->get_instructions().has_inst_apply_actions()) {
		of1x_action_group_t *apply_actions = of1x_init_action_group(0);
		try{
			of12_map_flow_entry_actions(ctl, sw,
					msg->get_instructions().get_inst_apply_actions().get_actions(),
					apply_actions, /*of1x_write_actions_t*/0);

			of1x_add_instruction_to_group(
						&(entry->inst_grp),
						OF1X_IT_APPLY_ACTIONS,
						(of1x_action_group_t*)apply_actions,
						NULL,
						NULL,
						/*go_to_table*/0);
		}catch(...){
			of1x_destroy_flow_entry(entry);
			of1x_destroy_action_group(apply_actions);
			throw eFlowModUnknown();
		}
	}

	/*
	 * Inst-Clear-Actions
	 */
	if (msg->get_instructions().has_inst_clear_actions()) {
		of1x_add_instruction_to_group(
				&(entry->inst_grp),
				OF1X_IT_CLEAR_ACTIONS,
				NULL,
				NULL,
				NULL,
				/*go_to_table*/0);
	}


	/*
	 * Inst-Experimenter
	 */
	if (msg->get_instructions().has_inst_experimenter()) {
		of1x_add_instruction_to_group(
					&(entry->inst_grp),
					OF1X_IT_EXPERIMENTER,
					NULL,
					NULL,
					NULL,
					/*go_to_table*/0);
	}


	/*
	 * Inst-Goto-Table
	 */
	if (msg->get_instructions().has_inst_goto_table()) {
		of1x_add_instruction_to_group(
				&(entry->inst_grp),
				OF1X_IT_GOTO_TABLE,
				NULL,
				NULL,
				NULL,
				/*go_to_table*/msg->get_instructions().get_inst_goto_table().get_table_id());
	}


	/*
	 * Inst-Write-Actions
	 */
	if (msg->get_instructions().has_inst_write_actions()) {
		of1x_write_actions_t *write_actions = of1x_init_write_actions();
		try{
			of12_map_flow_entry_actions(ctl, sw,
					msg->get_instructions().get_inst_write_actions().get_actions(),
					/*of1x_action_group_t*/0, write_actions);

			of1x_add_instruction_to_group(
					&(entry->inst_grp),
					OF1X_IT_WRITE_ACTIONS,
					NULL,
					(of1x_write_actions_t*)write_actions,
					NULL,
					/*go_to_table*/0);
		}catch(...){
			of1x_destroy_flow_entry(entry);
			throw eFlowModUnknown();
		}
	}


	/*
	 * Inst-Write-Metadata
	 */
	if (msg->get_instructions().has_inst_write_metadata()) {
		of1x_write_metadata_t metadata = {
				msg->get_instructions().get_inst_write_metadata().get_metadata(),
				msg->get_instructions().get_inst_write_metadata().get_metadata_mask()
		};

		of1x_add_instruction_to_group(
				&(entry->inst_grp),
				OF1X_IT_WRITE_METADATA,
				NULL,
				NULL,
				&metadata,
				/*go_to_table*/0);
	}


	return entry;
}



/**
* Maps a of1x_match from an OF1.2 Header
*/
void
of12_translation_utils::of12_map_flow_entry_matches(
		crofctl *ctl,
		rofl::openflow::cofmatch const& ofmatch,
		openflow_switch* sw, 
		of1x_flow_entry *entry)
{
	of1x_match_t *match;

	try {
		match = of1x_init_port_in_match(ofmatch.get_in_port());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_port_in_phy_match(ofmatch.get_in_phy_port());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_metadata_match(ofmatch.get_metadata(), ofmatch.get_metadata_mask());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		uint64_t maddr = ofmatch.get_eth_src_addr().get_mac();
		uint64_t mmask = ofmatch.get_eth_src_mask().get_mac();
		match = of1x_init_eth_src_match(maddr, mmask);
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		uint64_t maddr = ofmatch.get_eth_dst_addr().get_mac();
		uint64_t mmask = ofmatch.get_eth_dst_mask().get_mac();
		match = of1x_init_eth_dst_match(maddr, mmask);
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_eth_type_match(ofmatch.get_eth_type());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		uint16_t value = ofmatch.get_vlan_vid_value();
		uint16_t mask  = ofmatch.get_vlan_vid_mask();
		enum of1x_vlan_present vlan_present=OF1X_MATCH_VLAN_NONE;

		if ((value == rofl::openflow12::OFPVID_PRESENT) && (mask == rofl::openflow12::OFPVID_PRESENT)){
			vlan_present = OF1X_MATCH_VLAN_ANY;
		}else if (value == rofl::openflow12::OFPVID_NONE && mask==0xFFFF){
			vlan_present = OF1X_MATCH_VLAN_NONE;
		}else if (value /*&& mask == 0xFFFF*/){
			vlan_present = OF1X_MATCH_VLAN_SPECIFIC;
		}else{
			//Invalid 
			assert(0);
		}
		
		match = of1x_init_vlan_vid_match(value, mask, vlan_present);
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_vlan_pcp_match(ofmatch.get_vlan_pcp());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_ip_dscp_match(ofmatch.get_ip_dscp());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_ip_ecn_match(ofmatch.get_ip_ecn());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_ip_proto_match(ofmatch.get_ip_proto());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		caddress value(ofmatch.get_ipv4_src_value());
		caddress mask (ofmatch.get_ipv4_src_mask());
		match = of1x_init_ip4_src_match(value.get_ipv4_addr(), mask.get_ipv4_addr());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		caddress value(ofmatch.get_ipv4_dst_value());
		caddress mask (ofmatch.get_ipv4_dst_mask());
		match = of1x_init_ip4_dst_match(value.get_ipv4_addr(), mask.get_ipv4_addr());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_tcp_src_match(ofmatch.get_tcp_src());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_tcp_dst_match(ofmatch.get_tcp_dst());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_udp_src_match(ofmatch.get_udp_src());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_udp_dst_match(ofmatch.get_udp_dst());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		ofmatch.get_sctp_src();
		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() openflow12::OFPXMT_OFB_SCTP_SRC is missing")); // TODO
	} catch(...) {}

	try {
		ofmatch.get_sctp_dst();
		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() openflow12::OFPXMT_OFB_SCTP_DST is missing")); // TODO
	} catch(...) {}

	try {
		match = of1x_init_icmpv4_type_match(ofmatch.get_icmpv4_type());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_icmpv4_code_match(ofmatch.get_icmpv4_code());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_arp_opcode_match(ofmatch.get_arp_opcode());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		uint64_t maddr = ofmatch.get_arp_sha_addr().get_mac();
		uint64_t mmask = ofmatch.get_arp_sha_mask().get_mac();
		match = of1x_init_arp_sha_match(maddr, mmask);
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		caddress value(ofmatch.get_arp_spa_value());
		caddress mask(ofmatch.get_arp_spa_mask());

		match = of1x_init_arp_spa_match(value.get_ipv4_addr(), mask.get_ipv4_addr());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		uint64_t maddr = ofmatch.get_arp_tha_addr().get_mac();
		uint64_t mmask = ofmatch.get_arp_tha_mask().get_mac();
		match = of1x_init_arp_tha_match(maddr, mmask);
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		caddress value(ofmatch.get_arp_tpa_value());
		caddress mask (ofmatch.get_arp_tpa_mask());
		match = of1x_init_arp_tpa_match(value.get_ipv4_addr(), mask.get_ipv4_addr());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		caddress value(ofmatch.get_ipv6_src_value());
		uint128__t val = value.get_ipv6_addr();
		caddress mask (ofmatch.get_ipv6_src_mask());
		uint128__t msk = mask.get_ipv6_addr();
		match = of1x_init_ip6_src_match(val, msk);
		of1x_add_match_to_entry(entry,match);
	} catch(...) {}
	
	try {
		caddress value(ofmatch.get_ipv6_dst_value());
		uint128__t val = value.get_ipv6_addr();
		caddress mask (ofmatch.get_ipv6_dst_mask());
		uint128__t msk = mask.get_ipv6_addr();
		match = of1x_init_ip6_dst_match(val, msk);
		of1x_add_match_to_entry(entry,match);
	} catch(...) {}

	try {
		match = of1x_init_ip6_flabel_match(ofmatch.get_ipv6_flabel());
		of1x_add_match_to_entry(entry,match);
	} catch(...) {}

	try {
		match = of1x_init_icmpv6_type_match(ofmatch.get_icmpv6_type());
		of1x_add_match_to_entry(entry,match);
	} catch(...) {}

	try {
		match = of1x_init_icmpv6_code_match(ofmatch.get_icmpv6_code());
		of1x_add_match_to_entry(entry,match);
	} catch(...) {}

	try {
		caddress value(ofmatch.get_ipv6_nd_target());
		uint128__t val = value.get_ipv6_addr();
		match = of1x_init_ip6_nd_target_match(val);
		of1x_add_match_to_entry(entry,match);
	} catch(...) {}

	try {
		uint64_t mac = ofmatch.get_ipv6_nd_sll().get_mac();
		match = of1x_init_ip6_nd_sll_match(mac);
		of1x_add_match_to_entry(entry,match);
	} catch(...) {}

	try {
		uint64_t mac = ofmatch.get_ipv6_nd_tll().get_mac();
		match = of1x_init_ip6_nd_tll_match(mac);
		of1x_add_match_to_entry(entry,match);
	} catch(...) {}
	try {
		uint32_t label = ofmatch.get_mpls_label();
		match = of1x_init_mpls_label_match(label);
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		match = of1x_init_mpls_tc_match(ofmatch.get_mpls_tc());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	/* Extensions */
	try {
		rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_code oxm_pppoe_code(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::pppoe::OXM_TLV_EXPR_PPPOE_CODE));

		match = of1x_init_pppoe_code_match(oxm_pppoe_code.get_u8value());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_type oxm_pppoe_type(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::pppoe::OXM_TLV_EXPR_PPPOE_TYPE));
		match = of1x_init_pppoe_type_match(oxm_pppoe_type.get_u8value());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_sid oxm_pppoe_sid(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::pppoe::OXM_TLV_EXPR_PPPOE_SID));
		match = of1x_init_pppoe_session_match(oxm_pppoe_sid.get_u16value());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::pppoe::coxmatch_ofx_ppp_prot oxm_ppp_prot(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::pppoe::OXM_TLV_EXPR_PPP_PROT));
		match = of1x_init_ppp_prot_match(oxm_ppp_prot.get_u16value());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_msg_type oxm_gtp_msg_type(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::gtp::OXM_TLV_EXPR_GTP_MSGTYPE));
		match = of1x_init_gtp_msg_type_match(oxm_gtp_msg_type.get_u8value());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_teid oxm_gtp_teid(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::gtp::OXM_TLV_EXPR_GTP_TEID));
		match = of1x_init_gtp_teid_match(oxm_gtp_teid.get_u32value(),oxm_gtp_teid.get_u32mask());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}
	
	/* End of extensions */
}



/**
* Maps a of1x_action from an OF1.2 Header
*/
//FIXME TODO XXX: cofaction should have appropiate getters and setters instead of having  to access internals of the class!
void
of12_translation_utils::of12_map_flow_entry_actions(
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
		case rofl::openflow12::OFPAT_OUTPUT:
			field.u32 = NTOHB32(raction.oac_12output->port);
			action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, NTOHB16(raction.oac_12output->max_len));
			break;
		case rofl::openflow12::OFPAT_COPY_TTL_OUT:
			action = of1x_init_packet_action( OF1X_AT_COPY_TTL_OUT, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_COPY_TTL_IN:
			action = of1x_init_packet_action( OF1X_AT_COPY_TTL_IN, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_SET_MPLS_TTL:
			field.u8 = raction.oac_12mpls_ttl->mpls_ttl;
			action = of1x_init_packet_action( OF1X_AT_SET_MPLS_TTL, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_DEC_MPLS_TTL:
			action = of1x_init_packet_action( OF1X_AT_DEC_MPLS_TTL, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_PUSH_VLAN:
			field.u16 = NTOHB16(raction.oac_oacu.oacu_12push->ethertype);
			action = of1x_init_packet_action( OF1X_AT_PUSH_VLAN, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_POP_VLAN:
			field.u16 = NTOHB16(raction.oac_12push->ethertype);
			action = of1x_init_packet_action( OF1X_AT_POP_VLAN, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_PUSH_MPLS:
			field.u16 = NTOHB16(raction.oac_12push->ethertype);
			action = of1x_init_packet_action( OF1X_AT_PUSH_MPLS, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_POP_MPLS:
			field.u16 = NTOHB16(raction.oac_12push->ethertype);
			action = of1x_init_packet_action( OF1X_AT_POP_MPLS,  field, 0x0);
			break;
		case rofl::openflow12::OFPAT_SET_QUEUE:
			field.u32 = NTOHB32(raction.oac_12set_queue->queue_id);
			action = of1x_init_packet_action( OF1X_AT_SET_QUEUE, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_GROUP:
			field.u32 = NTOHB32(raction.oac_12group->group_id);
			action = of1x_init_packet_action( OF1X_AT_GROUP, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_SET_NW_TTL:
			field.u8 = raction.oac_12nw_ttl->nw_ttl;
			action = of1x_init_packet_action( OF1X_AT_SET_NW_TTL, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_DEC_NW_TTL:
			action = of1x_init_packet_action( OF1X_AT_DEC_NW_TTL, field, 0x0);
			break;
		case rofl::openflow12::OFPAT_SET_FIELD:
		{
			rofl::openflow::coxmatch oxm = raction.get_oxm();

			switch (oxm.get_oxm_class()) {
			case rofl::openflow12::OFPXMC_OPENFLOW_BASIC:
			{
				switch (oxm.get_oxm_field()) {
				case rofl::openflow12::OFPXMT_OFB_ETH_DST:
				{
					field.u64 = oxm.get_u48value().get_mac();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_DST, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_ETH_SRC:
				{
					field.u64 = oxm.get_u48value().get_mac();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_SRC, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_ETH_TYPE:
				{
					field.u16 = oxm.get_u16value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_TYPE, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_ARP_OP:
				{
					field.u16 = oxm.get_u16value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_OPCODE, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_ARP_SHA:
				{
					field.u64 = oxm.get_u48value().get_mac();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_SHA, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_ARP_SPA:
				{
					field.u32 = oxm.get_u32value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_SPA, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_ARP_THA:
				{
					field.u64 = oxm.get_u48value().get_mac();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_THA, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_ARP_TPA:
				{
					field.u32 = oxm.get_u32value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_TPA, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_ICMPV4_CODE:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ICMPV4_CODE, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_ICMPV4_TYPE:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ICMPV4_TYPE, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_IPV4_DST:
				{
					field.u32 = oxm.get_u32value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IPV4_DST, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_IPV4_SRC:
				{
					field.u32 = oxm.get_u32value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IPV4_SRC, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_IP_DSCP:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_DSCP, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_IP_ECN:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_ECN, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_IP_PROTO:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_PROTO, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_MPLS_LABEL:
				{
					field.u32 = oxm.get_u32value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_MPLS_LABEL, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_MPLS_TC:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_MPLS_TC, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_VLAN_VID:
				{
					field.u16 = oxm.get_u16value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_VID, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_VLAN_PCP:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_PCP, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_TCP_DST:
				{
					field.u16 = oxm.get_u16value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_TCP_DST, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_TCP_SRC:
				{
					field.u16 = oxm.get_u16value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_TCP_SRC, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_UDP_DST:
				{
					field.u16 = oxm.get_u16value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_UDP_DST, field, 0x0);
				}
					break;
				case rofl::openflow12::OFPXMT_OFB_UDP_SRC:
				{
					field.u16 = oxm.get_u16value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_UDP_SRC, field, 0x0);
				}
					break;

				case rofl::openflow12::OFPXMT_OFB_IPV6_SRC: {
					field.u128 = oxm.get_u128value().get_ipv6_addr();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_SRC, field, 0x0);
				}break;
				case rofl::openflow12::OFPXMT_OFB_IPV6_DST: {
					field.u128 = oxm.get_u128value().get_ipv6_addr();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_DST, field, 0x0);
				}break;
				case rofl::openflow12::OFPXMT_OFB_IPV6_FLABEL: {
					field.u32 = oxm.get_u32value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_FLABEL, field, 0x0);
				}break;
				case rofl::openflow12::OFPXMT_OFB_IPV6_ND_TARGET: {
					field.u128 = oxm.get_u128value().get_ipv6_addr();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_ND_TARGET, field, 0x0);
				}break;
				case rofl::openflow12::OFPXMT_OFB_IPV6_ND_SLL: {
					field.u64 = oxm.get_u48value().get_mac();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_ND_SLL, field, 0x0);
				}break;
				case rofl::openflow12::OFPXMT_OFB_IPV6_ND_TLL: {
					field.u64 = oxm.get_u48value().get_mac();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_ND_TLL, field, 0x0);
				}break;
				case rofl::openflow12::OFPXMT_OFB_ICMPV6_TYPE: {
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_ICMPV6_TYPE, field, 0x0);
				}break;
				case rofl::openflow12::OFPXMT_OFB_ICMPV6_CODE: {
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_ICMPV6_CODE, field, 0x0);
				}break;
					
				default:
				{
					std::stringstream sstr; sstr << raction;
					ROFL_ERR("of1x_endpoint(%s)::of12_map_flow_entry() "
							"unknown OXM type in action SET-FIELD found: %s",
							sw->dpname.c_str(), sstr.str().c_str());
				}
					break;
				}
			}
				break;
			case rofl::openflow12::OFPXMC_EXPERIMENTER: {
				switch (oxm.get_oxm_field()) {
				case rofl::openflow::experimental::pppoe::OFPXMT_OFX_PPPOE_CODE: {
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPPOE_CODE, field, 0x0);
				} break;
				case rofl::openflow::experimental::pppoe::OFPXMT_OFX_PPPOE_TYPE: {
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPPOE_TYPE, field, 0x0);
				} break;
				case rofl::openflow::experimental::pppoe::OFPXMT_OFX_PPPOE_SID: {
					field.u16 = oxm.get_u16value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPPOE_SID, field, 0x0);
				} break;
				case rofl::openflow::experimental::pppoe::OFPXMT_OFX_PPP_PROT: {
					field.u16 = oxm.get_u16value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPP_PROT, field, 0x0);
				} break;
				case rofl::openflow::experimental::gtp::OFPXMT_OFX_GTP_MSGTYPE: {
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_GTP_MSG_TYPE, field, 0x0);
				} break;
				case rofl::openflow::experimental::gtp::OFPXMT_OFX_GTP_TEID: {
					field.u32 = oxm.get_u32value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_GTP_TEID, field, 0x0);
				} break;
				}

			}
				break;
			default:
			{
				std::stringstream sstr; sstr << raction;
				ROFL_ERR("of1x_endpoint(%s)::of12_map_flow_entry() "
						"unknown OXM class in action SET-FIELD found: %s",
						sw->dpname.c_str(), sstr.str().c_str());
			}
				break;
			}
		}
			break;
		case rofl::openflow12::OFPAT_EXPERIMENTER: {

			rofl::openflow::cofaction_experimenter eaction(raction);

			switch (eaction.get_exp_id()) {
				case ROFL_EXPERIMENTER_ID: {

					/*
					 * but one does not have to, PPPoE still uses a different body definition
					 */
					// ROFL experimental actions contain experimental action type at position data[0]
					uint8_t acttype = eaction.oac_12experimenter->data[0];

					switch (acttype) {
						case rofl::openflow::cofaction_push_pppoe::OFXAT_PUSH_PPPOE: {
							rofl::openflow::cofaction_push_pppoe paction(eaction);
							field.u16 = NTOHB16(paction.eoac_push_pppoe->expbody.ethertype);
							action = of1x_init_packet_action( OF1X_AT_PUSH_PPPOE, field, 0x0);
						} break;
						case rofl::openflow::cofaction_pop_pppoe::OFXAT_POP_PPPOE: {
							rofl::openflow::cofaction_pop_pppoe paction(eaction);
							field.u16 = NTOHB16(paction.eoac_pop_pppoe->expbody.ethertype);
							action = of1x_init_packet_action( OF1X_AT_POP_PPPOE, field, 0x0);
						} break;
					}

				} break;
				default: {
					// TODO
				} break;
			}

			} break;
		}

		if (NULL != apply_actions)
		{
			of1x_push_packet_action_to_group(apply_actions, action);
		}

		if (NULL != write_actions)
		{
			of1x_set_packet_action_on_write_actions(write_actions, action);
		}
	}
}



/*
* Maps a of1x_action TO an OF1.2 Header
*/
void
of12_translation_utils::of12_map_reverse_flow_entry_matches(
		of1x_match_t* m,
		rofl::openflow::cofmatch& match)
{
	while (NULL != m)
	{
		switch (m->type) {
			case OF1X_MATCH_IN_PORT:
				match.set_in_port(of1x_get_match_value32(m));
				break;
			case OF1X_MATCH_IN_PHY_PORT:
				match.set_in_phy_port(of1x_get_match_value32(m));
				break;
			case OF1X_MATCH_METADATA:
				match.set_metadata(of1x_get_match_value64(m), of1x_get_match_mask64(m));
				break;
			case OF1X_MATCH_ETH_DST:
			{
				uint64_t mac = of1x_get_match_value64(m);
				uint64_t msk = of1x_get_match_mask64(m);
				match.set_eth_dst(cmacaddr(mac), cmacaddr(msk));
			}
				break;
			case OF1X_MATCH_ETH_SRC:
			{
				uint64_t mac = of1x_get_match_value64(m);
				uint64_t msk = of1x_get_match_mask64(m);
				match.set_eth_src(cmacaddr(mac), cmacaddr(msk));
			}
				break;
			case OF1X_MATCH_ETH_TYPE:
				match.set_eth_type(of1x_get_match_value16(m));
				break;
			case OF1X_MATCH_VLAN_VID:
				if(m->vlan_present == OF1X_MATCH_VLAN_SPECIFIC) {
					match.set_vlan_vid(of1x_get_match_value16(m), of1x_get_match_mask16(m));
				}
				if(m->vlan_present == OF1X_MATCH_VLAN_NONE) {
					match.set_vlan_vid(rofl::openflow12::OFPVID_NONE);
				}
				if(m->vlan_present == OF1X_MATCH_VLAN_ANY) {
					match.set_vlan_vid(rofl::openflow12::OFPVID_PRESENT, rofl::openflow12::OFPVID_PRESENT);
				}
				break;
			case OF1X_MATCH_VLAN_PCP:
				match.set_vlan_pcp(of1x_get_match_value8(m));
				break;
			case OF1X_MATCH_ARP_OP:
				match.set_arp_opcode(of1x_get_match_value16(m));
				break;
			case OF1X_MATCH_ARP_SHA:
			{
				uint64_t mac = of1x_get_match_value64(m);
				uint64_t msk = of1x_get_match_mask64(m);
				match.set_arp_sha(cmacaddr(mac), cmacaddr(msk));
			}
				break;
			case OF1X_MATCH_ARP_SPA:
			{
				caddress addr(AF_INET, "0.0.0.0");
				caddress mask(AF_INET, "0.0.0.0");
				addr.set_ipv4_addr(of1x_get_match_value32(m));
				mask.set_ipv4_addr(of1x_get_match_mask32(m));
				match.set_arp_spa(addr, mask);
			}
				break;
			case OF1X_MATCH_ARP_THA:
			{
				uint64_t mac = of1x_get_match_value64(m);
				uint64_t msk = of1x_get_match_mask64(m);
				match.set_arp_tha(cmacaddr(mac), cmacaddr(msk));
			}
				break;
			case OF1X_MATCH_ARP_TPA:
			{
				caddress addr(AF_INET, "0.0.0.0");
				caddress mask(AF_INET, "0.0.0.0");
				addr.set_ipv4_addr(of1x_get_match_value32(m));
				mask.set_ipv4_addr(of1x_get_match_mask32(m));
				match.set_arp_tpa(addr, mask);
			}
				break;
			case OF1X_MATCH_IP_DSCP:
				match.set_ip_dscp(of1x_get_match_value8(m));
				break;
			case OF1X_MATCH_IP_ECN:
				match.set_ip_ecn(of1x_get_match_value8(m));
				break;
			case OF1X_MATCH_IP_PROTO:
				match.set_ip_proto(of1x_get_match_value8(m));
				break;
			case OF1X_MATCH_IPV4_SRC:
			{
				caddress addr(AF_INET, "0.0.0.0");
				caddress mask(AF_INET, "0.0.0.0");
				addr.set_ipv4_addr(of1x_get_match_value32(m));
				mask.set_ipv4_addr(of1x_get_match_mask32(m));
				match.set_ipv4_src(addr, mask);
			}
				break;
			case OF1X_MATCH_IPV4_DST:
			{
				caddress addr(AF_INET, "0.0.0.0");
				caddress mask(AF_INET, "0.0.0.0");
				addr.set_ipv4_addr(of1x_get_match_value32(m));
				mask.set_ipv4_addr(of1x_get_match_mask32(m));
				match.set_ipv4_dst(addr, mask);
			}
				break;
			case OF1X_MATCH_TCP_SRC:
				match.set_tcp_src(of1x_get_match_value16(m));
				break;
			case OF1X_MATCH_TCP_DST:
				match.set_tcp_dst(of1x_get_match_value16(m));
				break;
			case OF1X_MATCH_UDP_SRC:
				match.set_udp_src(of1x_get_match_value16(m));
				break;
			case OF1X_MATCH_UDP_DST:
				match.set_udp_dst(of1x_get_match_value16(m));
				break;
			case OF1X_MATCH_SCTP_SRC:
				match.set_sctp_src(of1x_get_match_value16(m));
				break;
			case OF1X_MATCH_SCTP_DST:
				match.set_sctp_dst(of1x_get_match_value16(m));
				break;
			case OF1X_MATCH_ICMPV4_TYPE:
				match.set_icmpv4_type(of1x_get_match_value8(m));
				break;
			case OF1X_MATCH_ICMPV4_CODE:
				match.set_icmpv4_code(of1x_get_match_value8(m));
				break;
			case OF1X_MATCH_IPV6_SRC: {
				caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
				caddress mask(AF_INET6,"0:0:0:0:0:0:0:0");
				addr.set_ipv6_addr(of1x_get_match_value128(m));
				addr.set_ipv6_addr(of1x_get_match_mask128(m));
				match.set_ipv6_src(addr,mask);
				}break;
			case OF1X_MATCH_IPV6_DST:{
				caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
				caddress mask(AF_INET6,"0:0:0:0:0:0:0:0");
				addr.set_ipv6_addr(of1x_get_match_value128(m));
				mask.set_ipv6_addr(of1x_get_match_mask128(m));
				match.set_ipv6_dst(addr, mask);
				}break;
			case OF1X_MATCH_IPV6_FLABEL:
				match.set_ipv6_flabel(of1x_get_match_value32(m));
				break;
			case OF1X_MATCH_ICMPV6_TYPE:
				match.set_icmpv6_type(of1x_get_match_value8(m));
				break;
			case OF1X_MATCH_ICMPV6_CODE:
				match.set_icmpv6_code(of1x_get_match_value8(m));
				break;
			case OF1X_MATCH_IPV6_ND_TARGET:{
				caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
				addr.set_ipv6_addr(of1x_get_match_value128(m));
				match.set_ipv6_nd_target(addr);
				}break;
			case OF1X_MATCH_IPV6_ND_SLL:{
				uint64_t mac = of1x_get_match_value64(m);
				match.set_ipv6_nd_sll(cmacaddr(mac));
				}break;
			case OF1X_MATCH_IPV6_ND_TLL:{
				uint64_t mac = of1x_get_match_value64(m);
				match.set_ipv6_nd_tll(cmacaddr(mac));
				}break;
			case OF1X_MATCH_MPLS_LABEL:
				match.set_mpls_label(of1x_get_match_value32(m));
				break;
			case OF1X_MATCH_MPLS_TC:
				match.set_mpls_tc(of1x_get_match_value8(m));
				break;
			case OF1X_MATCH_PPPOE_CODE:
				match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_code(of1x_get_match_value8(m)));
				break;
			case OF1X_MATCH_PPPOE_TYPE:
				match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_type(of1x_get_match_value8(m)));
				break;
			case OF1X_MATCH_PPPOE_SID:
				match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_sid(of1x_get_match_value16(m)));
				break;
			case OF1X_MATCH_PPP_PROT:
				match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_ppp_prot(of1x_get_match_value16(m)));
				break;
			case OF1X_MATCH_GTP_MSG_TYPE:
				match.set_matches().add_match(rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_msg_type(of1x_get_match_value8(m)));
				break;
			case OF1X_MATCH_GTP_TEID:
				match.set_matches().add_match(rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_teid(of1x_get_match_value32(m), of1x_get_match_mask32(m)));
				break;
			default:
				break;
		}

		m = m->next;
	}
}

/**
* Maps a of1x_group_bucket from an OF1.2 Header
*/
void
of12_translation_utils::of12_map_bucket_list(
		crofctl *ctl,
		openflow_switch* sw,
		rofl::openflow::cofbuckets& of_buckets,
		of1x_bucket_list_t* bucket_list)
{	
	
	for (std::map<uint32_t, rofl::openflow::cofbucket>::iterator
			it = of_buckets.set_buckets().begin(); it != of_buckets.set_buckets().end(); ++it) {

		//for each bucket we must map its actions
		rofl::openflow::cofbucket& bucket_ptr = it->second;
		of1x_action_group_t* action_group = of1x_init_action_group(NULL);
		if(action_group == NULL){
			//TODO Handle Error
		}
		
		of12_map_flow_entry_actions(ctl,sw,bucket_ptr.set_actions(),action_group,NULL);
		of1x_insert_bucket_in_list(bucket_list,of1x_init_bucket(bucket_ptr.get_weight(), bucket_ptr.get_watch_port(), bucket_ptr.get_watch_group(), action_group));
	}
}

void of12_translation_utils::of12_map_reverse_bucket_list(
		rofl::openflow::cofbuckets& of_buckets,
		of1x_bucket_list_t* bucket_list){
	
	uint32_t bucket_id = 0;

	for(of1x_bucket_t *bu_it=bucket_list->head;bu_it;bu_it=bu_it->next){
		//cofbucket single_bucket;
		rofl::openflow::cofactions ac_list(rofl::openflow12::OFP_VERSION);
		for (of1x_packet_action_t *action_it = bu_it->actions->head; action_it != NULL; action_it = action_it->next) {
			if (OF1X_AT_NO_ACTION == action_it->type)
				continue;
			rofl::openflow::cofaction action(rofl::openflow12::OFP_VERSION);
			of12_map_reverse_flow_entry_action(action_it, action);
			//push this action into the list
			ac_list.append_action(action);
		}

		of_buckets.set_bucket(bucket_id).set_actions() = ac_list;
		of_buckets.set_bucket(bucket_id).set_watch_port(bu_it->port);
		of_buckets.set_bucket(bucket_id).set_watch_group(bu_it->group);
		of_buckets.set_bucket(bucket_id).set_weight(bu_it->weight);

		bucket_id++;
	}
}


/**
*
*/
void
of12_translation_utils::of12_map_reverse_flow_entry_instructions(
		of1x_instruction_group_t* group,
		rofl::openflow::cofinstructions& instructions)
{
	for (unsigned int i = 0; i < (sizeof(group->instructions) / sizeof(of1x_instruction_t)); i++) {
		if (OF1X_IT_NO_INSTRUCTION == group->instructions[i].type)
			continue;
		rofl::openflow::cofinst instruction(rofl::openflow12::OFP_VERSION);
		of12_map_reverse_flow_entry_instruction(&(group->instructions[i]), instruction);
		instructions.add_inst(instruction);
	}
}


void
of12_translation_utils::of12_map_reverse_flow_entry_instruction(
		of1x_instruction_t* inst,
		rofl::openflow::cofinst& instruction)
{
	switch (inst->type) {
	case OF1X_IT_APPLY_ACTIONS: {
		instruction = rofl::openflow::cofinst_apply_actions(rofl::openflow12::OFP_VERSION);
		for (of1x_packet_action_t *of1x_action = inst->apply_actions->head; of1x_action != NULL; of1x_action = of1x_action->next) {
			if (OF1X_AT_NO_ACTION == of1x_action->type)
				continue;
			rofl::openflow::cofaction action(rofl::openflow12::OFP_VERSION);
			of12_map_reverse_flow_entry_action(of1x_action, action);
			instruction.get_actions().append_action(action);
		}
	} break;
	case OF1X_IT_CLEAR_ACTIONS: {
		instruction = rofl::openflow::cofinst_clear_actions(rofl::openflow12::OFP_VERSION);
	} break;
	case OF1X_IT_WRITE_ACTIONS: {
		instruction = rofl::openflow::cofinst_write_actions(rofl::openflow12::OFP_VERSION);
		for (unsigned int i = 0; i < inst->write_actions->num_of_actions; i++) {
			if (OF1X_AT_NO_ACTION == inst->write_actions->actions[i].type)
				continue;
			rofl::openflow::cofaction action(rofl::openflow12::OFP_VERSION);
			of12_map_reverse_flow_entry_action(&(inst->write_actions->actions[i]), action);
			instruction.get_actions().append_action(action);
		}
	} break;
	case OF1X_IT_WRITE_METADATA: {
		instruction = rofl::openflow::cofinst_write_metadata(rofl::openflow12::OFP_VERSION,
				inst->write_metadata.metadata, inst->write_metadata.metadata_mask);
	} break;
	case OF1X_IT_EXPERIMENTER: {
		// TODO: both are marked TODO in of1x_pipeline
	} break;
	case OF1X_IT_GOTO_TABLE: {
		instruction = rofl::openflow::cofinst_goto_table(rofl::openflow12::OFP_VERSION, inst->go_to_table);
	} break;
	default: {
		// do nothing
	} break;
	}
}


void
of12_translation_utils::of12_map_reverse_flow_entry_action(
		of1x_packet_action_t* of1x_action,
		rofl::openflow::cofaction& action)
{
	switch (of1x_action->type) {
		case OF1X_AT_NO_ACTION: {
			// do nothing
		} break;
		case OF1X_AT_COPY_TTL_IN: {
			action = rofl::openflow::cofaction_copy_ttl_in(rofl::openflow12::OFP_VERSION);
		} break;
		case OF1X_AT_POP_VLAN: {
			action = rofl::openflow::cofaction_pop_vlan(rofl::openflow12::OFP_VERSION);
		} break;
		case OF1X_AT_POP_MPLS: {
			action = rofl::openflow::cofaction_pop_mpls(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field16(of1x_action));
		} break;
		
		/* Extensions */
		case OF1X_AT_POP_PPPOE: {
			action = rofl::openflow::cofaction_pop_pppoe(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field16(of1x_action));
		} break;
		case OF1X_AT_PUSH_PPPOE: {
			action = rofl::openflow::cofaction_push_pppoe(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field16(of1x_action));
		} break;
		/* End of extensions */

		case OF1X_AT_PUSH_MPLS: {
			action = rofl::openflow::cofaction_push_mpls(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field16(of1x_action));
		} break;
		case OF1X_AT_PUSH_VLAN: {
			action = rofl::openflow::cofaction_push_vlan(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field16(of1x_action));
		} break;
		case OF1X_AT_COPY_TTL_OUT: {
			action = rofl::openflow::cofaction_copy_ttl_out(rofl::openflow12::OFP_VERSION);
		} break;
		case OF1X_AT_DEC_NW_TTL: {
			action = rofl::openflow::cofaction_dec_nw_ttl(rofl::openflow12::OFP_VERSION);
		} break;
		case OF1X_AT_DEC_MPLS_TTL: {
			action = rofl::openflow::cofaction_dec_mpls_ttl(rofl::openflow12::OFP_VERSION);
		} break;
		case OF1X_AT_SET_MPLS_TTL: {
			action = rofl::openflow::cofaction_set_mpls_ttl(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field8(of1x_action));
		} break;
		case OF1X_AT_SET_NW_TTL: {
			action = rofl::openflow::cofaction_set_nw_ttl(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field8(of1x_action));
		} break;
		case OF1X_AT_SET_QUEUE: {
			action = rofl::openflow::cofaction_set_queue(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field8(of1x_action));
		} break;
		//case OF1X_AT_SET_FIELD_METADATA:
		case OF1X_AT_SET_FIELD_ETH_DST: {
			uint64_t mac = of1x_get_packet_action_field64(of1x_action);
			cmacaddr maddr(mac);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_eth_dst(maddr));
		} break;
		case OF1X_AT_SET_FIELD_ETH_SRC: {
			uint64_t mac = of1x_get_packet_action_field64(of1x_action);
			cmacaddr maddr(mac);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_eth_src(maddr));
		} break;
		case OF1X_AT_SET_FIELD_ETH_TYPE: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_eth_type(of1x_get_packet_action_field16(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_VLAN_VID: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_vlan_vid(of1x_get_packet_action_field16(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_VLAN_PCP: 
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_vlan_pcp(of1x_get_packet_action_field8(of1x_action)));
		break;
		case OF1X_AT_SET_FIELD_ARP_OPCODE: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_opcode(of1x_get_packet_action_field16(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_ARP_SHA: {
			uint64_t mac = of1x_get_packet_action_field64(of1x_action);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_sha(cmacaddr(mac)));
		} break;
		case OF1X_AT_SET_FIELD_ARP_SPA: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_spa(of1x_get_packet_action_field32(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_ARP_THA: {
			uint64_t mac = of1x_get_packet_action_field64(of1x_action);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_tha(cmacaddr(mac)));
		} break;
		case OF1X_AT_SET_FIELD_ARP_TPA: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_tpa(of1x_get_packet_action_field32(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_IP_DSCP: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ip_dscp(of1x_get_packet_action_field8(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_IP_ECN: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ip_ecn(of1x_get_packet_action_field8(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_IP_PROTO: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ip_proto(of1x_get_packet_action_field8(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_IPV4_SRC: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv4_src(of1x_get_packet_action_field32(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_IPV4_DST: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv4_dst(of1x_get_packet_action_field32(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_TCP_SRC: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_tcp_src(of1x_get_packet_action_field16(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_TCP_DST: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_tcp_dst(of1x_get_packet_action_field16(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_UDP_SRC: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_udp_src(of1x_get_packet_action_field16(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_UDP_DST: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_udp_dst(of1x_get_packet_action_field16(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_ICMPV4_TYPE: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_icmpv4_type(of1x_get_packet_action_field8(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_ICMPV4_CODE: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_icmpv4_code(of1x_get_packet_action_field8(of1x_action)));
		} break;
		
		case OF1X_AT_SET_FIELD_IPV6_SRC: {
			uint128__t value = of1x_get_packet_action_field128(of1x_action);
			rofl::caddress addr(AF_INET6);
			addr.set_ipv6_addr(value);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_src(addr));
		} break;
		case OF1X_AT_SET_FIELD_IPV6_DST: {
			uint128__t value = of1x_get_packet_action_field128(of1x_action);
			rofl::caddress addr(AF_INET6);
			addr.set_ipv6_addr(value);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_dst(addr));
		} break;
		case OF1X_AT_SET_FIELD_IPV6_FLABEL: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_flabel(of1x_get_packet_action_field32(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_IPV6_ND_TARGET: {
			uint128__t value = of1x_get_packet_action_field128(of1x_action);
			rofl::caddress addr(AF_INET6);
			addr.set_ipv6_addr(value);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_nd_target(addr));
		} break;
		case OF1X_AT_SET_FIELD_IPV6_ND_SLL: {
			uint64_t mac = of1x_get_packet_action_field64(of1x_action);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_nd_sll(cmacaddr(mac)));
		} break;
		case OF1X_AT_SET_FIELD_IPV6_ND_TLL: {
			uint64_t mac = of1x_get_packet_action_field64(of1x_action);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_nd_tll(cmacaddr(mac)));
		} break;
		case OF1X_AT_SET_FIELD_ICMPV6_TYPE: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_icmpv6_type(of1x_get_packet_action_field8(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_ICMPV6_CODE: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_icmpv6_code(of1x_get_packet_action_field8(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_MPLS_LABEL: {
			uint32_t label = of1x_get_packet_action_field32(of1x_action);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_mpls_label(label));
		} break;
		case OF1X_AT_SET_FIELD_MPLS_TC: {
			uint8_t tc = of1x_get_packet_action_field8(of1x_action);
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::coxmatch_ofb_mpls_tc(tc));
		} break;

		/* Extensions */
		case OF1X_AT_SET_FIELD_PPPOE_CODE: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_code(of1x_get_packet_action_field8(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_PPPOE_TYPE: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_type(of1x_get_packet_action_field8(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_PPPOE_SID: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_sid(of1x_get_packet_action_field16(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_PPP_PROT: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::experimental::pppoe::coxmatch_ofx_ppp_prot(of1x_get_packet_action_field16(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_GTP_MSG_TYPE: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_msg_type(of1x_get_packet_action_field8(of1x_action)));
		} break;
		case OF1X_AT_SET_FIELD_GTP_TEID: {
			action = rofl::openflow::cofaction_set_field(rofl::openflow12::OFP_VERSION, rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_teid(of1x_get_packet_action_field32(of1x_action)));
		} break;
		/* End of extensions */

		case OF1X_AT_GROUP: {
			action = rofl::openflow::cofaction_group(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field32(of1x_action));
		} break;
		case OF1X_AT_EXPERIMENTER: {
			// TODO
		} break;
		case OF1X_AT_OUTPUT: {
			action = rofl::openflow::cofaction_output(rofl::openflow12::OFP_VERSION, of1x_get_packet_action_field32(of1x_action), of1x_action->send_len);
		} break;
		default: {
			// do nothing
		} break;
	}
}


/*
* Maps packet actions to cofmatches
*/

void of12_translation_utils::of12_map_reverse_packet_matches(packet_matches_t* pm, rofl::openflow::cofmatch& match){

	uint128__t tmp;

	if(packet_matches_get_port_in_value(pm))
		match.set_in_port(packet_matches_get_port_in_value(pm));
	if(packet_matches_get_phy_port_in_value(pm))
		match.set_in_phy_port(packet_matches_get_phy_port_in_value(pm));
	if(packet_matches_get_metadata_value(pm))
		match.set_metadata(packet_matches_get_metadata_value(pm));
	if(packet_matches_get_eth_dst_value(pm)){
		uint64_t mac = packet_matches_get_eth_dst_value(pm); 
		match.set_eth_dst( cmacaddr(mac) );
	}
	if(packet_matches_get_eth_src_value(pm)){
		uint64_t mac = packet_matches_get_eth_src_value(pm); 
		match.set_eth_src( cmacaddr(mac) );
	}
	if(packet_matches_get_eth_type_value(pm))
		match.set_eth_type(packet_matches_get_eth_type_value(pm));
	if(packet_matches_get_vlan_vid_value(pm))
		match.set_vlan_vid(packet_matches_get_vlan_vid_value(pm));
	if(packet_matches_get_vlan_pcp_value(pm))
		match.set_vlan_pcp(packet_matches_get_vlan_pcp_value(pm));
	if(packet_matches_get_arp_opcode_value(pm))
		match.set_arp_opcode(packet_matches_get_arp_opcode_value(pm));
	if(packet_matches_get_arp_sha_value(pm)){
		uint64_t mac = packet_matches_get_arp_sha_value(pm);
		match.set_arp_sha( cmacaddr(mac) );
	}
	if(packet_matches_get_arp_spa_value(pm)); {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(packet_matches_get_arp_spa_value(pm));
		match.set_arp_spa(addr);
	}
	if(packet_matches_get_arp_tha_value(pm)){
		uint64_t mac = packet_matches_get_arp_tha_value(pm);
		match.set_arp_tha(cmacaddr(mac));
	}
	if(packet_matches_get_arp_tpa_value(pm)); {
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(packet_matches_get_arp_tpa_value(pm));
		match.set_arp_tpa(addr);
	}
	if(packet_matches_get_ip_dscp_value(pm))
		match.set_ip_dscp(packet_matches_get_ip_dscp_value(pm));
	if(packet_matches_get_ip_ecn_value(pm))
		match.set_ip_ecn(packet_matches_get_ip_ecn_value(pm));
	if(packet_matches_get_ip_proto_value(pm))
		match.set_ip_proto(packet_matches_get_ip_proto_value(pm));
	if(packet_matches_get_ipv4_src_value(pm)){
			caddress addr(AF_INET, "0.0.0.0");
			addr.set_ipv4_addr(packet_matches_get_ipv4_src_value(pm));
			match.set_ipv4_src(addr);

	}
	if(packet_matches_get_ipv4_dst_value(pm)){
		caddress addr(AF_INET, "0.0.0.0");
		addr.set_ipv4_addr(packet_matches_get_ipv4_dst_value(pm));
		match.set_ipv4_dst(addr);
	}
	if(packet_matches_get_tcp_src_value(pm))
		match.set_tcp_src(packet_matches_get_tcp_src_value(pm));
	if(packet_matches_get_tcp_dst_value(pm))
		match.set_tcp_dst(packet_matches_get_tcp_dst_value(pm));
	if(packet_matches_get_udp_src_value(pm))
		match.set_udp_src(packet_matches_get_udp_src_value(pm));
	if(packet_matches_get_udp_dst_value(pm))
		match.set_udp_dst(packet_matches_get_udp_dst_value(pm));
	if(packet_matches_get_icmpv4_type_value(pm))
		match.set_icmpv4_type(packet_matches_get_icmpv4_type_value(pm));
	if(packet_matches_get_icmpv4_code_value(pm))
		match.set_icmpv4_code(packet_matches_get_icmpv4_code_value(pm));
	
	tmp = packet_matches_get_ipv6_src_value(pm);	
	if( UINT128__T_IS_NOT_ZERO(tmp) ){
		caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
		uint128__t addru128 = packet_matches_get_ipv6_src_value(pm);
		addr.set_ipv6_addr(addru128);
		match.set_ipv6_src(addr);
	}
	
	tmp = packet_matches_get_ipv6_dst_value(pm);
	if( UINT128__T_IS_NOT_ZERO(tmp) ){
		caddress addr(AF_INET6,"0:0:0:0");
		uint128__t addru128 = packet_matches_get_ipv6_dst_value(pm);
		addr.set_ipv6_addr(addru128);
		match.set_ipv6_dst(addr);
	}
	if(packet_matches_get_ipv6_flabel_value(pm))
		match.set_ipv6_flabel(packet_matches_get_ipv6_flabel_value(pm));

	tmp = packet_matches_get_ipv6_nd_target_value(pm);
	if( UINT128__T_IS_NOT_ZERO(tmp) ){
		caddress addr(AF_INET6,"0:0:0:0");
		uint128__t addru128 = packet_matches_get_ipv6_nd_target_value(pm);
		addr.set_ipv6_addr(addru128);
		match.set_ipv6_nd_target(addr);
	}
	if(packet_matches_get_ipv6_nd_sll_value(pm)){
		uint64_t mac = packet_matches_get_ipv6_nd_sll_value(pm);
		match.set_ipv6_nd_sll(cmacaddr(mac));
	}
	if(packet_matches_get_ipv6_nd_tll_value(pm)){
		uint64_t mac = packet_matches_get_ipv6_nd_tll_value(pm);
		match.set_ipv6_nd_tll(cmacaddr(mac));
	}
	if(packet_matches_get_icmpv6_type_value(pm))
		match.set_icmpv6_type(packet_matches_get_icmpv6_type_value(pm));
	if(packet_matches_get_icmpv6_code_value(pm))
		match.set_icmpv6_code(packet_matches_get_icmpv6_code_value(pm));
		
	if(packet_matches_get_mpls_label_value(pm)){
		match.set_mpls_label(packet_matches_get_mpls_label_value(pm));
	}
	if(packet_matches_get_mpls_tc_value(pm))
		match.set_mpls_tc(packet_matches_get_mpls_tc_value(pm));

	//Extensions
	if(packet_matches_get_pppoe_code_value(pm))
		match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_code(packet_matches_get_pppoe_code_value(pm)));
	if(packet_matches_get_pppoe_type_value(pm))
		match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_type(packet_matches_get_pppoe_type_value(pm)));
	if(packet_matches_get_pppoe_sid_value(pm))
		match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_sid(packet_matches_get_pppoe_sid_value(pm)));
	if(packet_matches_get_ppp_proto_value(pm))
		match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_ppp_prot(packet_matches_get_ppp_proto_value(pm)));
	if(packet_matches_get_gtp_msg_type_value(pm))
		match.set_matches().add_match(rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_msg_type(packet_matches_get_gtp_msg_type_value(pm)));
	if(packet_matches_get_gtp_teid_value(pm))
		match.set_matches().add_match(rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_teid(packet_matches_get_gtp_teid_value(pm)));
}

/*
* Table capability bitmap
*/


uint64_t of12_translation_utils::of12_map_bitmap_matches(bitmap128_t* bitmap){

	uint64_t mapped_bitmap=0x0;

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IN_PORT))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IN_PORT);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IN_PHY_PORT))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IN_PHY_PORT);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_METADATA))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_METADATA);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ETH_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ETH_DST);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ETH_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ETH_SRC);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ETH_TYPE))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ETH_TYPE);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_VLAN_VID))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_VLAN_VID);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_VLAN_PCP))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_VLAN_PCP);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_MPLS_LABEL))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_MPLS_LABEL);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_MPLS_TC))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_MPLS_TC);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ARP_OP))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ARP_OP);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ARP_SPA))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ARP_SPA);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ARP_TPA))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ARP_TPA);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ARP_SHA))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ARP_SHA);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ARP_THA))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ARP_THA);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IP_DSCP))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IP_DSCP);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IP_ECN))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IP_ECN);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IP_PROTO))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IP_PROTO);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IPV4_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IPV4_SRC);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IPV4_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IPV4_DST);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IPV6_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IPV6_SRC);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IPV6_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IPV6_DST);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IPV6_FLABEL))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IPV6_FLABEL);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ICMPV6_TYPE))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ICMPV6_TYPE);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ICMPV6_CODE))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ICMPV6_CODE);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IPV6_ND_TARGET))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IPV6_ND_TARGET);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IPV6_ND_SLL))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IPV6_ND_SLL);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_IPV6_ND_TLL))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_IPV6_ND_TLL);
	
	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_TCP_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_TCP_SRC);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_TCP_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_TCP_DST);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_UDP_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_UDP_SRC);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_UDP_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_UDP_DST);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_SCTP_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_SCTP_SRC);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_SCTP_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_SCTP_DST);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ICMPV4_TYPE))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ICMPV4_TYPE);

	if(bitmap128_is_bit_set(bitmap, OF1X_MATCH_ICMPV4_CODE))
		mapped_bitmap |= ( UINT64_C(1) <<  openflow12::OFPXMT_OFB_ICMPV4_CODE);

	return mapped_bitmap;	
}

uint32_t of12_translation_utils::of12_map_bitmap_actions(bitmap128_t* bitmap){

	uint64_t t = 0x0;

	t |= 1 << rofl::openflow::OFPAT_SET_FIELD;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_OUTPUT ))
		t |= 1 << rofl::openflow::OFPAT_OUTPUT;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_COPY_TTL_OUT ))
		t |= 1 << rofl::openflow::OFPAT_COPY_TTL_OUT;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_COPY_TTL_IN ))
		t |= 1 << rofl::openflow::OFPAT_COPY_TTL_IN;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_MPLS_TTL ))
		t |= 1 << rofl::openflow::OFPAT_SET_MPLS_TTL;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_DEC_MPLS_TTL ))
		t |= 1 << rofl::openflow::OFPAT_DEC_MPLS_TTL;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_PUSH_VLAN ))
		t |= 1 << rofl::openflow::OFPAT_PUSH_VLAN;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_POP_VLAN ))
		t |= 1 << rofl::openflow::OFPAT_POP_VLAN;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_PUSH_MPLS ))
		t |= 1 << rofl::openflow::OFPAT_PUSH_MPLS;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_POP_MPLS ))
		t |= 1 << rofl::openflow::OFPAT_POP_MPLS;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_QUEUE ))
		t |= 1 << rofl::openflow::OFPAT_SET_QUEUE;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_GROUP ))
		t |= 1 << rofl::openflow::OFPAT_GROUP;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_NW_TTL ))
		t |= 1 << rofl::openflow::OFPAT_SET_NW_TTL;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_DEC_NW_TTL ))
		t |= 1 << rofl::openflow::OFPAT_DEC_NW_TTL;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_PUSH_PBB ))
		t |= 1 << rofl::openflow::OFPAT_PUSH_PBB;
	if(bitmap128_is_bit_set(bitmap, OF1X_AT_POP_PBB ))
		t |= 1 << rofl::openflow::OFPAT_POP_PBB;

	return t;	
}

uint64_t of12_translation_utils::of12_map_bitmap_set_fields(bitmap128_t* bitmap){

	uint64_t t = 0x0;

	//Metadata ???
	//if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_METADATA ))
	//	 t |= 1UL << rofl::openflow::OFPXMT_OFB_METADATA;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ETH_DST ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ETH_DST;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ETH_SRC ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ETH_SRC;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ETH_TYPE ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ETH_TYPE;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_VLAN_VID ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_VLAN_VID;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_VLAN_PCP ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_VLAN_PCP;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IP_DSCP ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_IP_DSCP;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IP_ECN ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_IP_ECN;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IP_PROTO ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_IP_PROTO;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IPV4_SRC ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_IPV4_SRC;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IPV4_DST ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_IPV4_DST;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_TCP_SRC ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_TCP_SRC;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_TCP_DST ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_TCP_DST;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_UDP_SRC ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_UDP_SRC;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_UDP_DST ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_UDP_DST;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_SCTP_SRC ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_SCTP_SRC;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_SCTP_DST ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_SCTP_DST;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ICMPV4_TYPE ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ICMPV4_TYPE;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ICMPV4_CODE ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ICMPV4_CODE;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ARP_OPCODE ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ARP_OP;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ARP_SPA ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ARP_SPA;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ARP_TPA ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ARP_TPA;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ARP_SHA ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ARP_SHA;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ARP_THA ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ARP_THA;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IPV6_SRC ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_IPV6_SRC;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IPV6_DST ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_IPV6_DST;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IPV6_FLABEL ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_IPV6_FLABEL;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ICMPV6_TYPE ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ICMPV6_TYPE;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_ICMPV6_CODE ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_ICMPV6_CODE;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IPV6_ND_TARGET ))
		 t |= 1UL << rofl::openflow::OFPXMT_OFB_IPV6_ND_TARGET;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IPV6_ND_SLL ))
		 t |= 1ULL << rofl::openflow::OFPXMT_OFB_IPV6_ND_SLL;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_IPV6_ND_TLL ))
		 t |= 1ULL << rofl::openflow::OFPXMT_OFB_IPV6_ND_TLL;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_MPLS_LABEL ))
		 t |= 1ULL << rofl::openflow::OFPXMT_OFB_MPLS_LABEL;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_MPLS_TC ))
		 t |= 1ULL << rofl::openflow::OFPXMT_OFB_MPLS_TC;
	 if(bitmap128_is_bit_set(bitmap, OF1X_AT_SET_FIELD_MPLS_BOS ))
		 t |= 1ULL << rofl::openflow::OFPXMT_OFB_MPLS_BOS;
	
	return t;
}

uint32_t of12_translation_utils::of12_map_bitmap_instructions(uint32_t* bitmap){
	
	uint32_t mapped_bitmap=0x0;

	if(*bitmap & ( 1 << OF1X_IT_APPLY_ACTIONS))
		mapped_bitmap |= (1 << openflow12::OFPIT_APPLY_ACTIONS);

	if(*bitmap & ( 1 << OF1X_IT_CLEAR_ACTIONS))
		mapped_bitmap |= (1 << openflow12::OFPIT_CLEAR_ACTIONS);

	if(*bitmap & ( 1 << OF1X_IT_WRITE_ACTIONS))
		mapped_bitmap |= (1 << openflow12::OFPIT_WRITE_ACTIONS);

	if(*bitmap & ( 1 << OF1X_IT_WRITE_METADATA))
		mapped_bitmap |= (1 << openflow12::OFPIT_WRITE_METADATA);
	
	if(*bitmap & ( 1 << OF1X_IT_GOTO_TABLE))
		mapped_bitmap |= (1 << openflow12::OFPIT_GOTO_TABLE);

	return mapped_bitmap;	
}
