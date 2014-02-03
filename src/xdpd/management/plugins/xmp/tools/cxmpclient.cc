/*
 * cxmpclient.cc
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#include "cxmpclient.h"

using namespace xdpd::mgmt::protocol;

cxmpclient::cxmpclient() :
		socket(this, AF_INET, SOCK_DGRAM, IPPROTO_UDP, 10),
		laddr(AF_INET, "0.0.0.0", 0),
		raddr(AF_INET, "127.0.0.1", 8444)
{
	socket.connect(
			raddr,
			laddr,
			AF_INET,
			SOCK_DGRAM,
			IPPROTO_UDP);
}


cxmpclient::~cxmpclient()
{

}


void
cxmpclient::handle_timeout(
		int opaque, void *data)
{
	switch (opaque) {
	case TIMER_XMPCLNT_EXIT: {
		exit(0);
	} break;
	default: {

	};
	}
}


void
cxmpclient::port_attach(
		uint64_t dpid, std::string const& portname)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);

	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_ATTACH);
	msg.get_xmpies().add_ie_portname().set_portname(portname);
	msg.get_xmpies().add_ie_dpid().set_dpid(dpid);

	std::cerr << "[xmpclient] sending Port-Attach request:" << std::endl << msg;

	rofl::cmemory *mem = new rofl::cmemory(msg.length());

	msg.pack(mem->somem(), mem->memlen());

	socket.send(mem, raddr);

	register_timer(TIMER_XMPCLNT_EXIT, 1);
}


void
cxmpclient::port_detach(
		uint64_t dpid, std::string const& portname)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);

	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_DETACH);
	msg.get_xmpies().add_ie_portname().set_portname(portname);
	msg.get_xmpies().add_ie_dpid().set_dpid(dpid);

	std::cerr << "[xmpclient] sending Port-Detach request:" << std::endl << msg;

	rofl::cmemory *mem = new rofl::cmemory(msg.length());

	msg.pack(mem->somem(), mem->memlen());

	socket.send(mem, raddr);

	register_timer(TIMER_XMPCLNT_EXIT, 1);
}


void
cxmpclient::port_enable(
		std::string const& portname)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);

	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_ENABLE);
	msg.get_xmpies().add_ie_portname().set_portname(portname);

	std::cerr << "[xmpclient] sending Port-Enable request:" << std::endl << msg;

	rofl::cmemory *mem = new rofl::cmemory(msg.length());

	msg.pack(mem->somem(), mem->memlen());

	socket.send(mem, raddr);

	register_timer(TIMER_XMPCLNT_EXIT, 1);
}


void
cxmpclient::port_disable(
		std::string const& portname)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);

	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_DISABLE);
	msg.get_xmpies().add_ie_portname().set_portname(portname);

	std::cerr << "[xmpclient] sending Port-Disable request:" << std::endl << msg;

	rofl::cmemory *mem = new rofl::cmemory(msg.length());

	msg.pack(mem->somem(), mem->memlen());

	socket.send(mem, raddr);

	register_timer(TIMER_XMPCLNT_EXIT, 1);
}


