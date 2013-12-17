#define __STDC_CONSTANT_MACROS 1 // todo enable globally
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
	
	// iterate over all instructions and store them in the instruction group contained in entry
	for (cofinlist::iterator
			it = msg->get_instructions().begin(); it != msg->get_instructions().end(); ++it)
	{
		switch ((*it).get_type()) {
		case OFPIT_APPLY_ACTIONS:
		{
			of1x_action_group_t *apply_actions = of1x_init_action_group(0);

			try{
				of12_map_flow_entry_actions(ctl, sw, (*it).actions, apply_actions, /*of1x_write_actions_t*/0);
			}catch(...){
				of1x_destroy_flow_entry(entry);
				of1x_destroy_action_group(apply_actions);
				throw eFlowModUnknown();
			}

			of1x_add_instruction_to_group(
					&(entry->inst_grp),
					OF1X_IT_APPLY_ACTIONS,
					(of1x_action_group_t*)apply_actions,
					NULL,
					NULL,
					/*go_to_table*/0);
		}
			break;
		case OFPIT_CLEAR_ACTIONS:
		{
			of1x_add_instruction_to_group(
					&(entry->inst_grp),
					OF1X_IT_CLEAR_ACTIONS,
					NULL,
					NULL,
					NULL,
					/*go_to_table*/0);
		}
			break;
		case OFPIT_EXPERIMENTER:
		{
			of1x_add_instruction_to_group(
					&(entry->inst_grp),
					OF1X_IT_EXPERIMENTER,
					NULL,
					NULL,
					NULL,
					/*go_to_table*/0);
		}
			break;
		case OFPIT_GOTO_TABLE:
		{
			of1x_add_instruction_to_group(
					&(entry->inst_grp),
					OF1X_IT_GOTO_TABLE,
					NULL,
					NULL,
					NULL,
					/*go_to_table*/(*it).oin_goto_table->table_id);
		}
			break;
		case OFPIT_WRITE_ACTIONS:
		{
			of1x_write_actions_t *write_actions = of1x_init_write_actions();

			try{
				of12_map_flow_entry_actions(ctl, sw, (*it).actions, /*of1x_action_group_t*/0, write_actions);
			}catch(...){
				of1x_destroy_flow_entry(entry);	
				throw eFlowModUnknown();
			}


			of1x_add_instruction_to_group(
					&(entry->inst_grp),
					OF1X_IT_WRITE_ACTIONS,
					NULL,
					(of1x_write_actions_t*)write_actions,
					NULL,
					/*go_to_table*/0);
		}
			break;
		case OFPIT_WRITE_METADATA:
		{
			of1x_write_metadata_t metadata = {(*it).oin_write_metadata->metadata, (*it).oin_write_metadata->metadata_mask};
			
			of1x_add_instruction_to_group(
					&(entry->inst_grp),
					OF1X_IT_WRITE_METADATA,
					NULL,
					NULL,
					&metadata,
					/*go_to_table*/0);
		}
			break;
		}

	}

	return entry;
}



/**
* Maps a of1x_match from an OF1.2 Header
*/
void
of12_translation_utils::of12_map_flow_entry_matches(
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

	try {
		of1x_match_t *match = of1x_init_port_in_phy_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_in_phy_port());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	// metadata not implemented
	try {
		ofmatch.get_metadata();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_METADATA is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t maddr = ofmatch.get_eth_dst_addr().get_mac();
		uint64_t mmask = ofmatch.get_eth_dst_mask().get_mac();

		of1x_match_t *match = of1x_init_eth_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								maddr,
								mmask);

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t maddr = ofmatch.get_eth_src_addr().get_mac();
		uint64_t mmask = ofmatch.get_eth_src_mask().get_mac();

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
								ofmatch.get_vlan_vid_value(),
								ofmatch.get_vlan_vid_mask());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_vlan_pcp_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_vlan_pcp());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_ip_dscp_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ip_dscp());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_ip_ecn_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ip_ecn());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_ip_proto_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ip_proto());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		caddress value(ofmatch.get_ipv4_src_value());
		caddress mask (ofmatch.get_ipv4_src_mask());

		of1x_match_t *match = of1x_init_ip4_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								/*be32toh(value.ca_s4addr->sin_addr.s_addr),
								be32toh( mask.ca_s4addr->sin_addr.s_addr)*/
								value.get_ipv4_addr(),
								mask.get_ipv4_addr());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		caddress value(ofmatch.get_ipv4_dst_value());
		caddress mask (ofmatch.get_ipv4_dst_mask());

		of1x_match_t *match = of1x_init_ip4_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								/*be32toh(value.ca_s4addr->sin_addr.s_addr),
								be32toh( mask.ca_s4addr->sin_addr.s_addr)*/
								value.get_ipv4_addr(),
								mask.get_ipv4_addr());
		
		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_tcp_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_src());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_tcp_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_tcp_dst());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_udp_src_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_udp_src());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_udp_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_udp_dst());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_sctp_src();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_SCTP_SRC is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_sctp_dst();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_SCTP_DST is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_icmpv4_type_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_icmpv4_type());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_icmpv4_code_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_icmpv4_code());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_arp_opcode_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_arp_opcode());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t maddr = ofmatch.get_arp_sha_addr().get_mac();
		uint64_t mmask = ofmatch.get_arp_sha_mask().get_mac();

		of1x_match_t *match = of1x_init_arp_sha_match(
								/*prev*/NULL,
								/*next*/NULL,
								maddr,
								mmask);

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		caddress value(ofmatch.get_arp_spa_value());
		caddress mask (ofmatch.get_arp_spa_mask());

		of1x_match_t *match = of1x_init_arp_spa_match(
								/*prev*/NULL,
								/*next*/NULL,
								be32toh(value.ca_s4addr->sin_addr.s_addr),
								be32toh( mask.ca_s4addr->sin_addr.s_addr));

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t maddr = ofmatch.get_arp_tha_addr().get_mac();
		uint64_t mmask = ofmatch.get_arp_tha_mask().get_mac();

		of1x_match_t *match = of1x_init_arp_tha_match(
								/*prev*/NULL,
								/*next*/NULL,
								maddr,
								mmask);

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		caddress value(ofmatch.get_arp_tpa_value());
		caddress mask (ofmatch.get_arp_tpa_mask());

		of1x_match_t *match = of1x_init_arp_tpa_match(
								/*prev*/NULL,
								/*next*/NULL,
								be32toh(value.ca_s4addr->sin_addr.s_addr),
								be32toh( mask.ca_s4addr->sin_addr.s_addr));

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		caddress value(ofmatch.get_ipv6_src_value());
		caddress mask (ofmatch.get_ipv6_src_mask());
		
		of1x_match_t *match = of1x_init_ip6_src_match(
								/*prev*/NULL,
								/*prev*/NULL,
								value.get_ipv6_addr(),
								mask.get_ipv6_addr());
		/*WARNING we are swapping the values 3 times here!! coxmatch, cofmatch and caddress*/
		of1x_add_match_to_entry(entry,match);
	} catch (eOFmatchNotFound& e) {}
	
	try {
		caddress value(ofmatch.get_ipv6_dst_value());
		caddress mask (ofmatch.get_ipv6_dst_mask());
		of1x_match_t *match = of1x_init_ip6_dst_match(
								/*prev*/NULL,
								/*prev*/NULL,
								value.get_ipv6_addr(),
								mask.get_ipv6_addr());
		/*WARNING we are swapping the values 3 times here!! coxmatch, cofmatch and caddress*/
		of1x_add_match_to_entry(entry,match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_ip6_flabel_match(
								NULL,
								NULL,
								ofmatch.get_ipv6_flabel());
		of1x_add_match_to_entry(entry,match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_icmpv6_type_match(
								NULL,
								NULL,
								ofmatch.get_icmpv6_type());
		of1x_add_match_to_entry(entry,match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_icmpv6_code_match(
								NULL,
								NULL,
								ofmatch.get_icmpv6_code());
		of1x_add_match_to_entry(entry,match);
	} catch (eOFmatchNotFound& e) {}

	try {
		caddress value(ofmatch.get_ipv6_nd_target());
		of1x_match_t *match = of1x_init_ip6_nd_target_match(
								NULL,
								NULL,
						      		value.get_ipv6_addr());
		of1x_add_match_to_entry(entry,match);
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t mac = ofmatch.get_icmpv6_neighbor_source_lladdr().get_mac();
		of1x_match_t *match = of1x_init_ip6_nd_sll_match(
								NULL,
								NULL,
								mac);
		of1x_add_match_to_entry(entry,match);
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t mac = ofmatch.get_icmpv6_neighbor_target_lladdr().get_mac();
		of1x_match_t *match = of1x_init_ip6_nd_tll_match(
								NULL,
								NULL,
								mac);
		of1x_add_match_to_entry(entry,match);
	} catch (eOFmatchNotFound& e) {}
#if 0	
	try{

		/*TODO IPV6_EXTHDR*/
		of1x_match_t *match = of1x_init_ip6_exthdr_match(
								NULL,
								NULL,
								ofmatch.get_ipv6_exthdr());
		of1x_add_match_to_entry(entry,match);

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_IPV6_EXTHDR is missing")); // TODO
	}catch (eOFmatchNotFound& e) {}
#endif	
	try {
		of1x_match_t *match = of1x_init_mpls_label_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_mpls_label());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of1x_match_t *match = of1x_init_mpls_tc_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_mpls_tc());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		coxmatch_ofx_pppoe_code oxm_pppoe_code(
				ofmatch.get_const_match(OFPXMC_EXPERIMENTER, OFPXMT_OFX_PPPOE_CODE));

		of1x_match_t *match = of1x_init_pppoe_code_match(
								/*prev*/NULL,
								/*next*/NULL,
								oxm_pppoe_code.get_pppoe_code());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		coxmatch_ofx_pppoe_type oxm_pppoe_type(
				ofmatch.get_const_match(OFPXMC_EXPERIMENTER, OFPXMT_OFX_PPPOE_TYPE));

		of1x_match_t *match = of1x_init_pppoe_type_match(
								/*prev*/NULL,
								/*next*/NULL,
								oxm_pppoe_type.get_pppoe_type());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		coxmatch_ofx_pppoe_sid oxm_pppoe_sid(
				ofmatch.get_const_match(OFPXMC_EXPERIMENTER, OFPXMT_OFX_PPPOE_SID));

		of1x_match_t *match = of1x_init_pppoe_session_match(
								/*prev*/NULL,
								/*next*/NULL,
								oxm_pppoe_sid.get_pppoe_sid());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		coxmatch_ofx_ppp_prot oxm_ppp_prot(
				ofmatch.get_const_match(OFPXMC_EXPERIMENTER, OFPXMT_OFX_PPP_PROT));

		of1x_match_t *match = of1x_init_ppp_prot_match(
								/*prev*/NULL,
								/*next*/NULL,
								oxm_ppp_prot.get_ppp_prot());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		coxmatch_ofx_gtp_msg_type oxm_gtp_msg_type(
				ofmatch.get_const_match(OFPXMC_EXPERIMENTER, OFPXMT_OFX_GTP_MSG_TYPE));

		of1x_match_t *match = of1x_init_gtp_msg_type_match(
								/*prev*/NULL,
								/*next*/NULL,
								oxm_gtp_msg_type.get_msg_type());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		coxmatch_ofx_gtp_teid oxm_gtp_teid(
				ofmatch.get_const_match(OFPXMC_EXPERIMENTER, OFPXMT_OFX_GTP_TEID));

		of1x_match_t *match = of1x_init_gtp_teid_match(
								/*prev*/NULL,
								/*next*/NULL,
								oxm_gtp_teid.get_teid_value(),
								oxm_gtp_teid.get_teid_mask());

		of1x_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}
}



/**
* Maps a of1x_action from an OF1.2 Header
*/
void
of12_translation_utils::of12_map_flow_entry_actions(
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
		case OFP12AT_OUTPUT:
			field.u32 = be32toh(raction.oac_12output->port);
			action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, NULL, NULL);
			break;
		case OFP12AT_COPY_TTL_OUT:
			action = of1x_init_packet_action( OF1X_AT_COPY_TTL_OUT, field, NULL, NULL);
			break;
		case OFP12AT_COPY_TTL_IN:
			action = of1x_init_packet_action( OF1X_AT_COPY_TTL_IN, field, NULL, NULL);
			break;
		case OFP12AT_SET_MPLS_TTL:
			field.u8 = raction.oac_12mpls_ttl->mpls_ttl;
			action = of1x_init_packet_action( OF1X_AT_SET_MPLS_TTL, field, NULL, NULL);
			break;
		case OFP12AT_DEC_MPLS_TTL:
			action = of1x_init_packet_action( OF1X_AT_DEC_MPLS_TTL, field, NULL, NULL);
			break;
		case OFP12AT_PUSH_VLAN:
			field.u16 = be16toh(raction.oac_oacu.oacu_12push->ethertype);
			action = of1x_init_packet_action( OF1X_AT_PUSH_VLAN, field, NULL, NULL);
			break;
		case OFP12AT_POP_VLAN:
			field.u16 = be16toh(raction.oac_12push->ethertype);
			action = of1x_init_packet_action( OF1X_AT_POP_VLAN, field, NULL, NULL);
			break;
		case OFP12AT_PUSH_MPLS:
			field.u16 = be16toh(raction.oac_12push->ethertype);
			action = of1x_init_packet_action( OF1X_AT_PUSH_MPLS, field, NULL, NULL);
			break;
		case OFP12AT_POP_MPLS:
			field.u16 = be16toh(raction.oac_12push->ethertype);
			action = of1x_init_packet_action( OF1X_AT_POP_MPLS,  field, NULL, NULL);
			break;
		case OFP12AT_SET_QUEUE:
			field.u32 = be32toh(raction.oac_12set_queue->queue_id);
			action = of1x_init_packet_action( OF1X_AT_SET_QUEUE, field, NULL, NULL);
			break;
		case OFP12AT_GROUP:
			field.u32 = be32toh(raction.oac_12group->group_id);
			action = of1x_init_packet_action( OF1X_AT_GROUP, field, NULL, NULL);
			break;
		case OFP12AT_SET_NW_TTL:
			field.u8 = raction.oac_12nw_ttl->nw_ttl;
			action = of1x_init_packet_action( OF1X_AT_SET_NW_TTL, field, NULL, NULL);
			break;
		case OFP12AT_DEC_NW_TTL:
			action = of1x_init_packet_action( OF1X_AT_DEC_NW_TTL, field, NULL, NULL);
			break;
		case OFP12AT_SET_FIELD:
		{
			coxmatch oxm = raction.get_oxm();

			switch (oxm.get_oxm_class()) {
			case OFPXMC_OPENFLOW_BASIC:
			{
				switch (oxm.get_oxm_field()) {
				case OFPXMT_OFB_ETH_DST:
				{
					cmacaddr mac(oxm.oxm_uint48t->value, 6);
					field.u64 = mac.get_mac();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_DST, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ETH_SRC:
				{
					cmacaddr mac(oxm.oxm_uint48t->value, 6);
					field.u64 = mac.get_mac();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_SRC, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ETH_TYPE:
				{
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ETH_TYPE, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ARP_OP:
				{
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_OPCODE, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ARP_SHA:
				{
					cmacaddr mac(oxm.oxm_uint48t->value, 6);
					field.u64 = mac.get_mac();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_SHA, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ARP_SPA:
				{
					field.u32 = oxm.uint32_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_SPA, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ARP_THA:
				{
					cmacaddr mac(oxm.oxm_uint48t->value, 6);
					field.u64 = mac.get_mac();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_THA, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ARP_TPA:
				{
					field.u32 = oxm.uint32_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ARP_TPA, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ICMPV4_CODE:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ICMPV4_CODE, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ICMPV4_TYPE:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_ICMPV4_TYPE, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IPV4_DST:
				{
					field.u32 = oxm.uint32_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IPV4_DST, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IPV4_SRC:
				{
					field.u32 = oxm.uint32_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IPV4_SRC, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IP_DSCP:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_DSCP, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IP_ECN:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_ECN, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IP_PROTO:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_IP_PROTO, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_MPLS_LABEL:
				{
					field.u32 = oxm.uint32_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_MPLS_LABEL, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_MPLS_TC:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_MPLS_TC, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_VLAN_VID:
				{
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_VID, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_VLAN_PCP:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_PCP, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_TCP_DST:
				{
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_TCP_DST, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_TCP_SRC:
				{
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_TCP_SRC, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_UDP_DST:
				{
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_UDP_DST, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_UDP_SRC:
				{
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_UDP_SRC, field, NULL, NULL);
				}
					break;

				case OFPXMT_OFB_IPV6_SRC: {
					field.u128 = oxm.u128addr().get_ipv6_addr();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_SRC, field, NULL, NULL);
				}break;
				case OFPXMT_OFB_IPV6_DST: {
					field.u128 = oxm.u128addr().get_ipv6_addr();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_DST, field, NULL, NULL);
				}break;
				case OFPXMT_OFB_IPV6_FLABEL: {
					field.u32 = oxm.uint32_value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_FLABEL, field, NULL, NULL);
				}break;
				case OFPXMT_OFB_IPV6_ND_TARGET: {
					field.u128 = oxm.u128addr().get_ipv6_addr();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_ND_TARGET, field, NULL, NULL);
				}break;
				case OFPXMT_OFB_IPV6_ND_SLL: {
					field.u64 = oxm.uint64_value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_ND_SLL, field, NULL, NULL);
				}break;
				case OFPXMT_OFB_IPV6_ND_TLL: {
					field.u64 = oxm.uint64_value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_ND_TLL, field, NULL, NULL);
				}break;
				case OFPXMT_OFB_IPV6_EXTHDR: {
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_IPV6_EXTHDR, field, NULL, NULL);
				}break;
				case OFPXMT_OFB_ICMPV6_TYPE: {
					field.u64 = oxm.uint64_value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_ICMPV6_TYPE, field, NULL, NULL);
				}break;
				case OFPXMT_OFB_ICMPV6_CODE: {
					field.u64 = oxm.uint64_value();
					action = of1x_init_packet_action(OF1X_AT_SET_FIELD_ICMPV6_CODE, field, NULL, NULL);
				}break;
					
				default:
				{
					ROFL_ERR("of1x_endpoint(%s)::of12_map_flow_entry() "
							"unknown OXM type in action SET-FIELD found: %s",
							sw->dpname.c_str(), raction.c_str());
				}
					break;
				}
			}
				break;
			case OFPXMC_EXPERIMENTER: {
				switch (oxm.get_oxm_field()) {
				case OFPXMT_OFX_PPPOE_CODE:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPPOE_CODE, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFX_PPPOE_TYPE:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPPOE_TYPE, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFX_PPPOE_SID:
				{
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPPOE_SID, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFX_PPP_PROT:
				{
					field.u16 = oxm.uint16_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_PPP_PROT, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFX_GTP_MSG_TYPE:
				{
					field.u8 = oxm.uint8_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_GTP_MSG_TYPE, field, NULL, NULL);
				}
					break;
				case OFPXMT_OFX_GTP_TEID:
				{
					field.u32 = oxm.uint32_value();
					action = of1x_init_packet_action( OF1X_AT_SET_FIELD_GTP_TEID, field, NULL, NULL);
				}
					break;
				}

			}
				break;
			default:
			{
				ROFL_ERR("of1x_endpoint(%s)::of12_map_flow_entry() "
						"unknown OXM class in action SET-FIELD found: %s",
						sw->dpname.c_str(), raction.c_str());
			}
				break;
			}
		}
			break;
		case OFP12AT_EXPERIMENTER: {

			cofaction_experimenter eaction(raction);

			switch (eaction.get_exp_id()) {
			case ROFL_EXPERIMENTER_ID: {

				// ROFL experimental actions contain experimental action type at position data[0]
				uint8_t acttype = eaction.oac_12experimenter->data[0];

				switch (acttype) {
				case cofaction_push_pppoe::OFXAT_PUSH_PPPOE: {
					cofaction_push_pppoe paction(eaction);
					field.u16 = be16toh(paction.eoac_push_pppoe->expbody.ethertype);
					action = of1x_init_packet_action( OF1X_AT_PUSH_PPPOE, field, NULL, NULL);
				} break;
				case cofaction_pop_pppoe::OFXAT_POP_PPPOE: {
					cofaction_pop_pppoe paction(eaction);
					field.u16 = be16toh(paction.eoac_pop_pppoe->expbody.ethertype);
					action = of1x_init_packet_action( OF1X_AT_POP_PPPOE, field, NULL, NULL);
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
		cofmatch& match)
{
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
			match.set_metadata(m->value->value.u64);
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
			match.set_vlan_vid(m->value->value.u16);
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
			addr.ca_s4addr->sin_addr.s_addr = htonl(m->value->value.u32);
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
			addr.ca_s4addr->sin_addr.s_addr = htonl(m->value->value.u32);
			match.set_arp_tpa(addr);
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
			addr.ca_s4addr->sin_addr.s_addr = htonl(m->value->value.u32);
			match.set_ipv4_src(addr);

		}
			break;
		case OF1X_MATCH_IPV4_DST:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = htonl(m->value->value.u32);
			match.set_ipv4_dst(addr);
		}
			break;
		case OF1X_MATCH_TCP_SRC:
			match.set_tcp_src(m->value->value.u16);
			break;
		case OF1X_MATCH_TCP_DST:
			match.set_tcp_dst(m->value->value.u16);
			break;
		case OF1X_MATCH_UDP_SRC:
			match.set_udp_src(m->value->value.u16);
			break;
		case OF1X_MATCH_UDP_DST:
			match.set_udp_dst(m->value->value.u16);
			break;
		case OF1X_MATCH_SCTP_SRC:
			match.set_sctp_src(m->value->value.u16);
			break;
		case OF1X_MATCH_SCTP_DST:
			match.set_sctp_dst(m->value->value.u16);
			break;
		case OF1X_MATCH_ICMPV4_TYPE:
			match.set_icmpv4_type(m->value->value.u8);
			break;
		case OF1X_MATCH_ICMPV4_CODE:
			match.set_icmpv4_code(m->value->value.u8);
			break;
		case OF1X_MATCH_IPV6_SRC: {
			caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
			/*TODO deal with endianess??*/
			memcpy(&(addr.ca_s6addr->sin6_addr.__in6_u.__u6_addr8), &(m->value->value.u128.val), sizeof(uint128__t));
			match.set_ipv6_src(addr);
			}break;
		case OF1X_MATCH_IPV6_DST:{
			caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
			/*TODO deal with endianess??*/
			memcpy(&(addr.ca_s6addr->sin6_addr.__in6_u.__u6_addr8), &(m->value->value.u128.val), sizeof(addr));
			match.set_ipv6_dst(addr);
			}break;
		case OF1X_MATCH_IPV6_FLABEL:
			match.set_ipv6_flabel(m->value->value.u64);
			break;
		case OF1X_MATCH_ICMPV6_TYPE:
			match.set_icmpv6_type(m->value->value.u64);
			break;
		case OF1X_MATCH_ICMPV6_CODE:
			match.set_icmpv6_code(m->value->value.u64);
			break;
		case OF1X_MATCH_IPV6_ND_TARGET:{
			caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
			/*TODO deal with endianess??*/
			memcpy(&(addr.ca_s6addr->sin6_addr.__in6_u.__u6_addr8), &(m->value->value.u128.val),sizeof(addr));
			match.set_ipv6_nd_target(addr);
			}break;
		case OF1X_MATCH_IPV6_ND_SLL:
			match.set_icmpv6_neighbor_source_lladdr(m->value->value.u64);
			break;
		case OF1X_MATCH_IPV6_ND_TLL:
			match.set_icmpv6_neighbor_target_lladdr(m->value->value.u64);
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
}

/**
* Maps a of1x_group_bucket from an OF1.2 Header
*/
void
of12_translation_utils::of12_map_bucket_list(
		cofctl *ctl,
		openflow_switch* sw,
		cofbclist& of_buckets,
		of1x_bucket_list_t* bucket_list)
{	
	
	for(cofbclist::iterator jt = of_buckets.begin();jt != of_buckets.end();++jt){
		//for each bucket we must map its actions
		cofbucket& bucket_ptr = (*jt);
		of1x_action_group_t* action_group = of1x_init_action_group(NULL);
		if(action_group == NULL){
			//TODO Handle Error
		}
		
		of12_map_flow_entry_actions(ctl,sw,bucket_ptr.actions,action_group,NULL);
		of1x_insert_bucket_in_list(bucket_list,of1x_init_bucket(bucket_ptr.weight, bucket_ptr.watch_port, bucket_ptr.watch_group, action_group));
	}
}

void of12_translation_utils::of12_map_reverse_bucket_list(
		cofbclist& of_buckets,
		of1x_bucket_list_t* bucket_list){
	
	for(of1x_bucket_t *bu_it=bucket_list->head;bu_it;bu_it=bu_it->next){
		//cofbucket single_bucket;
		cofaclist ac_list(OFP12_VERSION);
		for (of1x_packet_action_t *action_it = bu_it->actions->head; action_it != NULL; action_it = action_it->next) {
			if (OF1X_AT_NO_ACTION == action_it->type)
				continue;
			cofaction action(OFP12_VERSION);
			of12_map_reverse_flow_entry_action(action_it, action);
			//push this action into the list
			ac_list.next() = action;
		}
		of_buckets.next() = cofbucket(OFP12_VERSION);
		cofbucket &single_bucket = of_buckets.back();
		// insert action list in the bucket
		single_bucket.actions=ac_list;
		single_bucket.watch_port = bu_it->port;
		single_bucket.watch_group = bu_it->group;
		single_bucket.weight = bu_it->weight;
		// insert bucket in bucket_list

	}
}


/**
*
*/
void
of12_translation_utils::of12_map_reverse_flow_entry_instructions(
		of1x_instruction_group_t* group,
		cofinlist& instructions)
{
	for (unsigned int i = 0; i < (sizeof(group->instructions) / sizeof(of1x_instruction_t)); i++) {
		if (OF1X_IT_NO_INSTRUCTION == group->instructions[i].type)
			continue;
		cofinst instruction(OFP12_VERSION);
		of12_map_reverse_flow_entry_instruction(&(group->instructions[i]), instruction);
		instructions.next() = instruction;
	}
}


void
of12_translation_utils::of12_map_reverse_flow_entry_instruction(
		of1x_instruction_t* inst,
		cofinst& instruction)
{
	switch (inst->type) {
	case OF1X_IT_APPLY_ACTIONS: {
		instruction = cofinst_apply_actions(OFP12_VERSION);
		for (of1x_packet_action_t *of1x_action = inst->apply_actions->head; of1x_action != NULL; of1x_action = of1x_action->next) {
			if (OF1X_AT_NO_ACTION == of1x_action->type)
				continue;
			cofaction action(OFP12_VERSION);
			of12_map_reverse_flow_entry_action(of1x_action, action);
			instruction.actions.next() = action;
		}
	} break;
	case OF1X_IT_CLEAR_ACTIONS: {
		instruction = cofinst_clear_actions(OFP12_VERSION);
	} break;
	case OF1X_IT_WRITE_ACTIONS: {
		instruction = cofinst_write_actions(OFP12_VERSION);
		for (unsigned int i = 0; i < OF1X_IT_GOTO_TABLE; i++) {
			if (OF1X_AT_NO_ACTION == inst->write_actions->write_actions[i].type)
				continue;
			cofaction action(OFP12_VERSION);
			of12_map_reverse_flow_entry_action(&(inst->write_actions->write_actions[i]), action);
			instruction.actions.next() = action;
		}
	} break;
	case OF1X_IT_WRITE_METADATA:
	case OF1X_IT_EXPERIMENTER: {
		// TODO: both are marked TODO in of1x_pipeline
	} break;
	case OF1X_IT_GOTO_TABLE: {
		instruction = cofinst_goto_table(OFP12_VERSION, inst->go_to_table);
	} break;
	default: {
		// do nothing
	} break;
	}
}


void
of12_translation_utils::of12_map_reverse_flow_entry_action(
		of1x_packet_action_t* of1x_action,
		cofaction& action)
{
	/*
	 * FIXME: add masks for those fields defining masked values in the specification
	 */


	switch (of1x_action->type) {
	case OF1X_AT_NO_ACTION: {
		// do nothing
	} break;
	case OF1X_AT_COPY_TTL_IN: {
		action = cofaction_copy_ttl_in(OFP12_VERSION);
	} break;
	case OF1X_AT_POP_VLAN: {
		action = cofaction_pop_vlan(OFP12_VERSION);
	} break;
	case OF1X_AT_POP_MPLS: {
		action = cofaction_pop_mpls(OFP12_VERSION, (uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK));
	} break;
	case OF1X_AT_POP_PPPOE: {
		action = cofaction_pop_pppoe(OFP12_VERSION, (uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK));
	} break;
	case OF1X_AT_PUSH_PPPOE: {
		action = cofaction_push_pppoe(OFP12_VERSION, (uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK));
	} break;
	case OF1X_AT_PUSH_MPLS: {
		action = cofaction_push_mpls(OFP12_VERSION, (uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK));
	} break;
	case OF1X_AT_PUSH_VLAN: {
		action = cofaction_push_vlan(OFP12_VERSION, (uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK));
	} break;
	case OF1X_AT_COPY_TTL_OUT: {
		action = cofaction_copy_ttl_out(OFP12_VERSION);
	} break;
	case OF1X_AT_DEC_NW_TTL: {
		action = cofaction_dec_nw_ttl(OFP12_VERSION);
	} break;
	case OF1X_AT_DEC_MPLS_TTL: {
		action = cofaction_dec_mpls_ttl(OFP12_VERSION);
	} break;
	case OF1X_AT_SET_MPLS_TTL: {
		action = cofaction_set_mpls_ttl(OFP12_VERSION, (uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK));
	} break;
	case OF1X_AT_SET_NW_TTL: {
		action = cofaction_set_nw_ttl(OFP12_VERSION, (uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK));
	} break;
	case OF1X_AT_SET_QUEUE: {
		action = cofaction_set_queue(OFP12_VERSION, (uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK));
	} break;
	//case OF1X_AT_SET_FIELD_METADATA:
	case OF1X_AT_SET_FIELD_ETH_DST: {
		cmacaddr maddr(of1x_action->field.u64);
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_eth_dst(maddr));
	} break;
	case OF1X_AT_SET_FIELD_ETH_SRC: {
		cmacaddr maddr(of1x_action->field.u64);
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_eth_src(maddr));
	} break;
	case OF1X_AT_SET_FIELD_ETH_TYPE: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_eth_type((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_VLAN_VID: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_vlan_vid((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_VLAN_PCP: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_vlan_pcp((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_ARP_OPCODE: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_arp_opcode((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_ARP_SHA: {
		cmacaddr maddr(of1x_action->field.u64);
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_arp_sha(maddr));
	} break;
	case OF1X_AT_SET_FIELD_ARP_SPA: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_arp_spa((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_ARP_THA: {
		cmacaddr maddr(of1x_action->field.u64);
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_arp_tha(maddr));
	} break;
	case OF1X_AT_SET_FIELD_ARP_TPA: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_arp_tpa((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_IP_DSCP: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ip_dscp((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_IP_ECN: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ip_ecn((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_IP_PROTO: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ip_proto((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_IPV4_SRC: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ipv4_src((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_IPV4_DST: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ipv4_dst((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_TCP_SRC: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_tcp_src((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_TCP_DST: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_tcp_dst((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_UDP_SRC: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_udp_src((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_UDP_DST: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_udp_dst((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_ICMPV4_TYPE: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_icmpv4_type((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_ICMPV4_CODE: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_icmpv4_code((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	
	case OF1X_AT_SET_FIELD_IPV6_SRC: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ipv6_src((uint8_t*)(of1x_action->field.u128.val),16));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_DST: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ipv6_dst((uint8_t*)(of1x_action->field.u128.val),16));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_FLABEL: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ipv6_flabel((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_ND_TARGET: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ipv6_nd_target((uint8_t*)(of1x_action->field.u128.val),16));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_ND_SLL: {
		cmacaddr maddr(of1x_action->field.u64);
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ipv6_nd_sll(maddr));
	} break;
	case OF1X_AT_SET_FIELD_IPV6_ND_TLL: {
		cmacaddr maddr(of1x_action->field.u64);
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_ipv6_nd_tll(maddr));
	} break;
	/*TODO EXT HDR*/
	case OF1X_AT_SET_FIELD_IPV6_EXTHDR:
		throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_action() IPV6 ICMPV6"));
		break;
	case OF1X_AT_SET_FIELD_ICMPV6_TYPE: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_icmpv6_type((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_ICMPV6_CODE: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_icmpv6_code((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_MPLS_LABEL: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_mpls_label((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_MPLS_TC: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofb_mpls_tc((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_PPPOE_CODE: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofx_pppoe_code((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_PPPOE_TYPE: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofx_pppoe_type((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_PPPOE_SID: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofx_pppoe_sid((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_PPP_PROT: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofx_ppp_prot((uint16_t)(of1x_action->field.u16 & OF1X_2_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_GTP_MSG_TYPE: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofx_gtp_msg_type((uint8_t)(of1x_action->field.u8 & OF1X_1_BYTE_MASK)));
	} break;
	case OF1X_AT_SET_FIELD_GTP_TEID: {
		action = cofaction_set_field(OFP12_VERSION, coxmatch_ofx_gtp_teid((uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK)));
	} break;
	case OF1X_AT_GROUP: {
		action = cofaction_group(OFP12_VERSION, (uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK));
	} break;
	case OF1X_AT_EXPERIMENTER: {
		// TODO
	} break;
	case OF1X_AT_OUTPUT: {
		action = cofaction_output(OFP12_VERSION, (uint32_t)(of1x_action->field.u32 & OF1X_4_BYTE_MASK));
	} break;
	default: {
		// do nothing
	} break;
	}
}


/*
* Maps packet actions to cofmatches
*/

void of12_translation_utils::of12_map_reverse_packet_matches(of1x_packet_matches_t* packet_matches, cofmatch& match){
	if(packet_matches->port_in)
		match.set_in_port(packet_matches->port_in);
	if(packet_matches->phy_port_in)
		match.set_in_phy_port(packet_matches->phy_port_in);
	if(packet_matches->metadata)
		match.set_metadata(packet_matches->metadata);
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
		
	if( UINT128__T_HI(packet_matches->ipv6_src) || UINT128__T_LO(packet_matches->ipv6_src) ){
		caddress addr(AF_INET6,"0:0:0:0:0:0:0:0");
		addr.set_ipv6_addr(packet_matches->ipv6_src);
		match.set_ipv6_src(addr);
	}
	if( UINT128__T_HI(packet_matches->ipv6_dst) || UINT128__T_LO(packet_matches->ipv6_dst) ){
		caddress addr(AF_INET6,"0:0:0:0");
		addr.set_ipv6_addr(packet_matches->ipv6_dst);
		match.set_ipv6_dst(addr);
	}
	if(packet_matches->ipv6_flabel)
		match.set_ipv6_flabel(packet_matches->ipv6_flabel);
	if( UINT128__T_HI(packet_matches->ipv6_nd_target) || UINT128__T_LO(packet_matches->ipv6_nd_target) ){
		caddress addr(AF_INET6,"0:0:0:0");
		addr.set_ipv6_addr(packet_matches->ipv6_nd_target);
		match.set_ipv6_nd_target(addr);
	}
	if(packet_matches->ipv6_nd_sll)
		match.set_icmpv6_neighbor_source_lladdr(packet_matches->ipv6_nd_sll);
	if(packet_matches->ipv6_nd_tll)
		match.set_icmpv6_neighbor_target_lladdr(packet_matches->ipv6_nd_tll);
	//TODO IPv6 ext hdr not yet implemented in cofmatch
	//if(packet_matches->ipv6_exthdr)
		//match.set_ipv6_exthdr(packet_matches->ipv6_exthdr);
	
	if(packet_matches->icmpv6_type)
		match.set_icmpv6_type(packet_matches->icmpv6_type);
	if(packet_matches->icmpv6_code)
		match.set_icmpv6_code(packet_matches->icmpv6_code);
		
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

/*
* Table capability bitmap
*/


uint64_t of12_translation_utils::of12_map_bitmap_matches(uint64_t* bitmap){

	uint64_t mapped_bitmap=0x0;

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IN_PORT))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IN_PORT);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IN_PHY_PORT))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IN_PHY_PORT);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_METADATA))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_METADATA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ETH_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ETH_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ETH_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ETH_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ETH_TYPE))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ETH_TYPE);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_VLAN_VID))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_VLAN_VID);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_VLAN_PCP))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_VLAN_PCP);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_MPLS_LABEL))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_MPLS_LABEL);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_MPLS_TC))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_MPLS_TC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_OP))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ARP_OP);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_SPA))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ARP_SPA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_TPA))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ARP_TPA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_SHA))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ARP_SHA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ARP_THA))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ARP_THA);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IP_DSCP))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IP_DSCP);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IP_ECN))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IP_ECN);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IP_PROTO))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IP_PROTO);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV4_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IPV4_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV4_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IPV4_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IPV6_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IPV6_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_FLABEL))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IPV6_FLABEL);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ICMPV6_TYPE))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ICMPV6_TYPE);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ICMPV6_CODE))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ICMPV6_CODE);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_ND_TARGET))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IPV6_ND_TARGET);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_ND_SLL))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IPV6_ND_SLL);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_ND_TLL))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IPV6_ND_TLL);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_IPV6_EXTHDR))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IPV6_EXTHDR);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_TCP_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_TCP_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_TCP_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_TCP_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_UDP_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_UDP_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_UDP_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_UDP_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_SCTP_SRC))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_SCTP_SRC);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_SCTP_DST))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_SCTP_DST);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ICMPV4_TYPE))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ICMPV4_TYPE);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_ICMPV4_CODE))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_ICMPV4_CODE);

//	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_PBB_ISID))
//		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_MATCH_PBB_ISID);

//	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_TUNNEL_ID))
//		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_TUNNEL_ID);

//TODO: codes now collide
#if 0
	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_PPPOE_CODE))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IN_PORT);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_PPPOE_TYPE))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IN_PORT);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_PPPOE_SID))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IN_PORT);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_PPP_PROT))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IN_PORT);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_GTP_MSG_TYPE))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IN_PORT);

	if(*bitmap & ( UINT64_C(1) << OF1X_MATCH_GTP_TEID))
		mapped_bitmap |= ( UINT64_C(1) <<  OFPXMT_OFB_IN_PORT);
#endif

	return mapped_bitmap;	
}

uint32_t of12_translation_utils::of12_map_bitmap_actions(uint32_t* bitmap){
	//No mapping required (yet)
	return *bitmap;	
}

uint32_t of12_translation_utils::of12_map_bitmap_instructions(uint32_t* bitmap){
	
	uint32_t mapped_bitmap=0x0;

	if(*bitmap & ( 1 << OF1X_IT_APPLY_ACTIONS))
		mapped_bitmap |= (1 << OFPIT_APPLY_ACTIONS);

	if(*bitmap & ( 1 << OF1X_IT_CLEAR_ACTIONS))
		mapped_bitmap |= (1 << OFPIT_CLEAR_ACTIONS);

	if(*bitmap & ( 1 << OF1X_IT_WRITE_ACTIONS))
		mapped_bitmap |= (1 << OFPIT_WRITE_ACTIONS);

	if(*bitmap & ( 1 << OF1X_IT_WRITE_METADATA))
		mapped_bitmap |= (1 << OFPIT_WRITE_METADATA);
	
	if(*bitmap & ( 1 << OF1X_IT_GOTO_TABLE))
		mapped_bitmap |= (1 << OFPIT_GOTO_TABLE);
	
	if(*bitmap & ( 1 << OF1X_IT_METER))
		mapped_bitmap |= (1 << OFPIT_METER);
	
	return mapped_bitmap;	
}
