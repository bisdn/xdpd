/*
 * xdpd_manager.cc
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#include "xdpd_manager.h"

using namespace xdpd::mgmt;

xdpd_manager* xdpd_manager::sxdpdmanager = (xdpd_manager*)0;

xdpd_manager&
xdpd_manager::get_instance() {
	if (0 == xdpd_manager::sxdpdmanager) {
		xdpd_manager::sxdpdmanager = new xdpd_manager();
	}
	return *(xdpd_manager::sxdpdmanager);
}


xdpd_manager::xdpd_manager(
		std::string const& udp_addr,
		uint16_t udp_port) :
				socket(this, AF_INET, SOCK_DGRAM, IPPROTO_UDP, 10),
				udp_addr(udp_addr),
				udp_port(udp_port)
{
	std::cerr << "[xdpd][manager] initializing ..." << std::endl;
	socket.clisten(caddress(AF_INET, udp_addr.c_str(), udp_port), AF_INET, SOCK_DGRAM, IPPROTO_UDP, 10);
}


xdpd_manager::~xdpd_manager()
{

}


void
xdpd_manager::handle_timeout(
		int opaque)
{
	switch (opaque) {
	default:
		;;
	}
}


void
xdpd_manager::handle_read(
		csocket *socket,
		int sd)
{

}


