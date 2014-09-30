/*
 * cxmpie_controller.cc
 *
 *  Created on: Sep 26, 2014
 *      Author: tobi
 */

#include "cxmpie_controller.h"
#include <string.h>
#include <typeinfo>

using namespace xdpd::mgmt::protocol;

cxmpie_controller::cxmpie_controller() :
		cxmpie(XMPIET_CONTROLLER, sizeof(struct xmp_ie_controller_t))
{
}

cxmpie_controller::cxmpie_controller(const struct controller c) :
		cxmpie(XMPIET_CONTROLLER, sizeof(struct xmp_ie_controller_t))
{
	xmpie_generic = somem();

	set_id(c.id);
	set_proto(c.proto);
	set_ip_domain(c.ip_domain);
	set_ip_address(c.ip_domain, c.address);
	set_port(c.port);
}

cxmpie_controller::cxmpie_controller(uint8_t *buf, size_t buflen) :
		cxmpie(buf, buflen)
{
	xmpie_generic = somem();
	if (buflen < sizeof(struct xmp_ie_controller_t)) throw eXmpIeInval();
	unpack(buf, buflen);
}

cxmpie_controller::cxmpie_controller(cxmpie_controller const& elem) :
		cxmpie(elem)
{
	*this = elem;
}

cxmpie_controller&
cxmpie_controller::operator=(cxmpie_controller const& elem)
{
	if (this == &elem) return *this;

	cxmpie::operator=(elem);

	xmpie_generic = somem();

	return *this;
}

cxmpie_controller::cxmpie_controller(cxmpie const& elem) :
		cxmpie(elem)
{
	*this = elem;
}

cxmpie_controller&
cxmpie_controller::operator=(cxmpie const& elem)
{
	if (this == &elem) return *this;

	if (XMPIET_CONTROLLER != elem.get_type()) throw eXmpIeInval();
	if (elem.length() < sizeof(struct xmp_ie_controller_t)) throw eXmpIeInval();

	cxmpie::operator=(elem);

	xmpie_generic = somem();

	return *this;
}

cxmpie_controller::~cxmpie_controller()
{

}

uint8_t*
cxmpie_controller::resize(size_t len)
{
	return (xmpie_generic = cxmpie::resize(len));
}

size_t
cxmpie_controller::length() const
{
	return sizeof(struct xmp_ie_controller_t);
}

void
cxmpie_controller::pack(uint8_t *buf, size_t buflen)
{
	if (buflen < length()) throw eXmpIeInval();
	cxmpie::pack(buf, length());
}

void
cxmpie_controller::unpack(uint8_t* buf, size_t buflen)
{
	if (buflen < sizeof(struct xmp_ie_controller_t)) throw eXmpIeInval();
	cxmpie::unpack(buf, buflen);
	xmpie_generic = somem();
}

const rofl::caddress_in
cxmpie_controller::get_ip_address() const
{
	assert(xmpie_controller->ip_domain);

	if (AF_INET == xmpie_controller->ip_domain) {
		struct sockaddr_in sin;
		sin.sin_addr.s_addr = xmpie_controller->ip_address[0];
		return rofl::caddress_in4(&sin, sizeof(struct sockaddr_in));
	} else if (AF_INET6 == xmpie_controller->ip_domain) {
		struct sockaddr_in6 sin;
		memcpy(sin.sin6_addr.__in6_u.__u6_addr8, xmpie_controller->ip_address, 16);
		return rofl::caddress_in6(&sin, sizeof(struct sockaddr_in));
	} else {
		assert(0);
		return rofl::caddress_in4("0.0.0.0");
	}
}

void
cxmpie_controller::set_ip_address(const uint8_t ip_domain, const rofl::caddress& address)
{
	if (AF_INET == ip_domain) {
		const rofl::caddress_in4 &ipv4 = static_cast<const rofl::caddress_in4 &>(address);
		std::cerr << "v4" << std::endl;
		xmpie_controller->ip_address[0] = ipv4.get_addr_nbo();
		return;
	} else if (AF_INET6 == ip_domain) {
		const rofl::caddress_in6 &ipv6 = static_cast<const rofl::caddress_in6 &>(address);
		std::cerr << "v6" << std::endl;
		ipv6.pack((unsigned char*) xmpie_controller->ip_address, 16);

		return;
	} else {
		assert(0);
	}

}
