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
		xmpmap[XMPIET_PORTNAME] = new cxmpie_name(xmpie);
	} break;
	case XMPIET_DPID: {
		xmpmap[XMPIET_DPID] = new cxmpie_dpid(xmpie);
	} break;
	case XMPIET_PORTINFO: {
		xmpmap[XMPIET_PORTINFO] = new cxmpie_portinfo(xmpie);
	} break;
	case XMPIET_MULTIPART: {
		xmpmap[XMPIET_MULTIPART] = new cxmpie_multipart(xmpie);
	} break;
	case XMPIET_LSINAME: {
		xmpmap[XMPIET_LSINAME] = new cxmpie_name(xmpie);
	} break;
	case XMPIET_LSIINFO: {
		xmpmap[XMPIET_LSIINFO] = new cxmpie_lsiinfo(xmpie);
	} break;
	case XMPIET_CONTROLLER: {
		xmpmap[XMPIET_CONTROLLER] = new cxmpie_controller(xmpie);
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


cxmpie_name&
cxmpies::add_ie_portname()
{
	if (xmpmap.find(XMPIET_PORTNAME) != xmpmap.end()) {
		delete xmpmap[XMPIET_PORTNAME];
	}
	xmpmap[XMPIET_PORTNAME] = new cxmpie_name(XMPIET_PORTNAME);
	return *(dynamic_cast<cxmpie_name*>( xmpmap[XMPIET_PORTNAME] ));
}


cxmpie_name&
cxmpies::set_ie_portname()
{
	if (xmpmap.find(XMPIET_PORTNAME) == xmpmap.end()) {
		xmpmap[XMPIET_PORTNAME] = new cxmpie_name(XMPIET_PORTNAME);
	}
	return *(dynamic_cast<cxmpie_name*>( xmpmap[XMPIET_PORTNAME] ));
}


cxmpie_name const&
cxmpies::get_ie_portname() const
{
	if (xmpmap.find(XMPIET_PORTNAME) == xmpmap.end()) {
		throw eXmpIEsNotFound();
	}
	return *(dynamic_cast<cxmpie_name const*>( xmpmap.at(XMPIET_PORTNAME) ));
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

cxmpie_portinfo&
cxmpies::add_ie_portinfo()
{
	if (xmpmap.find(XMPIET_PORTINFO) != xmpmap.end()) {
		delete xmpmap[XMPIET_PORTINFO];
	}
	xmpmap[XMPIET_PORTINFO] = new cxmpie_portinfo();
	return *(dynamic_cast<cxmpie_portinfo*>(xmpmap[XMPIET_PORTINFO]));
}

cxmpie_portinfo&
cxmpies::set_ie_portinfo()
{
	if (xmpmap.find(XMPIET_PORTINFO) == xmpmap.end()) {
		xmpmap[XMPIET_PORTINFO] = new cxmpie_portinfo();
	}
	return *(dynamic_cast<cxmpie_portinfo*>(xmpmap[XMPIET_PORTINFO]));
}

cxmpie_portinfo const&
cxmpies::get_ie_portinfo() const
{
	if (xmpmap.find(XMPIET_PORTINFO) == xmpmap.end()) {
		throw eXmpIEsNotFound();
	}
	return *(dynamic_cast<cxmpie_portinfo const*>(xmpmap.at(XMPIET_PORTINFO)));
}

void
cxmpies::drop_ie_portinfo()
{
	if (xmpmap.find(XMPIET_PORTINFO) == xmpmap.end()) {
		return;
	}
	delete xmpmap[XMPIET_PORTINFO];
	xmpmap.erase(XMPIET_PORTINFO);
}

bool
cxmpies::has_ie_portinfo() const
{
	return (xmpmap.find(XMPIET_PORTINFO) != xmpmap.end());
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


cxmpie_name&
cxmpies::add_ie_lsiname()
{
	if (xmpmap.find(XMPIET_LSINAME) != xmpmap.end()) {
		delete xmpmap[XMPIET_LSINAME];
	}
	xmpmap[XMPIET_LSINAME] = new cxmpie_name(XMPIET_LSINAME);
	return *(dynamic_cast<cxmpie_name*>( xmpmap[XMPIET_LSINAME] ));
}

cxmpie_name&
cxmpies::set_ie_lsiname()
{
	if (xmpmap.find(XMPIET_LSINAME) == xmpmap.end()) {
		xmpmap[XMPIET_LSINAME] = new cxmpie_name(XMPIET_LSINAME);
	}
	return *(dynamic_cast<cxmpie_name*>( xmpmap[XMPIET_LSINAME] ));
}

cxmpie_name const&
cxmpies::get_ie_lsiname() const
{
	if (xmpmap.find(XMPIET_LSINAME) == xmpmap.end()) {
		throw eXmpIEsNotFound();
	}
	return *(dynamic_cast<cxmpie_name const*>( xmpmap.at(XMPIET_LSINAME) ));
}

void
cxmpies::drop_ie_lsiname()
{
	if (xmpmap.find(XMPIET_LSINAME) == xmpmap.end()) {
		return;
	}
	delete xmpmap[XMPIET_LSINAME];
	xmpmap.erase(XMPIET_LSINAME);
}

bool
cxmpies::has_ie_lsiname() const
{
	return (xmpmap.find(XMPIET_LSINAME) != xmpmap.end());
}


cxmpie_lsiinfo&
cxmpies::add_ie_lsiinfo()
{
	if (xmpmap.find(XMPIET_LSIINFO) != xmpmap.end()) {
		delete xmpmap[XMPIET_LSIINFO];
	}
	xmpmap[XMPIET_LSIINFO] = new cxmpie_lsiinfo();
	return *(dynamic_cast<cxmpie_lsiinfo*>( xmpmap[XMPIET_LSIINFO] ));
}

cxmpie_lsiinfo&
cxmpies::set_ie_lsiinfo()
{
	if (xmpmap.find(XMPIET_LSIINFO) == xmpmap.end()) {
		xmpmap[XMPIET_LSIINFO] = new cxmpie_lsiinfo();
	}
	return *(dynamic_cast<cxmpie_lsiinfo*>( xmpmap[XMPIET_LSIINFO] ));
}

cxmpie_lsiinfo const&
cxmpies::get_ie_lsiinfo() const
{
	if (xmpmap.find(XMPIET_LSIINFO) == xmpmap.end()) {
		throw eXmpIEsNotFound();
	}
	return *(dynamic_cast<cxmpie_lsiinfo const*>( xmpmap.at(XMPIET_LSIINFO) ));
}

void
cxmpies::drop_ie_lsiinfo()
{
	if (xmpmap.find(XMPIET_LSIINFO) == xmpmap.end()) {
		return;
	}
	delete xmpmap[XMPIET_LSIINFO];
	xmpmap.erase(XMPIET_LSIINFO);
}

bool
cxmpies::has_ie_lsiinfo() const
{
	return (xmpmap.find(XMPIET_LSIINFO) != xmpmap.end());
}


cxmpie_controller&
cxmpies::add_ie_controller()
{
	if (xmpmap.find(XMPIET_CONTROLLER) != xmpmap.end()) {
		delete xmpmap[XMPIET_CONTROLLER];
	}
	xmpmap[XMPIET_CONTROLLER] = new cxmpie_controller();
	return *(dynamic_cast<cxmpie_controller*>( xmpmap[XMPIET_CONTROLLER] ));
}

cxmpie_controller&
cxmpies::set_ie_controller()
{
	if (xmpmap.find(XMPIET_CONTROLLER) == xmpmap.end()) {
		xmpmap[XMPIET_CONTROLLER] = new cxmpie_controller();
	}
	return *(dynamic_cast<cxmpie_controller*>( xmpmap[XMPIET_CONTROLLER] ));
}

cxmpie_controller const&
cxmpies::get_ie_controller() const
{
	if (xmpmap.find(XMPIET_CONTROLLER) == xmpmap.end()) {
		throw eXmpIEsNotFound();
	}
	return *(dynamic_cast<cxmpie_controller const*>( xmpmap.at(XMPIET_CONTROLLER) ));
}

void
cxmpies::drop_ie_controller()
{
	if (xmpmap.find(XMPIET_CONTROLLER) == xmpmap.end()) {
		return;
	}
	delete xmpmap[XMPIET_CONTROLLER];
	xmpmap.erase(XMPIET_CONTROLLER);
}

bool
cxmpies::has_ie_controller() const
{
	return (xmpmap.find(XMPIET_CONTROLLER) != xmpmap.end());
}


cxmpie_multipart&
cxmpies::add_ie_multipart()
{
	if (xmpmap.find(XMPIET_MULTIPART) != xmpmap.end()) {
		delete xmpmap[XMPIET_MULTIPART];
	}
	xmpmap[XMPIET_MULTIPART] = new cxmpie_multipart();
	return *(dynamic_cast<cxmpie_multipart*>( xmpmap[XMPIET_MULTIPART] ));
}

cxmpie_multipart&
cxmpies::set_ie_multipart()
{
	if (xmpmap.find(XMPIET_MULTIPART) == xmpmap.end()) {
		xmpmap[XMPIET_MULTIPART] = new cxmpie_multipart();
	}
	return *(dynamic_cast<cxmpie_multipart*>( xmpmap[XMPIET_MULTIPART] ));
}

cxmpie_multipart const&
cxmpies::get_ie_multipart() const
{
	if (xmpmap.find(XMPIET_MULTIPART) == xmpmap.end()) {
		throw eXmpIEsNotFound();
	}
	return *(dynamic_cast<cxmpie_multipart const*>( xmpmap.at(XMPIET_MULTIPART) ));
}

void
cxmpies::drop_ie_multipart()
{
	if (xmpmap.find(XMPIET_MULTIPART) == xmpmap.end()) {
		return;
	}
	delete xmpmap[XMPIET_MULTIPART];
	xmpmap.erase(XMPIET_MULTIPART);
}

bool
cxmpies::has_ie_multipart() const
{
	return (xmpmap.find(XMPIET_MULTIPART) != xmpmap.end());
}
