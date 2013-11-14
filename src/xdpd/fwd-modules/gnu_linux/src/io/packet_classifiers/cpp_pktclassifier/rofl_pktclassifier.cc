#include "rofl_pktclassifier.h"
#include <rofl/common/utils/c_logger.h>
#include "../datapacketx86.h"

using namespace xdpd::gnu_linux;

#define ROFL_PKT_CLASSIFIER_MAX_NUM_OF_FRAMES 16
#define ROFL_PKT_CLASSIFIER_IS_LAST_FRAME(a) do{ if(a>ROFL_PKT_CLASSIFIER_MAX_NUM_OF_FRAMES) a = ROFL_PKT_CLASSIFIER_MAX_NUM_OF_FRAMES; }while(0)

void rofl_pktclassifier::classify(void){
	classify_reset();
	parse_ether(pkt->get_buffer(), pkt->get_buffer_length());
	is_classified = true;
}

void
rofl_pktclassifier::frame_append(
		rofl::fframe *frame)
{
	if ((NULL == fhead) && (NULL == ftail))
	{
		fhead = ftail = frame;
		frame->next = NULL;
		frame->prev = NULL;
	}
	else if ((fhead != NULL) && (ftail != NULL))
	{
		ftail->next = frame;
		frame->prev = ftail;
		ftail = frame;
	}
	else
	{
		// log error
	}
}


rofl::fframe*
rofl_pktclassifier::frame_insert(
		rofl::fframe *append_to,
		rofl::fframe *frame)
{
	if ((!append_to) || (!frame))
		return NULL;
	frame->next = append_to->next;
	frame->prev = append_to;
	if (frame->next)
	{
		frame->next->prev = frame;
	}
	if (frame->prev)
	{
		frame->prev->next = frame;
	}
	if (ftail == append_to)
	{
		ftail = frame;
	}
	return frame;
}


void
rofl_pktclassifier::frame_push(
		rofl::fframe *frame)
{
	if ((NULL == fhead) || (NULL == ftail))
	{
		fhead = ftail = frame;
	}
	else if ((NULL != fhead) && (NULL != ftail))
	{
		frame->next 	= fhead->next;
		fhead->next 	= frame;
		frame->prev 	= fhead;

		if (NULL != frame->next)
		{
			frame->next->prev = frame;
		}
		else
		{
			ftail = frame;
		}
	}
	else
	{
		// log error
	}
}



void
rofl_pktclassifier::frame_pop(
		rofl::fframe *frame)
{
	// check whether this is the second fframe (first after ether)
	// if not refuse dropping

	//ROFL_DEBUG("datapacketx86(%p)::frame_pop() "
	//		"frame: %p %s\n", this, frame, frame->c_str());

#if 0
	if ((0 == head) || (head->next != frame))
	{
		throw ePacketInval();
	}
#endif

	if (NULL != frame->next)
	{
		frame->next->prev = frame->prev;
	}
	else
	{
		ftail = frame->prev;
	}

	if (NULL != frame->prev)
	{
		frame->prev->next = frame->next;
	}
	else
	{
		fhead = frame->next;
	}

	frame->next = NULL;
	frame->prev = NULL; 
}



void
rofl_pktclassifier::parse_ether(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::fetherframe::eth_hdr_t)) { return; }

	rofl::fetherframe *ether =
			new rofl::fetherframe(p_ptr, sizeof(struct rofl::fetherframe::eth_hdr_t));

	frame_append(ether);
	t_frames[ROFL_PKT_CLASSIFIER_ETHER].push_back(ether);

	p_ptr += sizeof(struct rofl::fetherframe::eth_hdr_t);
	p_len -= sizeof(struct rofl::fetherframe::eth_hdr_t);

	eth_type = ether->get_dl_type();

	switch (eth_type) {
	case rofl::fvlanframe::VLAN_CTAG_ETHER:
	case rofl::fvlanframe::VLAN_STAG_ETHER:
	case rofl::fvlanframe::VLAN_ITAG_ETHER:
		{
			parse_vlan(p_ptr, p_len);
		}
		break;
	case rofl::fmplsframe::MPLS_ETHER:
	case rofl::fmplsframe::MPLS_ETHER_UPSTREAM:
		{
			parse_mpls(p_ptr, p_len);
		}
		break;
	case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
	case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
		{
			parse_pppoe(p_ptr, p_len);
		}
		break;
	case rofl::farpv4frame::ARPV4_ETHER:
		{
			parse_arpv4(p_ptr, p_len);
		}
		break;
	case rofl::fipv4frame::IPV4_ETHER:
		{
			parse_ipv4(p_ptr, p_len);
		}
		break;
	case rofl::fipv6frame::IPV6_ETHER:
		{
			parse_ipv6(p_ptr,p_len);
		}
		break;	
	default:
		{
			if (p_len > 0)
			{
				rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

				frame_append(payload);
			}
		}
		break;
	}
}



void
rofl_pktclassifier::parse_vlan(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::fvlanframe::vlan_hdr_t)) { return; }

	rofl::fvlanframe *vlan =
			new rofl::fvlanframe(p_ptr, sizeof(struct rofl::fvlanframe::vlan_hdr_t));

	// TODO: store VID and PCP for match

	frame_append(vlan);
	t_frames[ROFL_PKT_CLASSIFIER_VLAN].push_back(vlan);

	p_ptr += sizeof(struct rofl::fvlanframe::vlan_hdr_t);
	p_len -= sizeof(struct rofl::fvlanframe::vlan_hdr_t);

	eth_type = vlan->get_dl_type();

	switch (eth_type) {
	case rofl::fvlanframe::VLAN_CTAG_ETHER:
	case rofl::fvlanframe::VLAN_STAG_ETHER:
	case rofl::fvlanframe::VLAN_ITAG_ETHER:
		{
			parse_vlan(p_ptr, p_len);
		}
		break;
	case rofl::fmplsframe::MPLS_ETHER:
	case rofl::fmplsframe::MPLS_ETHER_UPSTREAM:
		{
			parse_mpls(p_ptr, p_len);
		}
		break;
	case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
	case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
		{
			parse_pppoe(p_ptr, p_len);
		}
		break;
	case rofl::farpv4frame::ARPV4_ETHER:
		{
			parse_arpv4(p_ptr, p_len);
		}
		break;
	case rofl::fipv4frame::IPV4_ETHER:
		{
			parse_ipv4(p_ptr, p_len);
		}
		break;
	default:
		{
			if (p_len > 0)
			{
				rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

				frame_append(payload);
			}
		}
		break;
	}
}




void
rofl_pktclassifier::parse_mpls(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::fmplsframe::mpls_hdr_t)) { return; }

	rofl::fmplsframe *mpls =
			new rofl::fmplsframe(p_ptr, sizeof(struct rofl::fmplsframe::mpls_hdr_t));

	// TODO: store outermost MPLS label

	frame_append(mpls);
	t_frames[ROFL_PKT_CLASSIFIER_MPLS].push_back(mpls);

	p_ptr += sizeof(struct rofl::fmplsframe::mpls_hdr_t);
	p_len -= sizeof(struct rofl::fmplsframe::mpls_hdr_t);


	if (not mpls->get_mpls_bos())
	{
		parse_mpls(p_ptr, p_len);
	}
	else
	{
		if (p_len > 0)
		{
			rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

			frame_append(payload);
		}
	}
}



void
rofl_pktclassifier::parse_pppoe(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::fpppoeframe::pppoe_hdr_t)) { return; }

	rofl::fpppoeframe *pppoe =
			new rofl::fpppoeframe(p_ptr, sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));

	frame_append(pppoe);
	t_frames[ROFL_PKT_CLASSIFIER_PPPOE].push_back(pppoe);

	switch (eth_type) {
	case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
		{
			p_len -= sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);

			uint16_t pppoe_len = pppoe->get_hdr_length() > p_len ? p_len : pppoe->get_hdr_length();

			/*
			 * parse any pppoe service tags
			 */
			pppoe->unpack(
					p_ptr,
					sizeof(struct rofl::fpppoeframe::pppoe_hdr_t) + pppoe_len);


			/*
			 * any remaining bytes after the pppoe tags => padding?
			 */
			if (p_len > pppoe->tags.length())
			{
				rofl::fframe *payload = new rofl::fframe(p_ptr, p_len - pppoe->tags.length());

				frame_append(payload);
			}
		}
		break;
	case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
		{
			p_ptr += sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);
			p_len -= sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);

			parse_ppp(p_ptr, p_len);
		}
		break;
	default:
		{
			// log error?
		}
		break;
	}


}



void
rofl_pktclassifier::parse_ppp(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::fpppframe::ppp_hdr_t)) { return; }

	rofl::fpppframe *ppp =
			new rofl::fpppframe(p_ptr, sizeof(struct rofl::fpppframe::ppp_hdr_t));

	frame_append(ppp);
	t_frames[ROFL_PKT_CLASSIFIER_PPP].push_back(ppp);

	switch (ppp->get_ppp_prot()) {
	case rofl::fpppframe::PPP_PROT_IPV4:
		{
			p_ptr += sizeof(struct rofl::fpppframe::ppp_hdr_t);
			p_len -= sizeof(struct rofl::fpppframe::ppp_hdr_t);

			parse_ipv4(p_ptr, p_len);
		}
		break;
	default:
		{
			ppp->unpack(p_ptr, p_len);
		}
		break;
	}
}



void
rofl_pktclassifier::parse_arpv4(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::farpv4frame::arpv4_hdr_t)) { return; }

	rofl::farpv4frame *arpv4 =
			new rofl::farpv4frame(p_ptr, sizeof(struct rofl::farpv4frame::arpv4_hdr_t));

	frame_append(arpv4);
	t_frames[ROFL_PKT_CLASSIFIER_ARPV4].push_back(arpv4);

	p_ptr += sizeof(struct rofl::farpv4frame::arpv4_hdr_t);
	p_len -= sizeof(struct rofl::farpv4frame::arpv4_hdr_t);

	if (p_len > 0)
	{
		rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

		frame_append(payload);
	}
}



void
rofl_pktclassifier::parse_ipv4(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::fipv4frame::ipv4_hdr_t)) { return; }

	rofl::fipv4frame *ipv4 =
			new rofl::fipv4frame(p_ptr, sizeof(struct rofl::fipv4frame::ipv4_hdr_t));

	frame_append(ipv4);
	t_frames[ROFL_PKT_CLASSIFIER_IPV4].push_back(ipv4);

	p_ptr += sizeof(struct rofl::fipv4frame::ipv4_hdr_t);
	p_len -= sizeof(struct rofl::fipv4frame::ipv4_hdr_t);

	if (ipv4->has_MF_bit_set())
	{
		// TODO: fragment handling

		return;
	}

	// FIXME: IP header with options



	switch (ipv4->get_ipv4_proto()) {
	case rofl::fipv4frame::IPV4_IP_PROTO:
		{
			parse_ipv4(p_ptr, p_len);
		}
		break;
	case rofl::ficmpv4frame::ICMPV4_IP_PROTO:
		{
			parse_icmpv4(p_ptr, p_len);
		}
		break;
	case rofl::fudpframe::UDP_IP_PROTO:
		{
			parse_udp(p_ptr, p_len);
		}
		break;
	case rofl::ftcpframe::TCP_IP_PROTO:
		{
			parse_tcp(p_ptr, p_len);
		}
		break;
	case rofl::fsctpframe::SCTP_IP_PROTO:
		{
			parse_sctp(p_ptr, p_len);
		}
		break;
	default:
		{
			if (p_len > 0)
			{
				rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

				frame_append(payload);
			}
		}
		break;
	}
}



void
rofl_pktclassifier::parse_icmpv4(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t)) { return; }

	rofl::ficmpv4frame *icmpv4 =
			new rofl::ficmpv4frame(p_ptr, sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t));

	frame_append(icmpv4);
	t_frames[ROFL_PKT_CLASSIFIER_ICMPV4].push_back(icmpv4);

	p_ptr += sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t);
	p_len -= sizeof(struct rofl::ficmpv4frame::icmpv4_hdr_t);


	if (p_len > 0)
	{
		rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

		frame_append(payload);
	}
}



void
rofl_pktclassifier::parse_ipv6(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::fipv6frame::ipv6_hdr_t)) { return; }

	rofl::fipv6frame *ipv6 =
			new rofl::fipv6frame(p_ptr, sizeof(struct rofl::fipv6frame::ipv6_hdr_t));

	frame_append(ipv6);
	t_frames[ROFL_PKT_CLASSIFIER_IPV6].push_back(ipv6);

	p_ptr += sizeof(struct rofl::fipv6frame::ipv6_hdr_t);
	p_len -= sizeof(struct rofl::fipv6frame::ipv6_hdr_t);

	// TODO Header extensions

	switch (ipv6->get_next_header()) {
	case rofl::fipv4frame::IPV4_IP_PROTO:
		{
			parse_ipv4(p_ptr, p_len);
		}
		break;
	case rofl::ficmpv4frame::ICMPV4_IP_PROTO:
		{
			parse_icmpv4(p_ptr, p_len);
		}
		break;
	case rofl::fipv6frame::IPV6_IP_PROTO:
		{
			parse_ipv6(p_ptr, p_len);
		}
		break;
	case rofl::ficmpv6frame::ICMPV6_IP_PROTO:
		{
			parse_icmpv6(p_ptr, p_len);
		}
		break;
	case rofl::fudpframe::UDP_IP_PROTO:
		{
			parse_udp(p_ptr, p_len);
		}
		break;
	case rofl::ftcpframe::TCP_IP_PROTO:
		{
			parse_tcp(p_ptr, p_len);
		}
		break;
	case rofl::fsctpframe::SCTP_IP_PROTO:
		{
			parse_sctp(p_ptr, p_len);
		}
		break;
	default:
		{
			if (p_len > 0)
			{
				rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

				frame_append(payload);
			}
		}
		break;
	}
}



void
rofl_pktclassifier::parse_icmpv6(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::ficmpv6frame::icmpv6_hdr_t)) { return; }

	rofl::ficmpv6frame *icmpv6 =
			new rofl::ficmpv6frame(p_ptr, sizeof(struct rofl::ficmpv6frame::icmpv6_hdr_t));

	frame_append(icmpv6);
	t_frames[ROFL_PKT_CLASSIFIER_ICMPV4].push_back(icmpv6);

	p_ptr += sizeof(struct rofl::ficmpv6frame::icmpv6_hdr_t);
	p_len -= sizeof(struct rofl::ficmpv6frame::icmpv6_hdr_t);


	if (p_len > 0)
	{
		rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

		frame_append(payload);
	}
}


void
rofl_pktclassifier::parse_udp(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::fudpframe::udp_hdr_t)) { return; }

	rofl::fudpframe *udp =
			new rofl::fudpframe(p_ptr, sizeof(struct rofl::fudpframe::udp_hdr_t));

	frame_append(udp);
	t_frames[ROFL_PKT_CLASSIFIER_UDP].push_back(udp);

	p_ptr += sizeof(struct rofl::fudpframe::udp_hdr_t);
	p_len -= sizeof(struct rofl::fudpframe::udp_hdr_t);


	if (p_len > 0)
	{
		rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

		frame_append(payload);
	}
}



void
rofl_pktclassifier::parse_tcp(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::ftcpframe::tcp_hdr_t)) { return; }

	rofl::ftcpframe *tcp =
			new rofl::ftcpframe(p_ptr, sizeof(struct rofl::ftcpframe::tcp_hdr_t));

	frame_append(tcp);
	t_frames[ROFL_PKT_CLASSIFIER_TCP].push_back(tcp);

	p_ptr += sizeof(struct rofl::ftcpframe::tcp_hdr_t);
	p_len -= sizeof(struct rofl::ftcpframe::tcp_hdr_t);

	if (p_len > 0)
	{
		rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

		frame_append(payload);
	}

}



void
rofl_pktclassifier::parse_sctp(
		uint8_t *data,
		size_t datalen)
{
	uint8_t 	*p_ptr 		= data;
	size_t 		 p_len 		= datalen;

	if (p_len < sizeof(struct rofl::fsctpframe::sctp_hdr_t)) { return; }

	rofl::fsctpframe *sctp =
			new rofl::fsctpframe(p_ptr, sizeof(struct rofl::fsctpframe::sctp_hdr_t));

	frame_append(sctp);
	t_frames[ROFL_PKT_CLASSIFIER_SCTP].push_back(sctp);

	p_ptr += sizeof(struct rofl::fsctpframe::sctp_hdr_t);
	p_len -= sizeof(struct rofl::fsctpframe::sctp_hdr_t);

	if (p_len > 0)
	{
		rofl::fframe *payload = new rofl::fframe(p_ptr, p_len);

		frame_append(payload);
	}


}

void
rofl_pktclassifier::pop_vlan(){
	// outermost vlan tag, if any, following immediately the initial ethernet header
	if (!fhead)
		return;
	rofl::fvlanframe* vlan = dynamic_cast<rofl::fvlanframe*>( fhead->next );
	if (!vlan)
		return;

	uint16_t ether_type = vlan->get_dl_type();

	pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(rofl::fvlanframe::vlan_hdr_t));

	frame_pop(vlan);
	delete vlan;

	ether(0)->shift_right(sizeof(struct rofl::fvlanframe::vlan_hdr_t));

	ether(0)->set_dl_type(ether_type);
}

void
rofl_pktclassifier::pop_mpls(uint16_t ether_type){
	// outermost mpls tag, if any, following immediately the initial ethernet header
	if (!fhead)
		return;
	rofl::fmplsframe* mpls = dynamic_cast<rofl::fmplsframe*>( fhead->next );
	if (!mpls)
		return;
	pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(rofl::fmplsframe::mpls_hdr_t));
	frame_pop(mpls);

	ether(0)->shift_right(sizeof(rofl::fmplsframe::mpls_hdr_t));

	ether(0)->set_dl_type(ether_type);
}

void
rofl_pktclassifier::pop_pppoe(uint16_t ether_type){
	// outermost mpls tag, if any, following immediately the initial ethernet header
	if (!fhead)
		return;

	switch (ether(0)->get_dl_type()) {
	case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
	{
		pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(rofl::fpppoeframe::pppoe_hdr_t));
		if (this->pppoe(0)) {
			frame_pop(pppoe(0));
		}
		ether(0)->shift_right(sizeof(rofl::fpppoeframe::pppoe_hdr_t));
	}
		break;
	case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
	{
		pkt_pop(/*offset=*/sizeof(struct rofl::fetherframe::eth_hdr_t),
						sizeof(rofl::fpppoeframe::pppoe_hdr_t) + sizeof(rofl::fpppframe::ppp_hdr_t));
		if (this->pppoe(0)) {
			frame_pop(pppoe(0));
		}
		if (ppp(0)) {
			frame_pop(ppp(0));
		}
		ether(0)->shift_right(sizeof(rofl::fpppoeframe::pppoe_hdr_t) +
									sizeof(rofl::fpppframe::ppp_hdr_t));
	}
		break;
	}

	ether(0)->set_dl_type(ether_type);
}


/* PUSH operations
 *
 * we want to avoid a total re-classification of the packet, so we make
 * some assumptions:
 * - the packet is stored in one consecutive buffer
 * - a push operation always occurs behind the initial ethernet header (ether(0))
 * - byte insertion moves the affected first ethernet header to the left
 * - the doubly linked list of fframe instances remains valid, except ether(0), so no
 *   re-classification is necessary, but ether(0) must be readjusted
 */

rofl::fvlanframe*
rofl_pktclassifier::push_vlan(uint16_t ether_type){
	if ((NULL == ether(0)) || (NULL == ether(0)->next)){
		return NULL;
	}
#if 0
	if (X86_DATAPACKET_BUFFERED_IN_NIC == buffering_status){
		transfer_to_user_space();
	}
#endif
	if(!is_classified)
		classify(); // this ensures that ether(0) exists

	uint16_t inner_ether_type = ether(0)->get_dl_type();

	/*
	 * this invalidates ether(0), as it shifts ether(0) to the left
	 */
	if (pkt_push(sizeof(rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fvlanframe::vlan_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}

	/*
	 * adjust ether(0): move one vlan tag to the left
	 */
	ether(0)->shift_left(sizeof(struct rofl::fvlanframe::vlan_hdr_t));

	/*
	 * create new vlan tag
	 */
	rofl::fvlanframe *vlan = new rofl::fvlanframe(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fvlanframe::vlan_hdr_t));

	/*
	 * append the new fvlanframe instance to ether(0)
	 */
	frame_insert(ether(0), vlan);

	/*
	 * set default values in vlan tag
	 */
	if (this->vlan(1)){
		vlan->set_dl_vlan_id(this->vlan(1)->get_dl_vlan_id());
		vlan->set_dl_vlan_pcp(this->vlan(1)->get_dl_vlan_pcp());
	} else {
		vlan->set_dl_vlan_id(0x0000);
		vlan->set_dl_vlan_pcp(0x00);
	}
	vlan->set_dl_type(inner_ether_type);
	ether(0)->set_dl_type(ether_type);

	return vlan;
}

rofl::fmplsframe*
rofl_pktclassifier::push_mpls(uint16_t ether_type){
#if 0
	if (X86_DATAPACKET_BUFFERED_IN_NIC == buffering_status){
		transfer_to_user_space();
	}
#endif
	if(!is_classified)
		classify();

	/*
	 * this invalidates ether(0), as it shifts ether(0) to the left
	 */
	if (pkt_push(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fmplsframe::mpls_hdr_t)) == ROFL_FAILURE){
		// TODO: log error
		return 0;
	}

	/*
	 * adjust ether(0): move one mpls tag to the left
	 */
	ether(0)->shift_left(sizeof(struct rofl::fmplsframe::mpls_hdr_t));

	ether(0)->set_dl_type(ether_type);

	/*
	 * create new mpls tag
	 */
	rofl::fmplsframe *mpls = new rofl::fmplsframe(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fmplsframe::mpls_hdr_t));

	/*
	 * append the new fmplsframe instance to ether(0)
	 */
	frame_insert(ether(0), mpls);

	/*
	 * set default values in mpls tag
	 */
	if (this->mpls(1) != this->mpls(0)){
		mpls->set_mpls_bos(false);
		mpls->set_mpls_label(this->mpls(1)->get_mpls_label());
		mpls->set_mpls_tc(this->mpls(1)->get_mpls_tc());
		mpls->set_mpls_ttl(this->mpls(1)->get_mpls_ttl());
	} else {
		mpls->set_mpls_bos(true);
		mpls->set_mpls_label(0x0000);
		mpls->set_mpls_tc(0x00);
		mpls->set_mpls_ttl(0x00);
	}

	return mpls;
}

rofl::fpppoeframe*
rofl_pktclassifier::push_pppoe(uint16_t ether_type){

#if 0
	if (X86_DATAPACKET_BUFFERED_IN_NIC == buffering_status){
		transfer_to_user_space();
	}
#endif
	if(!is_classified)
		classify();
	
	if (pppoe(0)){
		// TODO: log error => pppoe tag already exists
		return NULL;
	}

	rofl::fpppoeframe *n_pppoe = NULL; 
	rofl::fpppframe *n_ppp = NULL; 

	switch (ether_type) {
	case rofl::fpppoeframe::PPPOE_ETHER_SESSION:
	{
		unsigned int bytes_to_insert = sizeof(struct rofl::fpppoeframe::pppoe_hdr_t) +
										 sizeof(struct rofl::fpppframe::ppp_hdr_t);

		/*
		 * this invalidates ether(0), as it shifts ether(0) to the left
		 */
		if (pkt_push(sizeof(struct rofl::fetherframe::eth_hdr_t), bytes_to_insert) == ROFL_FAILURE){
			// TODO: log error
			return NULL;
		}

		/*
		 * adjust ether(0): move one pppoe tag to the left
		 */
		ether(0)->shift_left(bytes_to_insert);

		ether(0)->set_dl_type(rofl::fpppoeframe::PPPOE_ETHER_SESSION);

		/*
		 * create new pppoe tag
		 */
		n_pppoe = new rofl::fpppoeframe(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));

		n_ppp = new rofl::fpppframe(n_pppoe->soframe() + sizeof(struct rofl::fpppoeframe::pppoe_hdr_t), sizeof(struct rofl::fpppframe::ppp_hdr_t));

		/*
		 * append the new fpppoeframe instance to ether(0)
		 */
		frame_insert(ether(0), n_pppoe);

		frame_insert(n_pppoe, n_ppp);

		/*
		 * TODO: check if this is an appropiate fix 
		 */
		n_pppoe->set_hdr_length(pkt->get_buffer_length() - sizeof(struct rofl::fetherframe::eth_hdr_t) - sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));

		n_ppp->set_ppp_prot(0x0000);
	}
		break;

	case rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY:
	{
		unsigned int bytes_to_insert = sizeof(struct rofl::fpppoeframe::pppoe_hdr_t);

		/*
		 * this invalidates ether(0), as it shifts ether(0) to the left
		 */
		if (pkt_push(ether(0)->payload(), bytes_to_insert) == ROFL_FAILURE){
			// TODO: log error
			return NULL;
		}

		/*
		 * adjust ether(0): move one pppoe tag to the left
		 */
		ether(0)->shift_left(bytes_to_insert);

		ether(0)->set_dl_type(rofl::fpppoeframe::PPPOE_ETHER_DISCOVERY);

		/*
		 * create new pppoe tag
		 */
		n_pppoe = new rofl::fpppoeframe(ether(0)->soframe() + sizeof(struct rofl::fetherframe::eth_hdr_t), sizeof(struct rofl::fpppoeframe::pppoe_hdr_t));

		/*
		 * append the new fpppoeframe instance to ether(0)
		 */
		frame_insert(ether(0), n_pppoe);

	}
		break;
	}

	/*
	 * set default values in pppoe tag
	 */
	n_pppoe->set_pppoe_code(0x00);
	n_pppoe->set_pppoe_sessid(0x0000);
	n_pppoe->set_pppoe_type(rofl::fpppoeframe::PPPOE_TYPE);
	n_pppoe->set_pppoe_vers(rofl::fpppoeframe::PPPOE_VERSION);

	return NULL;
}


template<class T>
T* rofl_pktclassifier::header(int idx) const
{
	rofl::fframe *frame;	
	T* tframe;

	if(idx >= 0){
		//Start from head
		frame = fhead;
	
		while (NULL != frame) {
			//Try to  dynamic cast	
			tframe = dynamic_cast<T*>( frame );
			
			if (NULL != tframe) {
				if (0 == idx)
					return tframe;
				else
					--idx;
			} else {
				//Move forward
				frame = frame->next;
			}
		}
	}else if(idx == -1){
		//Start from the tail
		//we want the inner most frame
		frame = ftail;
		
		while (NULL != frame) {
			//Try to  dynamic cast	
			tframe = dynamic_cast<T*>( frame );
			
			if (NULL != tframe) {
				return tframe;
			} else {
				//Move backwards
				frame = frame->prev;
			}
		}
	}
		
	//Not found or invalid idx
	return NULL;
}

//Protocol specific getters
rofl::fetherframe* rofl_pktclassifier::ether(int idx) const
{
	return rofl_pktclassifier::header<rofl::fetherframe>(idx);
}

rofl::fvlanframe* rofl_pktclassifier::vlan(int idx) const
{
	return rofl_pktclassifier::header<rofl::fvlanframe>(idx);
}

rofl::fmplsframe* rofl_pktclassifier::mpls(int idx) const
{
	return rofl_pktclassifier::header<rofl::fmplsframe>(idx);
}

rofl::farpv4frame* rofl_pktclassifier::arpv4(int idx) const
{
	return rofl_pktclassifier::header<rofl::farpv4frame>(idx);
}

rofl::fipv4frame* rofl_pktclassifier::ipv4(int idx) const
{
	return rofl_pktclassifier::header<rofl::fipv4frame>(idx);
}

rofl::ficmpv4frame* rofl_pktclassifier::icmpv4(int idx) const
{
	return rofl_pktclassifier::header<rofl::ficmpv4frame>(idx);
}

rofl::fudpframe* rofl_pktclassifier::udp(int idx) const
{
	return rofl_pktclassifier::header<rofl::fudpframe>(idx);
}

rofl::ftcpframe* rofl_pktclassifier::tcp(int idx) const
{
	return rofl_pktclassifier::header<rofl::ftcpframe>(idx);
}

rofl::fsctpframe* rofl_pktclassifier::sctp(int idx) const
{
	return rofl_pktclassifier::header<rofl::fsctpframe>(idx);
}

rofl::fipv6frame* rofl_pktclassifier::ipv6(int idx) const
{
	return rofl_pktclassifier::header<rofl::fipv6frame>(idx);
}

rofl::ficmpv6frame* rofl_pktclassifier::icmpv6(int idx) const
{
	return rofl_pktclassifier::header<rofl::ficmpv6frame>(idx);
}

rofl::fpppoeframe* rofl_pktclassifier::pppoe(int idx) const
{
	return rofl_pktclassifier::header<rofl::fpppoeframe>(idx);
}

rofl::fpppframe* rofl_pktclassifier::ppp(int idx) const
{
	return rofl_pktclassifier::header<rofl::fpppframe>(idx);
}

void rofl_pktclassifier::classify_reset(void){

	is_classified = false;
	rofl::fframe *frame = fhead;

	while (NULL != frame) {
		rofl::fframe *tframe = frame;
		frame = frame->next;

		tframe->next = tframe->prev = NULL;
		delete tframe;
	};
	fhead = ftail = NULL;

	for (unsigned int i = 0; i < ROFL_PKT_CLASSIFIER_MAX; i++) {
		t_frames[(enum rofl_pktclassifier_type_t)i].clear();
	}
}


void rofl_pktclassifier::dump(){
	ROFL_DEBUG("datapacketx86(%p) soframe: %p framelen: %zu\n", this, pkt->get_buffer(), pkt->get_buffer_length());
	rofl::fframe *frame = fhead;
	while (NULL != frame) {
		ROFL_DEBUG("%s\n", frame->c_str());
		frame = frame->next;
	}
	rofl::fframe content(pkt->get_buffer(), pkt->get_buffer_length());
	ROFL_DEBUG("content: %s\n", content.c_str());
}


std::ostream& operator<<(std::ostream& os, datapacketx86& pack){
	// TODO
	return os;
}


size_t
rofl_pktclassifier::get_pkt_len(rofl::fframe *from, rofl::fframe *to)
{
	rofl::fframe *curr = (from != 0) ? from : fhead;
	//rofl::fframe *last =   (to != 0) ?   to : ftail;

	size_t len = 0;

	while ((curr != 0)){// && (curr->next != last)) {
		len += curr->framelen();
		curr = curr->next;
	}
	return len;
}

