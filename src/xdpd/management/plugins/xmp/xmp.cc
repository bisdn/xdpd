/*
 * xmp.cc
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#include "xmp.h"

using namespace xdpd::mgmt::protocol;


xmp::xmp() :
		socket(this, AF_INET, SOCK_DGRAM, IPPROTO_UDP, 10),
		udp_addr(MGMT_PORT_UDP_ADDR),
		udp_port(MGMT_PORT_UDP_PORT)
{

}


xmp::~xmp()
{

}



void
xmp::init(int args, char** argv)
{
	std::cerr << "[xdpd][xmp] initializing ..." << std::endl;
	socket.clisten(caddress(AF_INET, udp_addr.c_str(), udp_port), AF_INET, SOCK_DGRAM, IPPROTO_UDP, 10);
}



void
xmp::handle_timeout(
		int opaque)
{
	switch (opaque) {
	default:
		;;
	}
}


void
xmp::handle_read(
		csocket *socket,
		int sd)
{
	cmemory mem(128);

	int nbytes = ::read(socket->sd, mem.somem(), mem.memlen());

	if (nbytes == 0) {
		// socket closed
		std::cerr << "[xdpd][xmp] reading xmp socket failed, errno:"
				<< errno << " (" << strerror(errno) << ")" << std::endl;
		return;
	} else if (nbytes < 0) {
		std::cerr << "[xdpd][xmp] reading xmp socket failed, errno:"
				<< errno << " (" << strerror(errno) << ")" << std::endl;
		return;
	}

	if ((unsigned int)nbytes < sizeof(struct xmp_header_t)) {
		std::cerr << "[xdpd][xmp] short packet rcvd, rc:" << nbytes << std::endl;
		return;
	}

	struct xmp_header_t *hdr = (struct xmp_header_t*)mem.somem();
	cxmpmsg msg(mem.somem(), nbytes);

	switch (hdr->type) {
	case XMPT_REQUEST: {
		handle_request(msg);
	} break;
	case XMPT_REPLY:
	case XMPT_NOTIFICATION:
	default: {
		std::cerr << "[xdpd][xmp] unknown message rcvd" << std::endl;
	};
	}
}


void
xmp::handle_request(
		cxmpmsg& msg)
{
	std::cerr << "[xdpd][xmp] rcvd message:" << std::endl << msg;

	if (not msg.get_xmpies().has_ie_command()) {
		std::cerr << "[xdpd][xmp] rcvd xmp request without -COMMAND- IE, dropping message." << std::endl;
		return;
	}

	switch (msg.get_xmpies().get_ie_command().get_command()) {
	case XMPIEMCT_PORT_ATTACH: {
		handle_port_attach(msg);
	} break;
	case XMPIEMCT_PORT_DETACH: {
		handle_port_detach(msg);
	} break;
	case XMPIEMCT_PORT_ENABLE: {
		handle_port_enable(msg);
	} break;
	case XMPIEMCT_PORT_DISABLE: {
		handle_port_disable(msg);
	} break;
	case XMPIEMCT_NONE:
	default: {
		std::cerr << "[xdpd][xmp] rcvd xmp request with unknown command:"
				<< (int)msg.get_xmpies().get_ie_command().get_command() << ", dropping message." << std::endl;
		return;
	};
	}
}


void
xmp::handle_port_attach(
		cxmpmsg& msg)
{
	std::string portname;
	uint64_t dpid = 0;

	try {
		if (not msg.get_xmpies().has_ie_portname()) {
			std::cerr << "[xdpd][xmp] rcvd xmp Port-Attach request without -PORTNAME- IE, dropping message." << std::endl;
			return;
		}

		if (not msg.get_xmpies().has_ie_dpid()) {
			std::cerr << "[xdpd][xmp] rcvd xmp Port-Attach request without -DPID- IE, dropping message." << std::endl;
			return;
		}

		portname = msg.get_xmpies().get_ie_portname().get_portname();
		dpid = msg.get_xmpies().get_ie_dpid().get_dpid();

		unsigned int of_port_num;
		port_manager::attach_port_to_switch(dpid, portname, &of_port_num);
		std::cerr << "[xdpd][xmp] attached port:" << portname
				<< " to dpid:" << (unsigned long long)dpid << " "
				<< " port-no:" << of_port_num << std::endl;

	} catch(eOfSmDoesNotExist& e) {
		std::cerr << "[xdpd][xmp] attaching port:" << portname
				<< " to dpid:" << (unsigned long long)dpid << " failed, LSI does not exist" << std::endl;

	} catch(ePmInvalidPort& e) {
		std::cerr << "[xdpd][xmp] attaching port:" << portname
				<< " to dpid:" << (unsigned long long)dpid << " failed, port does not exist" << std::endl;

	} catch(eOfSmGeneralError& e) {
		std::cerr << "[xdpd][xmp] attaching port:" << portname
				<< " to dpid:" << (unsigned long long)dpid << " failed." << std::endl;

	}
}


void
xmp::handle_port_detach(
		cxmpmsg& msg)
{
	std::string portname;
	uint64_t dpid = 0;

	try {
		if (not msg.get_xmpies().has_ie_portname()) {
			std::cerr << "[xdpd][xmp] rcvd xmp Port-Detach request without -PORTNAME- IE, dropping message." << std::endl;
			return;
		}

		if (not msg.get_xmpies().has_ie_dpid()) {
			std::cerr << "[xdpd][xmp] rcvd xmp Port-Detach request without -DPID- IE, dropping message." << std::endl;
			return;
		}

		portname = msg.get_xmpies().get_ie_portname().get_portname();
		dpid = msg.get_xmpies().get_ie_dpid().get_dpid();

		port_manager::detach_port_from_switch(dpid, portname);
		std::cerr << "[xdpd][xmp] detached port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << std::endl;

	} catch(eOfSmDoesNotExist& e) {
		std::cerr << "[xdpd][xmp] detaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed, LSI does not exist" << std::endl;

	} catch(ePmInvalidPort& e) {
		std::cerr << "[xdpd][xmp] detaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed, port does not exist" << std::endl;

	} catch(eOfSmGeneralError& e) {
		std::cerr << "[xdpd][xmp] detaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed." << std::endl;

	}
}


void
xmp::handle_port_enable(
		cxmpmsg& msg)
{
	std::string portname;

	try {
		if (not msg.get_xmpies().has_ie_portname()) {
			std::cerr << "[xdpd][xmp] rcvd xmp Port-Enable request without -PORTNAME- IE, dropping message." << std::endl;
			return;
		}

		portname = msg.get_xmpies().get_ie_portname().get_portname();

		port_manager::enable_port(portname);
		std::cerr << "[xdpd][xmp] enabled port:" << portname << std::endl;

	} catch(ePmInvalidPort& e) {
		std::cerr << "[xdpd][xmp] enabling port:" << portname << " failed, port does not exist" << std::endl;

	} catch(eOfSmGeneralError& e) {
		std::cerr << "[xdpd][xmp] enabling port:" << portname << " failed." << std::endl;

	}
}


void
xmp::handle_port_disable(
		cxmpmsg& msg)
{
	std::string portname;

	try {
		if (not msg.get_xmpies().has_ie_portname()) {
			std::cerr << "[xdpd][xmp] rcvd xmp Port-Disable request without -PORTNAME- IE, dropping message." << std::endl;
			return;
		}

		portname = msg.get_xmpies().get_ie_portname().get_portname();

		port_manager::disable_port(portname);
		std::cerr << "[xdpd][xmp] disabled port:" << portname << std::endl;

	} catch(ePmInvalidPort& e) {
		std::cerr << "[xdpd][xmp] disabling port:" << portname << " failed, port does not exist" << std::endl;

	} catch(eOfSmGeneralError& e) {
		std::cerr << "[xdpd][xmp] disabling port:" << portname << " failed." << std::endl;

	}
}



