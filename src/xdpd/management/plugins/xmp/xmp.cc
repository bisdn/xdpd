/*
 * xmp.cc
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#include "xmp.h"

using namespace xdpd::mgmt;


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
	std::cerr << "[xdpd][manager] initializing ..." << std::endl;
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

	switch (hdr->type) {
	case XMPT_PORT_ATTACH: {
		handle_port_attach(protocol::cxmpmsg_port_attachment(mem.somem(), nbytes));
	} break;
	case XMPT_PORT_DETACH: {
		handle_port_detach(protocol::cxmpmsg_port_attachment(mem.somem(), nbytes));
	} break;
	case XMPT_PORT_ENABLE: {
		handle_port_enable(protocol::cxmpmsg_port_configuration(mem.somem(), nbytes));
	} break;
	case XMPT_PORT_DISABLE: {
		handle_port_disable(protocol::cxmpmsg_port_configuration(mem.somem(), nbytes));
	} break;
	default: {
		std::cerr << "[xdpd][xmp] unknown message rcvd" << std::endl;
	};
	}
}


void
xmp::handle_port_attach(
		protocol::cxmpmsg_port_attachment const& msg)
{
	try {
		unsigned int of_port_num;
		port_manager::attach_port_to_switch(msg.get_dpid(), msg.get_portname(), &of_port_num);
		std::cerr << "[xdpd][xmp] attached port:" << msg.get_portname()
				<< " from dpid:" << (unsigned long long)msg.get_dpid() << " "
				<< " port-no:" << of_port_num << std::endl;

	} catch(eOfSmDoesNotExist& e) {
		std::cerr << "[xdpd][xmp] attaching port:" << msg.get_portname()
				<< " to dpid:" << (unsigned long long)msg.get_dpid() << " failed, LSI does not exist" << std::endl;

	} catch(ePmInvalidPort& e) {
		std::cerr << "[xdpd][xmp] attaching port:" << msg.get_portname()
				<< " to dpid:" << (unsigned long long)msg.get_dpid() << " failed, port does not exist" << std::endl;

	} catch(eOfSmGeneralError& e) {
		std::cerr << "[xdpd][xmp] attaching port:" << msg.get_portname()
				<< " to dpid:" << (unsigned long long)msg.get_dpid() << " failed." << std::endl;

	}
}


void
xmp::handle_port_detach(
		protocol::cxmpmsg_port_attachment const& msg)
{
	try {
		port_manager::detach_port_from_switch(msg.get_dpid(), msg.get_portname());
		std::cerr << "[xdpd][xmp] detached port:" << msg.get_portname()
				<< " from dpid:" << (unsigned long long)msg.get_dpid() << std::endl;

	} catch(eOfSmDoesNotExist& e) {
		std::cerr << "[xdpd][xmp] detaching port:" << msg.get_portname()
				<< " from dpid:" << (unsigned long long)msg.get_dpid() << " failed, LSI does not exist" << std::endl;

	} catch(ePmInvalidPort& e) {
		std::cerr << "[xdpd][xmp] detaching port:" << msg.get_portname()
				<< " from dpid:" << (unsigned long long)msg.get_dpid() << " failed, port does not exist" << std::endl;

	} catch(eOfSmGeneralError& e) {
		std::cerr << "[xdpd][xmp] detaching port:" << msg.get_portname()
				<< " from dpid:" << (unsigned long long)msg.get_dpid() << " failed." << std::endl;

	}
}


void
xmp::handle_port_enable(
		protocol::cxmpmsg_port_configuration const& msg)
{
	try {
		port_manager::enable_port(msg.get_portname());
		std::cerr << "[xdpd][xmp] enabled port:" << msg.get_portname() << std::endl;

	} catch(ePmInvalidPort& e) {
		std::cerr << "[xdpd][xmp] enabling port:" << msg.get_portname() << " failed, port does not exist" << std::endl;

	} catch(eOfSmGeneralError& e) {
		std::cerr << "[xdpd][xmp] enabling port:" << msg.get_portname() << " failed." << std::endl;

	}
}


void
xmp::handle_port_disable(
		protocol::cxmpmsg_port_configuration const& msg)
{
	try {
		port_manager::disable_port(msg.get_portname());
		std::cerr << "[xdpd][xmp] disabled port:" << msg.get_portname() << std::endl;

	} catch(ePmInvalidPort& e) {
		std::cerr << "[xdpd][xmp] disabling port:" << msg.get_portname() << " failed, port does not exist" << std::endl;

	} catch(eOfSmGeneralError& e) {
		std::cerr << "[xdpd][xmp] disabling port:" << msg.get_portname() << " failed." << std::endl;

	}
}



