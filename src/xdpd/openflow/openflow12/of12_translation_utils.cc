#include "of12_translation_utils.h"

using namespace rofl;

//Byte masks
#define OF12_AT_6_BYTE_MASK 0x0000FFFFFFFFFFFF
#define OF12_AT_4_BYTE_MASK 0x00000000FFFFFFFF
#define OF12_AT_2_BYTE_MASK 0x000000000000FFFF
#define OF12_AT_1_BYTE_MASK 0x00000000000000FF

//Non-multiple of byte masks
#define OF12_AT_20_BITS_MASK 0x00000000000FFFFF
#define OF12_AT_13_BITS_MASK 0x00000000000FFFFF
#define OF12_AT_6_BITS_MASK 0x00000000000FFFFF
#define OF12_AT_3_BITS_MASK 0x00000000000FFFFF
#define OF12_AT_2_BITS_MASK 0x00000000000FFFFF


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
* Maps a of12_flow_entry from an OF1.2 Header
*/
of12_flow_entry_t*
of12_translation_utils::of12_map_flow_entry(
		cofctl *ctl, 
		cofmsg_flow_mod *msg,
		openflow_switch* sw)
{
	of12_flow_entry_t *entry = of12_init_flow_entry(NULL, NULL, msg->get_flags() & OFPFF_SEND_FLOW_REM);

	// store flow-mod fields in of12_flow_entry
	entry->priority 		= msg->get_priority();
	entry->cookie 			= msg->get_cookie();
	entry->cookie_mask 		= msg->get_cookie_mask();
	entry->timer_info.idle_timeout	= msg->get_idle_timeout(); // these timers must be activated some time, when?
	entry->timer_info.hard_timeout	= msg->get_hard_timeout();

	// extract OXM fields from pack and store them in of12_flow_entry
	of12_map_flow_entry_matches(ctl, msg->get_match(), sw, entry);

	// iterate over all instructions and store them in the instruction group contained in entry
	of12_init_instruction_group(&(entry->inst_grp));

	for (cofinlist::iterator
			it = msg->get_instructions().begin(); it != msg->get_instructions().end(); ++it)
	{
		switch ((*it).get_type()) {
		case OFPIT_APPLY_ACTIONS:
		{
			of12_action_group_t *apply_actions = of12_init_action_group(0);

			of12_map_flow_entry_actions(ctl, sw, (*it).actions, apply_actions, /*of12_write_actions_t*/0);

			of12_add_instruction_to_group(
					&(entry->inst_grp),
					OF12_IT_APPLY_ACTIONS,
					(of12_action_group_t*)apply_actions,
					NULL,
					/*go_to_table*/0);
		}
			break;
		case OFPIT_CLEAR_ACTIONS:
		{
			of12_add_instruction_to_group(
					&(entry->inst_grp),
					OF12_IT_CLEAR_ACTIONS,
					NULL,
					NULL,
					/*go_to_table*/0);
		}
			break;
		case OFPIT_EXPERIMENTER:
		{
			of12_add_instruction_to_group(
					&(entry->inst_grp),
					OF12_IT_EXPERIMENTER,
					NULL,
					NULL,
					/*go_to_table*/0);
		}
			break;
		case OFPIT_GOTO_TABLE:
		{
			of12_add_instruction_to_group(
					&(entry->inst_grp),
					OF12_IT_GOTO_TABLE,
					NULL,
					NULL,
					/*go_to_table*/(*it).oin_goto_table->table_id);
		}
			break;
		case OFPIT_WRITE_ACTIONS:
		{
			of12_write_actions_t *write_actions = of12_init_write_actions();

			of12_map_flow_entry_actions(ctl, sw, (*it).actions, /*of12_action_group_t*/0, write_actions);

			of12_add_instruction_to_group(
					&(entry->inst_grp),
					OF12_IT_WRITE_ACTIONS,
					NULL,
					(of12_write_actions_t*)write_actions,
					/*go_to_table*/0);
		}
			break;
		case OFPIT_WRITE_METADATA:
		{
			/*
			 * TODO: How do I write metadata and metadata-mask into the pipeline instruction?
			 */
			of12_add_instruction_to_group(
					&(entry->inst_grp),
					OF12_IT_WRITE_METADATA,
					NULL,
					NULL,
					/*go_to_table*/0);
		}
			break;
		}

	}

	return entry;
}



/**
* Maps a of12_match from an OF1.2 Header
*/
void
of12_translation_utils::of12_map_flow_entry_matches(
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

	try {
		of12_match_t *match = of12_init_port_in_phy_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_in_phy_port());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	// metadata not implemented
	try {
		ofmatch.get_metadata();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_METADATA is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t maddr = 0;
		{
			((uint8_t*)&maddr)[0] = ofmatch.get_eth_dst_addr()[0];
			((uint8_t*)&maddr)[1] = ofmatch.get_eth_dst_addr()[1];
			((uint8_t*)&maddr)[2] = ofmatch.get_eth_dst_addr()[2];
			((uint8_t*)&maddr)[3] = ofmatch.get_eth_dst_addr()[3];
			((uint8_t*)&maddr)[4] = ofmatch.get_eth_dst_addr()[4];
			((uint8_t*)&maddr)[5] = ofmatch.get_eth_dst_addr()[5];
		}

		uint64_t mmask = 0xffffffffffffffff;
		{
			((uint8_t*)&mmask)[0] = ofmatch.get_eth_dst_mask()[0];
			((uint8_t*)&mmask)[1] = ofmatch.get_eth_dst_mask()[1];
			((uint8_t*)&mmask)[2] = ofmatch.get_eth_dst_mask()[2];
			((uint8_t*)&mmask)[3] = ofmatch.get_eth_dst_mask()[3];
			((uint8_t*)&mmask)[4] = ofmatch.get_eth_dst_mask()[4];
			((uint8_t*)&mmask)[5] = ofmatch.get_eth_dst_mask()[5];
		}

		of12_match_t *match = of12_init_eth_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								maddr,
								mmask);

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		uint64_t maddr = 0;
		{
			((uint8_t*)&maddr)[0] = ofmatch.get_eth_src_addr()[0];
			((uint8_t*)&maddr)[1] = ofmatch.get_eth_src_addr()[1];
			((uint8_t*)&maddr)[2] = ofmatch.get_eth_src_addr()[2];
			((uint8_t*)&maddr)[3] = ofmatch.get_eth_src_addr()[3];
			((uint8_t*)&maddr)[4] = ofmatch.get_eth_src_addr()[4];
			((uint8_t*)&maddr)[5] = ofmatch.get_eth_src_addr()[5];
		}

		uint64_t mmask = 0xffffffffffffffff;
		{
			((uint8_t*)&mmask)[0] = ofmatch.get_eth_src_mask()[0];
			((uint8_t*)&mmask)[1] = ofmatch.get_eth_src_mask()[1];
			((uint8_t*)&mmask)[2] = ofmatch.get_eth_src_mask()[2];
			((uint8_t*)&mmask)[3] = ofmatch.get_eth_src_mask()[3];
			((uint8_t*)&mmask)[4] = ofmatch.get_eth_src_mask()[4];
			((uint8_t*)&mmask)[5] = ofmatch.get_eth_src_mask()[5];
		}

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
								ofmatch.get_vlan_vid_mask());

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

	try {
		of12_match_t *match = of12_init_ip_ecn_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ip_ecn());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

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

	try {
		of12_match_t *match = of12_init_udp_dst_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_udp_dst());

		of12_add_match_to_entry(entry, match);
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
		of12_match_t *match = of12_init_icmpv4_type_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_icmpv4_type());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_icmpv4_code_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_icmpv4_code());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_arp_opcode();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_ARP_OP is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_arp_spa();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_ARP_SPA is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_arp_tpa();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_ARP_TPA is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_arp_sha();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_ARP_SHA is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_arp_tha();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_ARP_THA is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_ipv6_src();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_IPV6_SRC is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_ipv6_dst();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_IPV6_DST is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_ipv6_flabel();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_IPV6_FLABEL is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_icmpv6_type();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_IPV6_TYPE is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_icmpv6_code();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_IPV6_CODE is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_ipv6_nd_target();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_IPV6_ND_TARGET is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_icmpv6_neighbor_source_lladdr();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_IPV6_ND_SLL is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		ofmatch.get_icmpv6_neighbor_target_lladdr();

		throw eNotImplemented(std::string("of12_translation_utils::flow_mod_add() OFPXMT_OFB_IPV6_ND_TLL is missing")); // TODO
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_mpls_label_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_mpls_label());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_mpls_tc_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_mpls_tc());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_pppoe_code_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_pppoe_code());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_pppoe_type_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_pppoe_type());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_pppoe_session_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_pppoe_sessid());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}

	try {
		of12_match_t *match = of12_init_ppp_prot_match(
								/*prev*/NULL,
								/*next*/NULL,
								ofmatch.get_ppp_prot());

		of12_add_match_to_entry(entry, match);
	} catch (eOFmatchNotFound& e) {}
}



/**
* Maps a of12_action from an OF1.2 Header
*/
void
of12_translation_utils::of12_map_flow_entry_actions(
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
		case OFPAT_COPY_TTL_OUT:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_COPY_TTL_OUT, 0, NULL, NULL);
			break;
		case OFPAT_COPY_TTL_IN:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_COPY_TTL_IN, 0, NULL, NULL);
			break;
		case OFPAT_SET_MPLS_TTL:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_MPLS_TTL, raction.oac_mpls_ttl->mpls_ttl, NULL, NULL);
			break;
		case OFPAT_DEC_MPLS_TTL:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_DEC_MPLS_TTL, 0, NULL, NULL);
			break;
		case OFPAT_PUSH_VLAN:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_PUSH_VLAN, be16toh(raction.oac_oacu.oacu_push->ethertype), NULL, NULL);
			break;
		case OFPAT_POP_VLAN:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_POP_VLAN, be16toh(raction.oac_push->ethertype), NULL, NULL);
			break;
		case OFPAT_PUSH_MPLS:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_PUSH_MPLS, be16toh(raction.oac_push->ethertype), NULL, NULL);
			break;
		case OFPAT_POP_MPLS:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_POP_MPLS,  be16toh(raction.oac_push->ethertype), NULL, NULL);
			break;
		case OFPAT_SET_QUEUE:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_QUEUE, be32toh(raction.oac_set_queue->queue_id), NULL, NULL);
			break;
		case OFPAT_GROUP:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_GROUP, be32toh(raction.oac_group->group_id), NULL, NULL);
			break;
		case OFPAT_SET_NW_TTL:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_NW_TTL, raction.oac_nw_ttl->nw_ttl, NULL, NULL);
			break;
		case OFPAT_DEC_NW_TTL:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_DEC_NW_TTL, 0, NULL, NULL);
			break;
		case OFPAT_SET_FIELD:
		{
			coxmatch oxm = raction.get_oxm();

			switch (oxm.get_oxm_class()) {
			case OFPXMC_OPENFLOW_BASIC:
			{
				switch (oxm.get_oxm_field()) {
				case OFPXMT_OFB_ETH_DST:
				{
					uint64_t maddr = 0;
					{
						((uint8_t*)&maddr)[0] = oxm.oxm_uint48t->value[0];
						((uint8_t*)&maddr)[1] = oxm.oxm_uint48t->value[1];
						((uint8_t*)&maddr)[2] = oxm.oxm_uint48t->value[2];
						((uint8_t*)&maddr)[3] = oxm.oxm_uint48t->value[3];
						((uint8_t*)&maddr)[4] = oxm.oxm_uint48t->value[4];
						((uint8_t*)&maddr)[5] = oxm.oxm_uint48t->value[5];
					}
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_ETH_DST, maddr, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ETH_SRC:
				{
					uint64_t maddr = 0;
					{
						((uint8_t*)&maddr)[0] = oxm.oxm_uint48t->value[0];
						((uint8_t*)&maddr)[1] = oxm.oxm_uint48t->value[1];
						((uint8_t*)&maddr)[2] = oxm.oxm_uint48t->value[2];
						((uint8_t*)&maddr)[3] = oxm.oxm_uint48t->value[3];
						((uint8_t*)&maddr)[4] = oxm.oxm_uint48t->value[4];
						((uint8_t*)&maddr)[5] = oxm.oxm_uint48t->value[5];
					}
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_ETH_SRC, maddr, NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ETH_TYPE:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_ETH_TYPE, oxm.uint16_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ICMPV4_CODE:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_ICMPV4_CODE, oxm.uint8_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_ICMPV4_TYPE:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_ICMPV4_TYPE, oxm.uint8_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IPV4_DST:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_IPV4_DST, oxm.uint32_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IPV4_SRC:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_IPV4_SRC, oxm.uint32_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IP_DSCP:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_IP_DSCP, oxm.uint8_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IP_ECN:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_IP_ECN, oxm.uint8_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_IP_PROTO:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_IP_PROTO, oxm.uint8_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_MPLS_LABEL:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_MPLS_LABEL, oxm.uint32_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_MPLS_TC:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_MPLS_TC, oxm.uint8_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_PPPOE_CODE:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_PPPOE_CODE, oxm.uint8_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_PPPOE_TYPE:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_PPPOE_TYPE, oxm.uint8_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_PPPOE_SID:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_PPPOE_SID, oxm.uint16_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_PPP_PROT:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_PPP_PROT, oxm.uint16_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_VLAN_VID:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_VLAN_VID, oxm.uint16_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_VLAN_PCP:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_VLAN_PCP, oxm.uint8_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_TCP_DST:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_TCP_DST, oxm.uint16_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_TCP_SRC:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_TCP_SRC, oxm.uint16_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_UDP_DST:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_UDP_DST, oxm.uint16_value(), NULL, NULL);
				}
					break;
				case OFPXMT_OFB_UDP_SRC:
				{
					action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_SET_FIELD_UDP_SRC, oxm.uint16_value(), NULL, NULL);
				}
					break;
				default:
				{
					WRITELOG(CDATAPATH, ERROR, "of12_endpoint(%s)::of12_map_flow_entry() "
							"unknown OXM type in action SET-FIELD found: %s",
							sw->dpname.c_str(), raction.c_str());
				}
					break;
				}
			}
				break;
			default:
			{
				WRITELOG(CDATAPATH, ERROR, "of12_endpoint(%s)::of12_map_flow_entry() "
						"unknown OXM class in action SET-FIELD found: %s",
						sw->dpname.c_str(), raction.c_str());
			}
				break;
			}
		}
			break;
		case OFPAT_PUSH_PPPOE:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_PUSH_PPPOE, be16toh(raction.oac_push->ethertype), NULL, NULL);
			break;
		case OFPAT_POP_PPPOE:
			action = of12_init_packet_action(/*(of12_switch_t*)sw,*/ OF12_AT_POP_PPPOE, be16toh(raction.oac_pop_pppoe->ethertype), NULL, NULL);
			break;
		case OFPAT_EXPERIMENTER:
			// TODO
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
of12_translation_utils::of12_map_reverse_flow_entry_matches(
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
			uint64_t addr = ((utern64_t*)m->value)->value;
			cmacaddr maddr((uint8_t*)&addr,OFP_ETH_ALEN);
			addr = ((utern64_t*)m->value)->mask;
			cmacaddr mmask((uint8_t*)&addr,OFP_ETH_ALEN);
			
			match.set_eth_dst(maddr, mmask);
		}
			break;
		case OF12_MATCH_ETH_SRC:
		{
			uint64_t addr = ((utern64_t*)m->value)->value;
			cmacaddr maddr((uint8_t*)&addr,OFP_ETH_ALEN);
			addr = ((utern64_t*)m->value)->mask;
			cmacaddr mmask((uint8_t*)&addr,OFP_ETH_ALEN);
			
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
			addr.ca_s4addr->sin_addr.s_addr = ((utern32_t*)(m->value))->value;
			match.set_ipv4_src(addr);

		}
			break;
		case OF12_MATCH_IPV4_DST:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = ((utern32_t*)(m->value))->value;
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
		case OF12_MATCH_ARP_OP:
			match.set_arp_opcode(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_ARP_SPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = ((utern32_t*)(m->value))->value;
			match.set_arp_spa(addr);
		}
			break;
		case OF12_MATCH_ARP_TPA:
		{
			caddress addr(AF_INET, "0.0.0.0");
			addr.ca_s4addr->sin_addr.s_addr = ((utern32_t*)(m->value))->value;
			match.set_arp_tpa(addr);
		}
			break;
		case OF12_MATCH_ARP_SHA:
		{
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() ARP_SHA"));
		}
			break;
		case OF12_MATCH_ARP_THA:
		{
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() ARP_THA"));
		}
			break;
		case OF12_MATCH_IPV6_SRC:
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_SRC"));
		case OF12_MATCH_IPV6_DST:
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_DST"));
		case OF12_MATCH_IPV6_FLABEL:
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_FLABEL"));
		case OF12_MATCH_ICMPV6_TYPE:
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() IPCMPV6_TYPE"));
		case OF12_MATCH_ICMPV6_CODE:
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() IPCMPV6_CODE"));
		case OF12_MATCH_IPV6_ND_TARGET:
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_ND_TARGET"));
		case OF12_MATCH_IPV6_ND_SLL:
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_ND_SLL"));
		case OF12_MATCH_IPV6_ND_TLL:
			throw eNotImplemented(std::string("of12_translation_utils::of12_map_reverse_flow_entry_matches() IPV6_ND_TLL"));
		case OF12_MATCH_MPLS_LABEL:
			match.set_mpls_label(((utern32_t*)(m->value))->value);
			break;
		case OF12_MATCH_MPLS_TC:
			match.set_mpls_tc(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_PPPOE_CODE:
			match.set_pppoe_code(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_PPPOE_TYPE:
			match.set_pppoe_type(((utern8_t*)(m->value))->value);
			break;
		case OF12_MATCH_PPPOE_SID:
			match.set_pppoe_sessid(((utern16_t*)(m->value))->value);
			break;
		case OF12_MATCH_PPP_PROT:
			match.set_ppp_prot(((utern16_t*)(m->value))->value);
			break;
		default:
			break;
		}


		m = m->next;
	}
}

/**
* Maps a of12_group_bucket from an OF1.2 Header
*/
void
of12_translation_utils::of12_map_bucket_list(
		cofctl *ctl,
		openflow_switch* sw,
		cofbclist& of_buckets,
		of12_bucket_list_t* bucket_list)
{	
	
	for(cofbclist::iterator jt = of_buckets.begin();jt != of_buckets.end();++jt){
		//for each bucket we must map its actions
		cofbucket& bucket_ptr = (*jt);
		of12_action_group_t* action_group = of12_init_action_group(NULL);
		if(action_group == NULL){
			//TODO Handle Error
		}
		
		of12_map_flow_entry_actions(ctl,sw,bucket_ptr.actions,action_group,NULL);
		of12_insert_bucket_in_list(bucket_list,of12_init_bucket(bucket_ptr.weight, bucket_ptr.watch_port, bucket_ptr.watch_group, action_group));
	}
}



/**
*
*/
void
of12_translation_utils::of12_map_reverse_flow_entry_instructions(
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
of12_translation_utils::of12_map_reverse_flow_entry_instruction(
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
		for (unsigned int i = 0; i < (sizeof(inst->write_actions->write_actions) / sizeof(of12_write_actions_t)); i++) {
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
of12_translation_utils::of12_map_reverse_flow_entry_action(
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
		cmacaddr maddr;
		maddr[0] = ((uint8_t*)&(of12_action->field))[0];
		maddr[1] = ((uint8_t*)&(of12_action->field))[1];
		maddr[2] = ((uint8_t*)&(of12_action->field))[2];
		maddr[3] = ((uint8_t*)&(of12_action->field))[3];
		maddr[4] = ((uint8_t*)&(of12_action->field))[4];
		maddr[5] = ((uint8_t*)&(of12_action->field))[5];
		action = cofaction_set_field(coxmatch_ofb_eth_dst(maddr));
	} break;
	case OF12_AT_SET_FIELD_ETH_SRC: {
		cmacaddr maddr;
		maddr[0] = ((uint8_t*)&(of12_action->field))[0];
		maddr[1] = ((uint8_t*)&(of12_action->field))[1];
		maddr[2] = ((uint8_t*)&(of12_action->field))[2];
		maddr[3] = ((uint8_t*)&(of12_action->field))[3];
		maddr[4] = ((uint8_t*)&(of12_action->field))[4];
		maddr[5] = ((uint8_t*)&(of12_action->field))[5];
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
		action = cofaction_set_field(coxmatch_ofb_pppoe_code((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_PPPOE_TYPE: {
		action = cofaction_set_field(coxmatch_ofb_pppoe_type((uint8_t)(of12_action->field & OF12_AT_1_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_PPPOE_SID: {
		action = cofaction_set_field(coxmatch_ofb_pppoe_sid((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
	} break;
	case OF12_AT_SET_FIELD_PPP_PROT: {
		action = cofaction_set_field(coxmatch_ofb_ppp_prot((uint16_t)(of12_action->field & OF12_AT_2_BYTE_MASK)));
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


