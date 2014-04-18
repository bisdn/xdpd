#include "of13_translation_utils.h"

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
uint32_t of13_translation_utils::get_port_speed_kb(port_features_t features){

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
* Maps a of1x_flow_entry from an OF1.3 Header
*/
of1x_flow_entry_t*
of13_translation_utils::of13_map_flow_entry(
		crofctl *ctl, 
		rofl::openflow::cofmsg_flow_mod *msg,
		openflow_switch* sw)
{

	of1x_flow_entry_t *entry = of1x_init_flow_entry(msg->get_flags() & rofl::openflow13::OFPFF_SEND_FLOW_REM);

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
		of13_map_flow_entry_matches(ctl, msg->get_match(), sw, entry);
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
			of13_map_flow_entry_actions(ctl, sw,
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
			of13_map_flow_entry_actions(ctl, sw,
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
* Maps a of1x_match from an OF1.3 Header
*/
void
of13_translation_utils::of13_map_flow_entry_matches(
		crofctl *ctl,
		rofl::openflow::cofmatch const& ofmatch,
		openflow_switch* sw, 
		of1x_flow_entry *entry)
{

#if 0
	try {
		of1x_match_t *match = of1x_init_port_in_match(ofmatch.get_in_port());

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_port_in_phy_match(ofmatch.get_in_phy_port());

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	// metadata not implemented
	try {
		uint64_t value = ofmatch.get_metadata_value();
		uint64_t mask = ofmatch.get_metadata_mask();
		of1x_match_t *match = of1x_init_metadata_match(value, mask);

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		uint64_t maddr = ofmatch.get_eth_dst_addr().get_mac();
		MACTOBE(maddr);
		uint64_t mmask = ofmatch.get_eth_dst_mask().get_mac();
		MACTOBE(mmask);

		of1x_match_t *match = of1x_init_eth_dst_match(maddr, mmask);
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		uint64_t maddr = ofmatch.get_eth_src_addr().get_mac();
		MACTOBE(maddr);
		uint64_t mmask = ofmatch.get_eth_src_mask().get_mac();
		MACTOBE(mmask);

		of1x_match_t *match = of1x_init_eth_src_match(maddr, mmask);
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_eth_type_match(htobe16(ofmatch.get_eth_type()));
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		uint16_t value = ofmatch.get_vlan_vid_value();
		uint16_t mask  = ofmatch.get_vlan_vid_mask();
		enum of1x_vlan_present vlan_present = OF1X_MATCH_VLAN_NONE;

		if ((value == rofl::openflow12::OFPVID_PRESENT) && (mask == rofl::openflow12::OFPVID_PRESENT)){
			vlan_present = OF1X_MATCH_VLAN_ANY;
		} else
		if (value & rofl::openflow12::OFPVID_PRESENT) {
			vlan_present = OF1X_MATCH_VLAN_SPECIFIC;
		}
		
		of1x_match_t *match = of1x_init_vlan_vid_match(
								htobe16(ofmatch.get_vlan_vid_value() & ~openflow::OFPVID_PRESENT),
								htobe16(ofmatch.get_vlan_vid_mask()),
								vlan_present);

		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		of1x_match_t *match = of1x_init_vlan_pcp_match(OF1X_VLAN_PCP_ALIGN(ofmatch.get_vlan_pcp()));
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_ip_dscp_match(ofmatch.get_ip_dscp());
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_ip_ecn_match(ofmatch.get_ip_ecn());
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_ip_proto_match(ofmatch.get_ip_proto());
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		caddress value(ofmatch.get_ipv4_src_value());
		caddress mask (ofmatch.get_ipv4_src_mask());

		of1x_match_t *match = of1x_init_ip4_src_match(
			htobe32(value.get_ipv4_addr()), 
			htobe32(mask.get_ipv4_addr()));
		
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		caddress value(ofmatch.get_ipv4_dst_value());
		caddress mask (ofmatch.get_ipv4_dst_mask());

		of1x_match_t *match = of1x_init_ip4_dst_match(
			htobe32(value.get_ipv4_addr()),
			htobe32(mask.get_ipv4_addr()));
		
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_tcp_src_match(htobe16(ofmatch.get_tcp_src()));

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_tcp_dst_match(htobe16(ofmatch.get_tcp_dst()));

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_udp_src_match(htobe16(ofmatch.get_udp_src()));

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_udp_dst_match(htobe16(ofmatch.get_udp_dst()));

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		ofmatch.get_sctp_src();

		throw eNotImplemented(std::string("of13_translation_utils::flow_mod_add() rofl::openflow13::OFPXMT_OFB_SCTP_SRC is missing")); // TODO
	} catch (...) {}

	try {
		ofmatch.get_sctp_dst();

		throw eNotImplemented(std::string("of13_translation_utils::flow_mod_add() rofl::openflow13::OFPXMT_OFB_SCTP_DST is missing")); // TODO
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_icmpv4_type_match(ofmatch.get_icmpv4_type());

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_icmpv4_code_match(ofmatch.get_icmpv4_code());

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_arp_opcode_match(htobe16(ofmatch.get_arp_opcode()));

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		uint64_t maddr = ofmatch.get_arp_sha_addr().get_mac();
		MACTOBE(maddr);
		uint64_t mmask = ofmatch.get_arp_sha_mask().get_mac();
		MACTOBE(mmask);

		of1x_match_t *match = of1x_init_arp_sha_match(maddr, mmask);
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		caddress value(ofmatch.get_arp_spa_value());
		caddress mask (ofmatch.get_arp_spa_mask());

		of1x_match_t *match = of1x_init_arp_spa_match(
			value.ca_s4addr->sin_addr.s_addr,
			mask.ca_s4addr->sin_addr.s_addr);
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		uint64_t maddr = ofmatch.get_arp_tha_addr().get_mac();
		MACTOBE(maddr);
		uint64_t mmask = ofmatch.get_arp_tha_mask().get_mac();
		MACTOBE(mmask);

		of1x_match_t *match = of1x_init_arp_tha_match(maddr, mmask);
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		caddress value(ofmatch.get_arp_tpa_value());
		caddress mask (ofmatch.get_arp_tpa_mask());

		of1x_match_t *match = of1x_init_arp_tpa_match(
			value.ca_s4addr->sin_addr.s_addr,
			mask.ca_s4addr->sin_addr.s_addr);
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		caddress value(ofmatch.get_ipv6_src_value());
		uint128__t val = value.get_ipv6_addr();
		HTONB128(val);
		caddress mask (ofmatch.get_ipv6_src_mask());
		uint128__t msk = mask.get_ipv6_addr();
		HTONB128(msk);
		
		of1x_match_t *match = of1x_init_ip6_src_match(val, msk);
		of1x_add_match_to_entry(entry,match);
	} catch (...) {}
	
	try {
		caddress value(ofmatch.get_ipv6_dst_value());
		uint128__t val = value.get_ipv6_addr();
		HTONB128(val);
		caddress mask (ofmatch.get_ipv6_dst_mask());
		uint128__t msk = mask.get_ipv6_addr();
		HTONB128(msk);
		
		of1x_match_t *match = of1x_init_ip6_dst_match(val, msk);
		of1x_add_match_to_entry(entry,match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_ip6_flabel_match(htobe32(ofmatch.get_ipv6_flabel()));
		of1x_add_match_to_entry(entry,match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_icmpv6_type_match(ofmatch.get_icmpv6_type());
		of1x_add_match_to_entry(entry,match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_icmpv6_code_match(ofmatch.get_icmpv6_code());
		of1x_add_match_to_entry(entry,match);
	} catch (...) {}

	try {
		caddress value(ofmatch.get_ipv6_nd_target());
		uint128__t val = value.get_ipv6_addr();
		HTONB128(val);
		of1x_match_t *match = of1x_init_ip6_nd_target_match(val);
		of1x_add_match_to_entry(entry,match);
	} catch (...) {}

	try {
		uint64_t mac = ofmatch.get_ipv6_nd_sll().get_mac();
		MACTOBE(mac);
		of1x_match_t *match = of1x_init_ip6_nd_sll_match(mac);
		of1x_add_match_to_entry(entry,match);
	} catch (...) {}

	try {
		uint64_t mac = ofmatch.get_ipv6_nd_tll().get_mac();
		MACTOBE(mac);
		of1x_match_t *match = of1x_init_ip6_nd_tll_match(mac);
		of1x_add_match_to_entry(entry,match);
	} catch (...) {}
#if 0	
	try{

		/*TODO IPV6_EXTHDR*/
		of1x_match_t *match = of1x_init_ip6_exthdr_match(ofmatch.get_ipv6_exthdr());
		of1x_add_match_to_entry(entry,match);

		throw eNotImplemented(std::string("of13_translation_utils::flow_mod_add() rofl::openflow13::OFPXMT_OFB_IPV6_EXTHDR is missing")); // TODO
	}catch (...) {}
#endif	

	try {
		uint32_t label = HTONB32(OF1X_MPLS_LABEL_ALIGN(ofmatch.get_mpls_label()));
		MPLABELTOBE(label);
		of1x_match_t *match = of1x_init_mpls_label_match(label);
		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_mpls_tc_match(OF1X_MPLS_TC_ALIGN(ofmatch.get_mpls_tc()));

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		of1x_match_t *match = of1x_init_mpls_bos_match(ofmatch.get_mpls_bos());

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		uint64_t tunnel_id = htobe64(ofmatch.get_tunnel_id_value());
		uint64_t mask = htobe64(ofmatch.get_tunnel_id_mask());
		of1x_match_t *match = of1x_init_tunnel_id_match(tunnel_id, mask);

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

	try {
		uint32_t pbb_isid = ofmatch.get_pbb_isid_value();
		H24BITTOBE(pbb_isid);
		uint32_t mask = ofmatch.get_pbb_isid_mask();
		H24BITTOBE(mask);
		of1x_match_t *match = of1x_init_pbb_isid_match(pbb_isid, mask);

		of1x_add_match_to_entry(entry, match);
	} catch (...) {}

#if 0
	try {
		/*TODO IPV6_EXTHDR*/
		uint16_t ipv6_exthdr = ofmatch.get_ipv6_exthdr_value();
		uint16_t mask = ofmatch.get_ipv6_exthdr_mask();
		of1x_match_t *match = of1x_init_ipv6_exthdr(ipv6_exthdr, mask);

		of1x_add_match_to_entry(entry, match);
	} catch (rofl::openflow::eOFmatchNotFound& e) {}
#endif

	/* Experimental */
	try {
		rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_code oxm_pppoe_code(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::pppoe::OXM_TLV_EXPR_PPPOE_CODE));

		of1x_match_t *match = of1x_init_pppoe_code_match(oxm_pppoe_code.get_u8value());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_type oxm_pppoe_type(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::pppoe::OXM_TLV_EXPR_PPPOE_TYPE));

		of1x_match_t *match = of1x_init_pppoe_type_match(oxm_pppoe_type.get_u8value());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_sid oxm_pppoe_sid(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::pppoe::OXM_TLV_EXPR_PPPOE_SID));

		of1x_match_t *match = of1x_init_pppoe_session_match(htobe16(oxm_pppoe_sid.get_u16value()));
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::pppoe::coxmatch_ofx_ppp_prot oxm_ppp_prot(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::pppoe::OXM_TLV_EXPR_PPP_PROT));

		of1x_match_t *match = of1x_init_ppp_prot_match(htobe16(oxm_ppp_prot.get_u16value()));
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_msg_type oxm_gtp_msg_type(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::gtp::OXM_TLV_EXPR_GTP_MSGTYPE));

		of1x_match_t *match = of1x_init_gtp_msg_type_match(oxm_gtp_msg_type.get_u8value());
		of1x_add_match_to_entry(entry, match);
	} catch(...) {}

	try {
		rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_teid oxm_gtp_teid(
				ofmatch.get_matches().get_match(rofl::openflow::experimental::gtp::OXM_TLV_EXPR_GTP_TEID));

		of1x_match_t *match = of1x_init_gtp_teid_match(htobe32(oxm_gtp_teid.get_u32value()),htobe32(oxm_gtp_teid.get_u32mask()));

		of1x_add_match_to_entry(entry, match);
	} catch(...) {}
#endif
}



/**
* Maps a of1x_action from an OF1.3 Header
*/
void
of13_translation_utils::of13_map_flow_entry_actions(
		crofctl *ctl,
		openflow_switch* sw, 
		rofl::openflow::cofactions& actions,
		of1x_action_group_t *apply_actions,
		of1x_write_actions_t *write_actions)
{

#if 0
	for (std::list<rofl::openflow::cofaction*>::iterator
			jt = actions.begin(); jt != actions.end(); ++jt)
	{
		rofl::openflow::cofaction& raction = *(*jt);

		of1x_packet_action_t *action = NULL;
		wrap_uint_t field;
		memset(&field,0,sizeof(wrap_uint_t));

		switch (raction.get_type()) {
		case rofl::openflow13::OFPAT_OUTPUT:
			field.u32 = be32toh(raction.oac_12output->port);
			action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, be16toh(raction.oac_12output->max_len));
			break;
		case rofl::openflow13::OFPAT_COPY_TTL_OUT:
			action = of1x_init_packet_action( OF1X_AT_COPY_TTL_OUT, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_COPY_TTL_IN:
			action = of1x_init_packet_action( OF1X_AT_COPY_TTL_IN, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_SET_MPLS_TTL:
			field.u8 = raction.oac_12mpls_ttl->mpls_ttl;
			action = of1x_init_packet_action( OF1X_AT_SET_MPLS_TTL, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_DEC_MPLS_TTL:
			action = of1x_init_packet_action( OF1X_AT_DEC_MPLS_TTL, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_PUSH_VLAN:
			field.u16 = raction.oac_oacu.oacu_12push->ethertype;
			action = of1x_init_packet_action( OF1X_AT_PUSH_VLAN, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_POP_VLAN:
			field.u16 = raction.oac_12push->ethertype;
			action = of1x_init_packet_action( OF1X_AT_POP_VLAN, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_PUSH_MPLS:
			field.u16 = raction.oac_12push->ethertype;
			action = of1x_init_packet_action( OF1X_AT_PUSH_MPLS, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_POP_MPLS:
			field.u16 = raction.oac_12push->ethertype;
			action = of1x_init_packet_action( OF1X_AT_POP_MPLS,  field, 0x0);
			break;
		case rofl::openflow13::OFPAT_SET_QUEUE:
			field.u32 = be32toh(raction.oac_12set_queue->queue_id);
			action = of1x_init_packet_action( OF1X_AT_SET_QUEUE, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_GROUP:
			field.u32 = be32toh(raction.oac_12group->group_id);
			action = of1x_init_packet_action( OF1X_AT_GROUP, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_SET_NW_TTL:
			field.u8 = raction.oac_12nw_ttl->nw_ttl;
			action = of1x_init_packet_action( OF1X_AT_SET_NW_TTL, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_DEC_NW_TTL:
			action = of1x_init_packet_action( OF1X_AT_DEC_NW_TTL, field, 0x0);
			break;
		case rofl::openflow13::OFPAT_SET_FIELD:
		{
			rofl::openflow::coxmatch oxm = raction.get_oxm();

			switch (oxm.get_oxm_class()) {
			case rofl::openflow13::OFPXMC_OPENFLOW_BASIC:
			{
				switch (oxm.get_oxm_field()) {
				case rofl::openflow13::OFPXMT_OFB_ETH_DST:
				{
					field.u64 = oxm.get_u48value().get_mac();
					MACTOBE(field.u64);
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_DST, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_ETH_SRC:
				{
					field.u64 = oxm.get_u48value().get_mac();
					MACTOBE(field.u64);
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_SRC, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_ETH_TYPE:
				{
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_TYPE, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_ARP_OP:
				{
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_OPCODE, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_ARP_SHA:
				{
					field.u64 = oxm.get_u48value().get_mac();
					MACTOBE(field.u64);
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_SHA, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_ARP_SPA:
				{
					field.u32 = htobe32(oxm.get_u32value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_SPA, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_ARP_THA:
				{
					field.u64 = oxm.get_u48value().get_mac();
					MACTOBE(field.u64);
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_THA, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_ARP_TPA:
				{
					field.u32 = htobe32(oxm.get_u32value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_TPA, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_ICMPV4_CODE:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ICMPV4_CODE, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_ICMPV4_TYPE:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ICMPV4_TYPE, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_IPV4_DST:
				{
					field.u32 = htobe32(oxm.get_u32value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IPV4_DST, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_IPV4_SRC:
				{
					field.u32 = htobe32(oxm.get_u32value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IPV4_SRC, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_IP_DSCP:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_DSCP, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_IP_ECN:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_ECN, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_IP_PROTO:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_PROTO, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_MPLS_LABEL:
				{
					field.u32 = HTONB32(OF1X_MPLS_LABEL_ALIGN(oxm.get_u32value()));
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_MPLS_LABEL, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_MPLS_TC:
				{
					field.u8 = OF1X_MPLS_TC_ALIGN(oxm.get_u8value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_MPLS_TC, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_MPLS_BOS:
				{
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_MPLS_BOS, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_VLAN_VID:
				{
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_VID, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_VLAN_PCP:
				{
					field.u8 = OF1X_VLAN_PCP_ALIGN(oxm.get_u8value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_PCP, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_TCP_DST:
				{
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_TCP_DST, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_TCP_SRC:
				{
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_TCP_SRC, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_UDP_DST:
				{
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_UDP_DST, field, 0x0);
				}
					break;
				case rofl::openflow13::OFPXMT_OFB_UDP_SRC:
				{
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_UDP_SRC, field, 0x0);
				}
					break;

				case rofl::openflow13::OFPXMT_OFB_IPV6_SRC: {
					field.u128 = oxm.get_u128value().get_ipv6_addr();
					HTONB128(field.u128);
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_SRC, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_IPV6_DST: {
					field.u128 = oxm.get_u128value().get_ipv6_addr();
					HTONB128(field.u128);
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_DST, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_IPV6_FLABEL: {
					field.u32 = htobe32(oxm.get_u32value());
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_FLABEL, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_IPV6_ND_TARGET: {
					field.u128 = oxm.get_u128value().get_ipv6_addr();
					HTONB128(field.u128);
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_ND_TARGET, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_IPV6_ND_SLL: {
					field.u64 = oxm.get_u48value().get_mac();
					MACTOBE(field.u64);
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_ND_SLL, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_IPV6_ND_TLL: {
					field.u64 = oxm.get_u48value().get_mac();
					MACTOBE(field.u64);
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_ND_TLL, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_ICMPV6_TYPE: {
					//field.u64 = htobe64(oxm.get_u64value()); // FIXME: check required
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_ICMPV6_TYPE, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_ICMPV6_CODE: {
					//field.u64 = htobe64(oxm.get_u64value()); // FIXME: check required
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_ICMPV6_CODE, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_TUNNEL_ID: {
					field.u64 = htobe64(oxm.get_u64value());
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_TUNNEL_ID, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_PBB_ISID: {
					field.u32 = oxm.get_u32value();
					H24BITTOBE(field.u32);
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PBB_ISID, field, 0x0);
				}break;
				case rofl::openflow13::OFPXMT_OFB_IPV6_EXTHDR: {
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_EXTHDR, field, 0x0);
				}break;
				default:
				{
					std::stringstream sstr; sstr << raction;
					ROFL_ERR("of1x_endpoint(%s)::of13_map_flow_entry() "
							"unknown OXM type in action SET-FIELD found: %s",
							sw->dpname.c_str(), sstr.str().c_str());
				}
					break;
				}
			}
				break;
	
			/* Experimental */
			case rofl::openflow13::OFPXMC_EXPERIMENTER: {
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
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPPOE_SID, field, 0x0);
				} break;
				case rofl::openflow::experimental::pppoe::OFPXMT_OFX_PPP_PROT: {
					field.u16 = htobe16(oxm.get_u16value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPP_PROT, field, 0x0);
				} break;
				case rofl::openflow::experimental::gtp::OFPXMT_OFX_GTP_MSGTYPE: {
					field.u8 = oxm.get_u8value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_GTP_MSG_TYPE, field, 0x0);
				} break;
				case rofl::openflow::experimental::gtp::OFPXMT_OFX_GTP_TEID: {
					field.u32 = htobe32(oxm.get_u32value());
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_GTP_TEID, field, 0x0);
				} break;
				}

			}
				break;
			/* End of experimental */
			default:
			{
				std::stringstream sstr; sstr << raction;
				ROFL_ERR("of1x_endpoint(%s)::of13_map_flow_entry() "
						"unknown OXM class in action SET-FIELD found: %s",
						sw->dpname.c_str(), sstr.str().c_str());
			}
				break;
			}
		}
			break;
		case rofl::openflow13::OFPAT_EXPERIMENTER: {

			rofl::openflow::cofaction_experimenter eaction(raction);

			switch (eaction.get_exp_id()) {
			case ROFL_EXPERIMENTER_ID: {
				/* Experimental */
				/*
				 * but one does not have to, PPPoE still uses a different body definition
				 */
				// ROFL experimental actions contain experimental action type at position data[0]
				uint8_t acttype = eaction.oac_12experimenter->data[0];

				switch (acttype) {
				case rofl::openflow::cofaction_push_pppoe::OFXAT_PUSH_PPPOE: {
					rofl::openflow::cofaction_push_pppoe paction(eaction);
					field.u16 = be16toh(paction.eoac_push_pppoe->expbody.ethertype);
					action = of1x_init_packet_action( OF1X_AT_PUSH_PPPOE, field, 0x0);
				} break;
				case rofl::openflow::cofaction_pop_pppoe::OFXAT_POP_PPPOE: {
					rofl::openflow::cofaction_pop_pppoe paction(eaction);
					field.u16 = be16toh(paction.eoac_pop_pppoe->expbody.ethertype);
					action = of1x_init_packet_action( OF1X_AT_POP_PPPOE, field, 0x0);
				} break;
				}
				/* End of experimental */

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
#endif
}



/*
* Maps a of1x_action TO an OF1.3 Header
*/
void
of13_translation_utils::of13_map_reverse_flow_entry_matches(
		of1x_match_t* m,
		rofl::openflow::cofmatch& match)
{

#if 0
	while (NULL != m)
	{
		switch (m->type) {
		case OF1X_MATCH_IN_PORT:
			match.set_in_port(m->value->value.u32);
			break;
		case OF1X_MATCH_IN_PHY_PORT:
			match.set_in_phy_port(m->value->value.u32);
			break;
		case OF1X_MATCH_METADATA:
			match.set_metadata(m->value->value.u64, m->value->mask.u64);
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
			if(m->vlan_present == OF1X_MATCH_VLAN_SPECIFIC) {
				match.set_vlan_vid(be16toh(m->value->value.u16), be16toh(m->value->mask.u16));
			}
			if(m->vlan_present == OF1X_MATCH_VLAN_NONE) {
				match.set_vlan_vid(rofl::openflow13::OFPVID_NONE);
			}
			if(m->vlan_present == OF1X_MATCH_VLAN_ANY) {
				match.set_vlan_vid(rofl::openflow13::OFPVID_PRESENT, rofl::openflow13::OFPVID_PRESENT);
			}
			break;
		case OF1X_MATCH_VLAN_PCP:
			match.set_vlan_pcp(OF1X_VLAN_PCP_VALUE(m->value->value.u8));
			break;
		case OF1X_MATCH_ARP_OP:
			match.set_arp_opcode(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_ARP_SHA:
		{
			uint64_t mac = m->value->value.u64;
			BETOHMAC(mac);
			uint64_t msk = m->value->mask.u64;
			BETOHMAC(msk);
			match.set_arp_sha(cmacaddr(mac), cmacaddr(msk));
		}
			break;
		case OF1X_MATCH_ARP_SPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = m->value->value.u32;
			caddress mask(AF_INET, "0.0.0.0");
			mask.ca_s4addr->sin_addr.s_addr = m->value->mask.u32;
			match.set_arp_spa(addr, mask);
		}
			break;
		case OF1X_MATCH_ARP_THA:
		{
			uint64_t mac = m->value->value.u64;
			BETOHMAC(mac);
			uint64_t msk = m->value->mask.u64;
			BETOHMAC(msk);
			match.set_arp_tha(cmacaddr(mac), cmacaddr(msk));
		}
			break;
		case OF1X_MATCH_ARP_TPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = m->value->value.u32;
			caddress mask(AF_INET, "0.0.0.0");
			mask.ca_s4addr->sin_addr.s_addr = m->value->mask.u32;
			match.set_arp_tpa(addr, mask);
		}
			break;
		case OF1X_MATCH_IP_DSCP:
			match.set_ip_dscp(m->value->value.u8);
			break;
		case OF1X_MATCH_IP_ECN:
			match.set_ip_ecn(m->value->value.u8);
			break;
		case OF1X_MATCH_IP_PROTO:
			match.set_ip_proto(m->value->value.u8);
			break;
		case OF1X_MATCH_IPV4_SRC:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = m->value->value.u32;
			caddress mask(AF_INET, "0.0.0.0");
			mask.ca_s4addr->sin_addr.s_addr = m->value->mask.u32;
			match.set_ipv4_src(addr, mask);
		}
			break;
		case OF1X_MATCH_IPV4_DST:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = m->value->value.u32;
			caddress mask(AF_INET, "0.0.0.0");
			mask.ca_s4addr->sin_addr.s_addr = m->value->mask.u32;
			match.set_ipv4_dst(addr, mask);
		}
			break;
		case OF1X_MATCH_TCP_SRC:
			match.set_tcp_src(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_TCP_DST:
			match.set_tcp_dst(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_UDP_SRC:
			match.set_udp_src(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_UDP_DST:
			match.set_udp_dst(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_SCTP_SRC:
			match.set_sctp_src(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_SCTP_DST:
			match.set_sctp_dst(be16toh(m->value->value.u16));
			break;
		case OF1X_MATCH_ICMPV4_TYPE:
			match.set_icmpv4_type(m->value->value.u8);
			break;
		case OF1X_MATCH_ICMPV4_CODE:
			match.set_icmpv4_code(m->value->value.u8);
			break;
		case OF1X_MATCH_IPV6_SRC: {
			/*TODO deal with endianess??*/
			caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
			memcpy(&(addr.ca_s6addr->sin6_addr.__in6_u.__u6_addr8), &(m->value->value.u128.val), sizeof(uint128__t));
			NTOHB128(addr.ca_s6addr->sin6_addr.__in6_u.__u6_addr8);
			caddress mask(AF_INET6,"0:0:0:0:0:0:0:0");
			memcpy(&(mask.ca_s6addr->sin6_addr.__in6_u.__u6_addr8), &(m->value->mask.u128.val), sizeof(uint128__t));
			NTOHB128(mask.ca_s6addr->sin6_addr.__in6_u.__u6_addr8);
			match.set_ipv6_src(addr, mask);
			}break;
		case OF1X_MATCH_IPV6_DST:{
			/*TODO deal with endianess??*/
			caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
			memcpy(&(addr.ca_s6addr->sin6_addr.__in6_u.__u6_addr8), &(m->value->value.u128.val), sizeof(uint128__t));
			NTOHB128(addr.ca_s6addr->sin6_addr.__in6_u.__u6_addr8);
			caddress mask(AF_INET6,"0:0:0:0:0:0:0:0");
			memcpy(&(mask.ca_s6addr->sin6_addr.__in6_u.__u6_addr8), &(m->value->mask.u128.val), sizeof(uint128__t));
			NTOHB128(mask.ca_s6addr->sin6_addr.__in6_u.__u6_addr8);
			match.set_ipv6_dst(addr, mask);
			}break;
		case OF1X_MATCH_IPV6_FLABEL:
			//match.set_ipv6_flabel(be64toh(m->value->value.u64)); // FIXME: check required
			match.set_ipv6_flabel(be32toh(m->value->value.u32));
			break;
		case OF1X_MATCH_ICMPV6_TYPE:
			//match.set_icmpv6_type(be64toh(m->value->value.u64)); // FIXME: check required
			match.set_icmpv6_type(m->value->value.u8);
			break;
		case OF1X_MATCH_ICMPV6_CODE:
			//match.set_icmpv6_code(be64toh(m->value->value.u64)); // FIXME: check required
			match.set_icmpv6_code(m->value->value.u8);
			break;
		case OF1X_MATCH_IPV6_ND_TARGET:{
			caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
			/*TODO deal with endianess??*/
			memcpy(&(addr.ca_s6addr->sin6_addr.__in6_u.__u6_addr8), &(m->value->value.u128.val),sizeof(uint128__t));
			NTOHB128(addr.ca_s6addr->sin6_addr.__in6_u.__u6_addr8);
			match.set_ipv6_nd_target(addr);
			}break;
		case OF1X_MATCH_IPV6_ND_SLL:{
			//match.set_ipv6_nd_sll(be64toh(m->value->value.u64)); // FIXME: check required
			uint64_t mac = m->value->value.u64;
			BETOHMAC(mac);
			match.set_ipv6_nd_sll(cmacaddr(mac));
			}break;
		case OF1X_MATCH_IPV6_ND_TLL:{
			//match.set_ipv6_nd_tll(be64toh(m->value->value.u64)); // FIXME: check required
			uint64_t mac = m->value->value.u64;
			BETOHMAC(mac);
			match.set_ipv6_nd_tll(cmacaddr(mac));
			}break;
		case OF1X_MATCH_MPLS_LABEL:{
			uint32_t label = OF1X_MPLS_LABEL_VALUE(NTOHB32(m->value->value.u32));
			match.set_mpls_label(label);
			}break;
		case OF1X_MATCH_MPLS_TC:
			match.set_mpls_tc(OF1X_MPLS_TC_VALUE(m->value->value.u8));
			break;
		case OF1X_MATCH_MPLS_BOS:
			match.set_mpls_bos(m->value->value.u8);
			break;
		case OF1X_MATCH_TUNNEL_ID:
			match.set_tunnel_id(be64toh(m->value->value.u64), be64toh(m->value->mask.u64));
			break;
		case OF1X_MATCH_PBB_ISID:{
			uint32_t value = m->value->value.u32;
			BETOH24BIT(value);
			uint32_t mask = m->value->mask.u32;
			BETOH24BIT(mask);
			match.set_pbb_isid(value, mask);
			}break;
		case OF1X_MATCH_IPV6_EXTHDR:
			match.set_ipv6_exthdr(be16toh(m->value->value.u16), be16toh(m->value->mask.u16));
			break;
				
		/* Experimental */
		case OF1X_MATCH_PPPOE_CODE:
			match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_code(m->value->value.u8));
			break;
		case OF1X_MATCH_PPPOE_TYPE:
			match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_type(m->value->value.u8));
			break;
		case OF1X_MATCH_PPPOE_SID:
			match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_sid(be16toh(m->value->value.u16)));
			break;
		case OF1X_MATCH_PPP_PROT:
			match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_ppp_prot(be16toh(m->value->value.u16)));
			break;
		case OF1X_MATCH_GTP_MSG_TYPE:
			match.set_matches().add_match(rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_msg_type(m->value->value.u8));
			break;
		case OF1X_MATCH_GTP_TEID:
			match.set_matches().add_match(rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_teid(be32toh(m->value->value.u32)));
			break;
		default:
			break;
		}


		m = m->next;
	}
#endif
}

/**
* Maps a of1x_group_bucket from an OF1.3 Header
*/
void
of13_translation_utils::of13_map_bucket_list(
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
		
		of13_map_flow_entry_actions(ctl,sw,bucket_ptr.set_actions(),action_group,NULL);
		of1x_insert_bucket_in_list(bucket_list,of1x_init_bucket(bucket_ptr.get_weight(), bucket_ptr.get_watch_port(), bucket_ptr.get_watch_group(), action_group));
	}
}

void of13_translation_utils::of13_map_reverse_bucket_list(
		rofl::openflow::cofbuckets& of_buckets,
		of1x_bucket_list_t* bucket_list){
	
	uint32_t bucket_id = 0;

	for(of1x_bucket_t *bu_it=bucket_list->head;bu_it;bu_it=bu_it->next){
		//cofbucket single_bucket;
		rofl::openflow::cofactions ac_list(rofl::openflow13::OFP_VERSION);
		for (of1x_packet_action_t *action_it = bu_it->actions->head; action_it != NULL; action_it = action_it->next) {
			if (OF1X_AT_NO_ACTION == action_it->type)
				continue;
			rofl::openflow::cofaction action(rofl::openflow13::OFP_VERSION);
			of13_map_reverse_flow_entry_action(action_it, action);
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
of13_translation_utils::of13_map_reverse_flow_entry_instructions(
		of1x_instruction_group_t* group,
		rofl::openflow::cofinstructions& instructions)
{
	for (unsigned int i = 0; i < (sizeof(group->instructions) / sizeof(of1x_instruction_t)); i++) {
		if (OF1X_IT_NO_INSTRUCTION == group->instructions[i].type)
			continue;
		rofl::openflow::cofinst instruction(rofl::openflow13::OFP_VERSION);
		of13_map_reverse_flow_entry_instruction(&(group->instructions[i]), instruction);
		instructions.add_inst(instruction);
	}
}


void
of13_translation_utils::of13_map_reverse_flow_entry_instruction(
		of1x_instruction_t* inst,
		rofl::openflow::cofinst& instruction)
{
	switch (inst->type) {
	case OF1X_IT_APPLY_ACTIONS: {
		instruction = rofl::openflow::cofinst_apply_actions(rofl::openflow13::OFP_VERSION);
		for (of1x_packet_action_t *of1x_action = inst->apply_actions->head; of1x_action != NULL; of1x_action = of1x_action->next) {
			if (OF1X_AT_NO_ACTION == of1x_action->type)
				continue;
			rofl::openflow::cofaction action(rofl::openflow13::OFP_VERSION);
			of13_map_reverse_flow_entry_action(of1x_action, action);
			instruction.get_actions().append_action(action);
		}
	} break;
	case OF1X_IT_CLEAR_ACTIONS: {
		instruction = rofl::openflow::cofinst_clear_actions(rofl::openflow13::OFP_VERSION);
	} break;
	case OF1X_IT_WRITE_ACTIONS: {
		instruction = rofl::openflow::cofinst_write_actions(rofl::openflow13::OFP_VERSION);
		for (unsigned int i = 0; i < inst->write_actions->num_of_actions; i++) {
			if (OF1X_AT_NO_ACTION == inst->write_actions->actions[i].type)
				continue;
			rofl::openflow::cofaction action(rofl::openflow13::OFP_VERSION);
			of13_map_reverse_flow_entry_action(&(inst->write_actions->actions[i]), action);
			instruction.get_actions().append_action(action);
		}
	} break;
	case OF1X_IT_WRITE_METADATA:
	case OF1X_IT_EXPERIMENTER: {
		// TODO: both are marked TODO in of1x_pipeline
	} break;
	case OF1X_IT_GOTO_TABLE: {
		instruction = rofl::openflow::cofinst_goto_table(rofl::openflow13::OFP_VERSION, inst->go_to_table);
	} break;
	case OF1X_IT_METER: {
		instruction = rofl::openflow::cofinst_meter(rofl::openflow13::OFP_VERSION); //TODO: meter-id
	} break;
	default: {
		// do nothing
	} break;
	}
}


void
of13_translation_utils::of13_map_reverse_flow_entry_action(
		of1x_packet_action_t* of1x_action,
		rofl::openflow::cofaction& action)
{

#if 0
	/*
	 * FIXME: add masks for those fields defining masked values in the specification
	 */


	switch (of1x_action->type) {
	case OF1X_AT_NO_ACTION: {
		// do nothing
	} break;
	case OF1X_AT_COPY_TTL_IN: {
		action = rofl::openflow::cofaction_copy_ttl_in(rofl::openflow13::OFP_VERSION);
	} break;
	case OF1X_AT_POP_VLAN: {
		action = rofl::openflow::cofaction_pop_vlan(rofl::openflow13::OFP_VERSION);
	} break;
	case OF1X_AT_POP_MPLS: {
		action = rofl::openflow::cofaction_pop_mpls(rofl::openflow13::OFP_VERSION, be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	/* Experimental */
	case OF1X_AT_POP_PPPOE: {
		action = rofl::openflow::cofaction_pop_pppoe(rofl::openflow13::OFP_VERSION, be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_PUSH_PPPOE: {
		action = rofl::openflow::cofaction_push_pppoe(rofl::openflow13::OFP_VERSION, be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	/* End of experimental */
	case OF1X_AT_PUSH_MPLS: {
		action = rofl::openflow::cofaction_push_mpls(rofl::openflow13::OFP_VERSION, be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_PUSH_VLAN: {
		action = rofl::openflow::cofaction_push_vlan(rofl::openflow13::OFP_VERSION, be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_COPY_TTL_OUT: {
		action = rofl::openflow::cofaction_copy_ttl_out(rofl::openflow13::OFP_VERSION);
	} break;
	case OF1X_AT_DEC_NW_TTL: {
		action = rofl::openflow::cofaction_dec_nw_ttl(rofl::openflow13::OFP_VERSION);
	} break;
	case OF1X_AT_DEC_MPLS_TTL: {
		action = rofl::openflow::cofaction_dec_mpls_ttl(rofl::openflow13::OFP_VERSION);
	} break;
	case OF1X_AT_SET_MPLS_TTL: {
		action = rofl::openflow::cofaction_set_mpls_ttl(rofl::openflow13::OFP_VERSION, (uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK));
	} break;
	case OF1X_AT_SET_NW_TTL: {
		action = rofl::openflow::cofaction_set_nw_ttl(rofl::openflow13::OFP_VERSION, (uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK));
	} break;
	case OF1X_AT_SET_QUEUE: {
		action = rofl::openflow::cofaction_set_queue(rofl::openflow13::OFP_VERSION, (uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK));
	} break;
	//case OF1X_AT_SET_FIELD_METADATA:
	case OF1X_AT_SET_FIELD_ETH_DST: {
		uint64_t mac = of1x_action->field.u64;
		BETOHMAC(mac);
		cmacaddr maddr(mac);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_eth_dst(maddr));
	} break;
	case OF1X_AT_SET_FIELD_ETH_SRC: {
		uint64_t mac = of1x_action->field.u64;
		BETOHMAC(mac);
		cmacaddr maddr(mac);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_eth_src(maddr));
	} break;
	case OF1X_AT_SET_FIELD_ETH_TYPE: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_eth_type(be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_VLAN_VID: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_vlan_vid(be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_VLAN_PCP: {
		uint8_t pcp = OF1X_VLAN_PCP_VALUE((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK));
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_vlan_pcp(pcp));
	} break;
	case OF1X_AT_SET_FIELD_ARP_OPCODE: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_opcode(be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_ARP_SHA: {
		uint64_t mac = of1x_action->field.u64;
		BETOHMAC(mac);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_sha(cmacaddr(mac)));
	} break;
	case OF1X_AT_SET_FIELD_ARP_SPA: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_spa(be32toh((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_ARP_THA: {
		uint64_t mac = of1x_action->field.u64;
		BETOHMAC(mac);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_tha(cmacaddr(mac)));
	} break;
	case OF1X_AT_SET_FIELD_ARP_TPA: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_arp_tpa(be32toh((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_IP_DSCP: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ip_dscp((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_IP_ECN: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ip_ecn((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_IP_PROTO: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ip_proto((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_IPV4_SRC: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv4_src(be32toh((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_IPV4_DST: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv4_dst(be32toh((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_TCP_SRC: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_tcp_src(be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_TCP_DST: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_tcp_dst(be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_UDP_SRC: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_udp_src(be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_UDP_DST: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_udp_dst(be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_ICMPV4_TYPE: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_icmpv4_type((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_ICMPV4_CODE: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_icmpv4_code((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	
	case OF1X_AT_SET_FIELD_IPV6_SRC: {
		uint128__t value = of1x_action->field.u128;
		NTOHB128(value);
		rofl::caddress addr(AF_INET6);
		memcpy(addr.ca_s6addr->sin6_addr.s6_addr, (uint8_t*)(value.val), 16);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_src(addr));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_DST: {
		uint128__t value = of1x_action->field.u128;
		NTOHB128(value);
		rofl::caddress addr(AF_INET6);
		memcpy(addr.ca_s6addr->sin6_addr.s6_addr, (uint8_t*)(value.val), 16);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_dst(addr));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_FLABEL: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_flabel(be32toh((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_ND_TARGET: {
		uint128__t value = of1x_action->field.u128;
		NTOHB128(value);
		rofl::caddress addr(AF_INET6);
		memcpy(addr.ca_s6addr->sin6_addr.s6_addr, (uint8_t*)(value.val), 16);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_nd_target(addr));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_ND_SLL: {
		uint64_t mac = of1x_action->field.u64;
		BETOHMAC(mac);
		cmacaddr maddr(mac);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_nd_sll(maddr));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_ND_TLL: {
		uint64_t mac = of1x_action->field.u64;
		BETOHMAC(mac);
		cmacaddr maddr(mac);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_ipv6_nd_tll(maddr));
	} break;
	case OF1X_AT_SET_FIELD_ICMPV6_TYPE: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_icmpv6_type((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_ICMPV6_CODE: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_icmpv6_code((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_MPLS_LABEL: {
		uint32_t label = OF1X_MPLS_LABEL_VALUE(NTOHB32(of1x_action->field.u32));
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_mpls_label(label & OF1X_4_BYTE_MASK));
	} break;
	case OF1X_AT_SET_FIELD_MPLS_TC: {
		uint8_t tc = OF1X_MPLS_TC_VALUE((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK));
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_mpls_tc(tc));
	} break;
	case OF1X_AT_SET_FIELD_MPLS_BOS: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_mpls_bos((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_TUNNEL_ID: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_tunnel_id(be64toh((uint64_t)(of1x_action->field.u64 & OF1X_8_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_PBB_ISID: {
		uint32_t value = of1x_action->field.u32;
		BETOH24BIT(value);
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::coxmatch_ofb_pbb_isid(value & OF1X_3_BYTE_MASK));
	} break;

	/*TODO EXT HDR*/
	case OF1X_AT_SET_FIELD_IPV6_EXTHDR:
		throw eNotImplemented(std::string("of13_translation_utils::of13_map_reverse_flow_entry_action() IPV6 ICMPV6"));
		break;

	/* Experimental */
	case OF1X_AT_SET_FIELD_PPPOE_CODE: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_code((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_PPPOE_TYPE: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_type((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_PPPOE_SID: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_sid(be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_PPP_PROT: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::experimental::pppoe::coxmatch_ofx_ppp_prot(be16toh((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK))));
	} break;
	case OF1X_AT_SET_FIELD_GTP_MSG_TYPE: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_msg_type((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_GTP_TEID: {
		action = rofl::openflow::cofaction_set_field(rofl::openflow13::OFP_VERSION, rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_teid(be32toh((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK))));
	} break;
	/* End of experimental section */
	case OF1X_AT_GROUP: {
		action = rofl::openflow::cofaction_group(rofl::openflow13::OFP_VERSION, (uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK));
	} break;
	case OF1X_AT_EXPERIMENTER: {
		// TODO
	} break;
	case OF1X_AT_OUTPUT: {
		action = rofl::openflow::cofaction_output(rofl::openflow13::OFP_VERSION, (uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK), of1x_action->send_len);
	} break;
	default: {
		// do nothing
	} break;
	}

#endif
}


/*
* Maps packet actions to cofmatches
*/

void of13_translation_utils::of13_map_reverse_packet_matches(packet_matches_t* packet_matches, rofl::openflow::cofmatch& match){

#if 0
	if(packet_matches->port_in)
		match.set_in_port(packet_matches->port_in);
	if(packet_matches->phy_port_in)
		match.set_in_phy_port(packet_matches->phy_port_in);
	if(packet_matches->metadata)
		match.set_metadata(packet_matches->metadata);
	if(packet_matches->eth_dst){
		uint64_t mac = packet_matches->eth_dst;
		BETOHMAC(mac);
		match.set_eth_dst(cmacaddr(mac));
	}
	if(packet_matches->eth_src){
		uint64_t mac = packet_matches->eth_src;
		BETOHMAC(mac);
		match.set_eth_src(cmacaddr(mac));
	}
	if(packet_matches->eth_type)
		match.set_eth_type(be16toh(packet_matches->eth_type));
	if(packet_matches->vlan_vid)
		match.set_vlan_vid(be16toh(packet_matches->vlan_vid));
	if(packet_matches->vlan_pcp)
		match.set_vlan_pcp(OF1X_VLAN_PCP_VALUE(packet_matches->vlan_pcp));
	if(packet_matches->arp_opcode)
		match.set_arp_opcode(be16toh(packet_matches->arp_opcode));
	if(packet_matches->arp_sha){
		uint64_t mac = packet_matches->arp_sha;
		BETOHMAC(mac);
		match.set_arp_sha( cmacaddr(mac) );
	}
	if(packet_matches->arp_spa) {
		caddress addr(AF_INET, "0.0.0.0");
		addr.ca_s4addr->sin_addr.s_addr = packet_matches->arp_spa;
		match.set_arp_spa(addr);
	}
	if(packet_matches->arp_tha){
		uint64_t mac = packet_matches->arp_tha;
		BETOHMAC(mac);
		match.set_arp_tha(cmacaddr(mac));
	}
	if(packet_matches->arp_tpa) {
		caddress addr(AF_INET, "0.0.0.0");
		addr.ca_s4addr->sin_addr.s_addr = packet_matches->arp_tpa;
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
			addr.ca_s4addr->sin_addr.s_addr = packet_matches->ipv4_src;
			match.set_ipv4_src(addr);

	}
	if(packet_matches->ipv4_dst){
		caddress addr(AF_INET, "0.0.0.0");
		addr.ca_s4addr->sin_addr.s_addr = packet_matches->ipv4_dst;
		match.set_ipv4_dst(addr);
	}
	if(packet_matches->tcp_src)
		match.set_tcp_src(be16toh(packet_matches->tcp_src));
	if(packet_matches->tcp_dst)
		match.set_tcp_dst(be16toh(packet_matches->tcp_dst));
	if(packet_matches->udp_src)
		match.set_udp_src(be16toh(packet_matches->udp_src));
	if(packet_matches->udp_dst)
		match.set_udp_dst(be16toh(packet_matches->udp_dst));
	if(packet_matches->icmpv4_type)
		match.set_icmpv4_type(packet_matches->icmpv4_type);
	if(packet_matches->icmpv4_code)
		match.set_icmpv4_code(packet_matches->icmpv4_code);
		
	if( UINT128__T_HI(packet_matches->ipv6_src) || UINT128__T_LO(packet_matches->ipv6_src) ){
		caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
		uint128__t addru128 = packet_matches->ipv6_src;
		NTOHB128(addru128);
		addr.set_ipv6_addr(addru128);
		match.set_ipv6_src(addr);
	}
	if( UINT128__T_HI(packet_matches->ipv6_dst) || UINT128__T_LO(packet_matches->ipv6_dst) ){
		caddress addr(AF_INET6,"0:0:0:0");
		uint128__t addru128 = packet_matches->ipv6_dst;
		NTOHB128(addru128);
		addr.set_ipv6_addr(addru128);
		match.set_ipv6_dst(addr);
	}
	if(packet_matches->ipv6_flabel)
		//match.set_ipv6_flabel(be64toh(packet_matches->ipv6_flabel)); // FIXME: check required => packet_matches->ipv6_flabel is u64 in pipeline?
		match.set_ipv6_flabel(be32toh((uint32_t)packet_matches->ipv6_flabel));
	if( UINT128__T_HI(packet_matches->ipv6_nd_target) || UINT128__T_LO(packet_matches->ipv6_nd_target) ){
		caddress addr(AF_INET6,"0:0:0:0");
		uint128__t addru128 = packet_matches->ipv6_nd_target;
		NTOHB128(addru128);
		addr.set_ipv6_addr(addru128);
		match.set_ipv6_nd_target(addr);
	}
	if(packet_matches->ipv6_nd_sll){
		uint64_t mac = packet_matches->ipv6_nd_sll;
		BETOHMAC(mac);
		match.set_ipv6_nd_sll(rofl::cmacaddr(mac));
	}
	if(packet_matches->ipv6_nd_tll){
		uint64_t mac = packet_matches->ipv6_nd_tll;
		BETOHMAC(mac);
		match.set_ipv6_nd_tll(rofl::cmacaddr(mac));
	}
	if(packet_matches->icmpv6_type)
		match.set_icmpv6_type(packet_matches->icmpv6_type);
	if(packet_matches->icmpv6_code)
		match.set_icmpv6_code(packet_matches->icmpv6_code);
		
	if(packet_matches->mpls_label){
		uint32_t label = OF1X_MPLS_LABEL_VALUE(NTOHB32(packet_matches->mpls_label));
		match.set_mpls_label(label);
	}
	if(packet_matches->mpls_tc)
		match.set_mpls_tc(OF1X_MPLS_TC_VALUE(packet_matches->mpls_tc));

	if(packet_matches->mpls_bos)
		match.set_mpls_bos(packet_matches->mpls_bos);
	if(packet_matches->tunnel_id)
		match.set_tunnel_id(be64toh(packet_matches->tunnel_id));
	if(packet_matches->pbb_isid){
		uint32_t value=packet_matches->pbb_isid;
		BETOH24BIT(value);
		match.set_pbb_isid(value);
	}
	if(packet_matches->ipv6_exthdr)
		match.set_ipv6_exthdr(be16toh(packet_matches->ipv6_exthdr));

	/* Experimental */
	if(packet_matches->pppoe_code)
		match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_code(packet_matches->pppoe_code));
	if(packet_matches->pppoe_type)
		match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_type(packet_matches->pppoe_type));
	if(packet_matches->pppoe_sid)
		match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_pppoe_sid(be16toh(packet_matches->pppoe_sid)));
	if(packet_matches->ppp_proto)
		match.set_matches().add_match(rofl::openflow::experimental::pppoe::coxmatch_ofx_ppp_prot(be16toh(packet_matches->ppp_proto)));
	if(packet_matches->gtp_msg_type)
		match.set_matches().add_match(rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_msg_type(packet_matches->gtp_msg_type));
	if(packet_matches->gtp_teid)
		match.set_matches().add_match(rofl::openflow::experimental::gtp::coxmatch_ofx_gtp_teid(be32toh(packet_matches->gtp_teid)));

#endif
}

/*
* Table capability bitmap
*/

void of13_translation_utils::of13_map_bitmap_matches(bitmap128_t* bitmap, rofl::openflow::coftable_feature_prop_oxm& matches)
{

#if 0
	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IN_PORT))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IN_PORT);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IN_PHY_PORT))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IN_PHY_PORT);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_METADATA))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_METADATA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ETH_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ETH_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ETH_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ETH_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ETH_TYPE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ETH_TYPE);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_VLAN_VID))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_VLAN_VID);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_VLAN_PCP))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_VLAN_PCP);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_MPLS_LABEL))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_MPLS_LABEL);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_MPLS_BOS))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_MPLS_BOS);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_MPLS_TC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_MPLS_TC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_OP))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_OP);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_SPA))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_SPA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_TPA))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_TPA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_SHA))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_SHA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_THA))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_THA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IP_DSCP))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IP_DSCP);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IP_ECN))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IP_ECN);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IP_PROTO))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IP_PROTO);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV4_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV4_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV4_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV4_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_FLABEL))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_FLABEL);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ICMPV6_TYPE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ICMPV6_TYPE);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ICMPV6_CODE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ICMPV6_CODE);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_ND_TARGET))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_ND_TARGET);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_ND_SLL))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_ND_SLL);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_ND_TLL))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_ND_TLL);
#if 0
	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_EXTHDR))
		mapped_bitmap |= ( UINT64_C(1) <<  rofl::openflow13::OFPXMT_OFB_IPV6_EXTHDR);
#endif
	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_TCP_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_TCP_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_TCP_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_TCP_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_UDP_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_UDP_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_UDP_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_UDP_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_SCTP_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_SCTP_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_SCTP_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_SCTP_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ICMPV4_TYPE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ICMPV4_TYPE);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ICMPV4_CODE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ICMPV4_CODE);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_PBB_ISID))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_PBB_ISID);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_TUNNEL_ID))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_TUNNEL_ID);

#endif
}

void of13_translation_utils::of13_map_bitmap_set_fields(bitmap128_t* bitmap, rofl::openflow::coftable_feature_prop_oxm& matches)
{

#if 0
	if (*bitmap & (1UL << OF1X_MATCH_ETH_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ETH_DST);

	if (*bitmap & (1UL << OF1X_MATCH_ETH_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ETH_SRC);

	if (*bitmap & (1UL << OF1X_MATCH_ETH_TYPE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ETH_TYPE);

	if (*bitmap & (1UL << OF1X_MATCH_MPLS_LABEL))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_MPLS_LABEL);

	if (*bitmap & (1UL << OF1X_MATCH_MPLS_TC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_MPLS_TC);

	if (*bitmap & (1UL << OF1X_MATCH_MPLS_BOS))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_MPLS_BOS);

	if (*bitmap & (1UL << OF1X_MATCH_VLAN_VID))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_VLAN_VID);

	if (*bitmap & (1UL << OF1X_MATCH_VLAN_PCP))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_VLAN_PCP);

	if (*bitmap & (1UL << OF1X_MATCH_ARP_OP))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_OP);

	if (*bitmap & (1UL << OF1X_MATCH_ARP_SHA))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_SHA);

	if (*bitmap & (1UL << OF1X_MATCH_ARP_SPA))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_SPA);

	if (*bitmap & (1UL << OF1X_MATCH_ARP_THA))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_THA);

	if (*bitmap & (1UL << OF1X_MATCH_ARP_TPA))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ARP_TPA);

	if (*bitmap & (1UL << OF1X_MATCH_IP_DSCP))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IP_DSCP);

	if (*bitmap & (1UL << OF1X_MATCH_IP_ECN))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IP_ECN);

	if (*bitmap & (1UL << OF1X_MATCH_IP_PROTO))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IP_PROTO);

	if (*bitmap & (1UL << OF1X_MATCH_IPV4_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV4_SRC);

	if (*bitmap & (1UL << OF1X_MATCH_IPV4_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV4_DST);

	if (*bitmap & (1UL << OF1X_MATCH_IPV6_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_SRC);

	if (*bitmap & (1UL << OF1X_MATCH_IPV6_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_DST);

	if (*bitmap & (1UL << OF1X_MATCH_IPV6_FLABEL))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_FLABEL);

	if (*bitmap & (1UL << OF1X_MATCH_IPV6_ND_TARGET))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_ND_TARGET);

	if (*bitmap & (1UL << OF1X_MATCH_IPV6_ND_SLL))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_ND_SLL);

	if (*bitmap & (1UL << OF1X_MATCH_IPV6_ND_TLL))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_ND_TLL);

	if (*bitmap & (1UL << OF1X_MATCH_IPV6_EXTHDR))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_IPV6_EXTHDR);

	if (*bitmap & (1UL << OF1X_MATCH_TCP_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_TCP_SRC);

	if (*bitmap & (1UL << OF1X_MATCH_TCP_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_TCP_DST);

	if (*bitmap & (1UL << OF1X_MATCH_UDP_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_UDP_SRC);

	if (*bitmap & (1UL << OF1X_MATCH_UDP_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_UDP_DST);

	if (*bitmap & (1UL << OF1X_MATCH_SCTP_SRC))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_SCTP_SRC);

	if (*bitmap & (1UL << OF1X_MATCH_SCTP_DST))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_SCTP_DST);

	if (*bitmap & (1UL << OF1X_MATCH_ICMPV4_TYPE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ICMPV4_TYPE);

	if (*bitmap & (1UL << OF1X_MATCH_ICMPV4_CODE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ICMPV4_CODE);

	if (*bitmap & (1UL << OF1X_MATCH_ICMPV6_TYPE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ICMPV6_TYPE);

	if (*bitmap & (1UL << OF1X_MATCH_ICMPV6_CODE))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_ICMPV6_CODE);

	if (*bitmap & (1UL << OF1X_MATCH_PBB_ISID))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_PBB_ISID);

	if (*bitmap & (1UL << OF1X_MATCH_TUNNEL_ID))
		matches.add_oxm(rofl::openflow::OXM_TLV_BASIC_TUNNEL_ID);

#endif
}


void of13_translation_utils::of13_map_bitmap_actions(bitmap128_t *bitmap, rofl::openflow::coftable_feature_prop_actions& actions)
{
	
#if 0
	if (*bitmap & (1 << rofl::openflow13::OFPAT_COPY_TTL_IN))
		actions.add_action(rofl::openflow13::OFPAT_COPY_TTL_IN, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_POP_VLAN))
		actions.add_action(rofl::openflow13::OFPAT_POP_VLAN, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_POP_MPLS))
		actions.add_action(rofl::openflow13::OFPAT_POP_MPLS, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_POP_PBB))
		actions.add_action(rofl::openflow13::OFPAT_POP_PBB, 4);

	// TODO: POP_PPPOE
	// TODO: POP_GTP

	// TODO: PUSH_GTP
	// TODO: PUSH_PPPOE

	if (*bitmap & (1 << rofl::openflow13::OFPAT_PUSH_PBB))
		actions.add_action(rofl::openflow13::OFPAT_PUSH_PBB, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_PUSH_MPLS))
		actions.add_action(rofl::openflow13::OFPAT_PUSH_MPLS, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_PUSH_VLAN))
		actions.add_action(rofl::openflow13::OFPAT_PUSH_VLAN, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_COPY_TTL_OUT))
		actions.add_action(rofl::openflow13::OFPAT_COPY_TTL_OUT, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_DEC_NW_TTL))
		actions.add_action(rofl::openflow13::OFPAT_DEC_NW_TTL, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_DEC_MPLS_TTL))
		actions.add_action(rofl::openflow13::OFPAT_DEC_MPLS_TTL, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_SET_MPLS_TTL))
		actions.add_action(rofl::openflow13::OFPAT_SET_MPLS_TTL, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_SET_NW_TTL))
		actions.add_action(rofl::openflow13::OFPAT_SET_NW_TTL, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_SET_QUEUE))
		actions.add_action(rofl::openflow13::OFPAT_SET_QUEUE, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_GROUP))
		actions.add_action(rofl::openflow13::OFPAT_GROUP, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_OUTPUT))
		actions.add_action(rofl::openflow13::OFPAT_OUTPUT, 4);

	if (*bitmap & (1 << rofl::openflow13::OFPAT_SET_FIELD))
		actions.add_action(rofl::openflow13::OFPAT_SET_FIELD, 4);
#endif
}

void of13_translation_utils::of13_map_bitmap_instructions(uint32_t* bitmap, rofl::openflow::coftable_feature_prop_instructions& instructions)
{
	if(*bitmap & ( 1 << OF1X_IT_APPLY_ACTIONS))
		instructions.add_instruction(rofl::openflow::OFPIT_APPLY_ACTIONS, 4);

	if(*bitmap & ( 1 << OF1X_IT_CLEAR_ACTIONS))
		instructions.add_instruction(rofl::openflow::OFPIT_CLEAR_ACTIONS, 4);

	if(*bitmap & ( 1 << OF1X_IT_WRITE_ACTIONS))
		instructions.add_instruction(rofl::openflow::OFPIT_WRITE_ACTIONS, 4);

	if(*bitmap & ( 1 << OF1X_IT_WRITE_METADATA))
		instructions.add_instruction(rofl::openflow::OFPIT_WRITE_METADATA, 4);

	if(*bitmap & ( 1 << OF1X_IT_GOTO_TABLE))
		instructions.add_instruction(rofl::openflow::OFPIT_GOTO_TABLE, 4);

	if(*bitmap & ( 1 << OF1X_IT_METER))
		instructions.add_instruction(rofl::openflow::OFPIT_METER, 4);

#if 0
	if(*bitmap & ( 1 << OF1X_IT_METER))
		instructions.add_instruction(rofl::openflow::OFPIT_METER, 4);
#endif
}
