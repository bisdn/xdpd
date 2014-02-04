/*
 * cxmpies.cc
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#include "cxmpies.h"

using namespace xdpd::mgmt::protocol;

cxmpies::cxmpies()
{

}


cxmpies::cxmpies(
		uint8_t* buf, size_t buflen)
{
	unpack(buf, buflen);
}


cxmpies::cxmpies(
		cxmpies const& xmpies)
{
	*this = xmpies;
}


cxmpies&
cxmpies::operator= (
		cxmpies const& xmpies)
{
	if (this == &xmpies)
		return *this;

	clear();

	for (cxmpies::const_iterator it = xmpies.begin(); it != xmpies.end(); ++it) {
		map_and_insert(*(it->second));
	}

	return *this;
}


cxmpies::~cxmpies()
{
	clear();
}


void
cxmpies::clear()
{
	for (cxmpies::iterator it = xmpmap.begin(); it != xmpmap.end(); ++it) {
		delete it->second;
	}
	xmpmap.clear();
}


size_t
cxmpies::length() const
{
	size_t len = 0;
	for (cxmpies::const_iterator it = begin(); it != end(); ++it) {
		len += it->second->length();
	}
	return len;
}


void
cxmpies::pack(
		uint8_t* buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpIEsInval();
	for (cxmpies::iterator it = begin(); it != end(); ++it) {
		cxmpie& xmpie = *(it->second);
		xmpie.pack(buf, xmpie.length());
		buf += xmpie.length();
		buflen -= xmpie.length();
	}
}


void
cxmpies::unpack(
		uint8_t* buf, size_t buflen)
{
	clear();

	while (buflen > sizeof(struct xmp_ie_header_t)) {
		struct xmp_ie_header_t *hdr = (struct xmp_ie_header_t*)buf;
		if ((be16toh(hdr->len) > buflen) || (be16toh(hdr->len) < sizeof(struct xmp_ie_header_t))) {
			rofl::logging::error << "[xdpd][xmp] unpacking IE list, invalid length field in IE" << std::endl;
			return;
		}
		map_and_insert(cxmpie(buf, be16toh(hdr->len)));
		buf += be16toh(hdr->len);
		buflen -= be16toh(hdr->len);
	}
}


void
cxmpies::map_and_insert(
		cxmpie const& xmpie)
{
	if (xmpmap.find(xmpie.get_type()) != xmpmap.end()) {
		delete xmpmap[xmpie.get_type()];
		xmpmap.erase(xmpie.get_type());
	}

	switch (xmpie.get_type()) {
	case XMPIET_NONE: {
		return;
	} break;
	case XMPIET_COMMAND: {
		xmpmap[XMPIET_COMMAND] = new cxmpie_command(xmpie);
	} break;
	case XMPIET_PORTNAME: {
		xmpmap[XMPIET_PORTNAME] = new cxmpie_portname(xmpie);
	} break;
	case XMPIET_DPID: {
		xmpmap[XMPIET_DPID] = new cxmpie_dpid(xmpie);
	} break;
	default: {
		xmpmap[xmpie.get_type()] = new cxmpie(xmpie);
	};
	}
}


cxmpie_command&
cxmpies::add_ie_command()
{
	if (xmpmap.find(XMPIET_COMMAND) != xmpmap.end()) {
		delete xmpmap[XMPIET_COMMAND];
	}
	xmpmap[XMPIET_COMMAND] = new cxmpie_command();
	return *(dynamic_cast<cxmpie_command*>( xmpmap[XMPIET_COMMAND] ));
}


cxmpie_command&
cxmpies::set_ie_command()
{
	if (xmpmap.find(XMPIET_COMMAND) == xmpmap.end()) {
		xmpmap[XMPIET_COMMAND] = new cxmpie_command();
	}
	return *(dynamic_cast<cxmpie_command*>( xmpmap[XMPIET_COMMAND] ));
}


cxmpie_command const&
cxmpies::get_ie_command() const
{
	if (xmpmap.find(XMPIET_COMMAND) == xmpmap.end()) {
		throw eXmpIEsNotFound();
	}
	return *(dynamic_cast<cxmpie_command const*>( xmpmap.at(XMPIET_COMMAND) ));
}


void
cxmpies::drop_ie_command()
{
	if (xmpmap.find(XMPIET_COMMAND) == xmpmap.end()) {
		return;
	}
	delete xmpmap[XMPIET_COMMAND];
	xmpmap.erase(XMPIET_COMMAND);
}


bool
cxmpies::has_ie_command() const
{
	return (xmpmap.find(XMPIET_COMMAND) != xmpmap.end());
}


cxmpie_portname&
cxmpies::add_ie_portname()
{
	if (xmpmap.find(XMPIET_PORTNAME) != xmpmap.end()) {
		delete xmpmap[XMPIET_PORTNAME];
	}
	xmpmap[XMPIET_PORTNAME] = new cxmpie_portname();
	return *(dynamic_cast<cxmpie_portname*>( xmpmap[XMPIET_PORTNAME] ));
}


cxmpie_portname&
cxmpies::set_ie_portname()
{
	if (xmpmap.find(XMPIET_PORTNAME) == xmpmap.end()) {
		xmpmap[XMPIET_PORTNAME] = new cxmpie_portname();
	}
	return *(dynamic_cast<cxmpie_portname*>( xmpmap[XMPIET_PORTNAME] ));
}


cxmpie_portname const&
cxmpies::get_ie_portname() const
{
	if (xmpmap.find(XMPIET_PORTNAME) == xmpmap.end()) {
		throw eXmpIEsNotFound();
	}
	return *(dynamic_cast<cxmpie_portname const*>( xmpmap.at(XMPIET_PORTNAME) ));
}


void
cxmpies::drop_ie_portname()
{
	if (xmpmap.find(XMPIET_PORTNAME) == xmpmap.end()) {
		return;
	}
	delete xmpmap[XMPIET_PORTNAME];
	xmpmap.erase(XMPIET_PORTNAME);
}


bool
cxmpies::has_ie_portname() const
{
	return (xmpmap.find(XMPIET_PORTNAME) != xmpmap.end());
}


cxmpie_dpid&
cxmpies::add_ie_dpid()
{
	if (xmpmap.find(XMPIET_DPID) != xmpmap.end()) {
		delete xmpmap[XMPIET_DPID];
	}
	xmpmap[XMPIET_DPID] = new cxmpie_dpid();
	return *(dynamic_cast<cxmpie_dpid*>( xmpmap[XMPIET_DPID] ));
}


cxmpie_dpid&
cxmpies::set_ie_dpid()
{
	if (xmpmap.find(XMPIET_DPID) == xmpmap.end()) {
		xmpmap[XMPIET_DPID] = new cxmpie_dpid();
	}
	return *(dynamic_cast<cxmpie_dpid*>( xmpmap[XMPIET_DPID] ));
}


cxmpie_dpid const&
cxmpies::get_ie_dpid() const
{
	if (xmpmap.find(XMPIET_DPID) == xmpmap.end()) {
		throw eXmpIEsNotFound();
	}
	return *(dynamic_cast<cxmpie_dpid const*>( xmpmap.at(XMPIET_DPID) ));
}


void
cxmpies::drop_ie_dpid()
{
	if (xmpmap.find(XMPIET_DPID) == xmpmap.end()) {
		return;
	}
	delete xmpmap[XMPIET_DPID];
	xmpmap.erase(XMPIET_DPID);
}


bool
cxmpies::has_ie_dpid() const
{
	return (xmpmap.find(XMPIET_DPID) != xmpmap.end());
}


