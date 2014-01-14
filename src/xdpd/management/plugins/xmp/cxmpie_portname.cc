/*
 * cxmpie_portname.cc
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#include "cxmpie_portname.h"


using namespace xdpd::mgmt::protocol;

cxmpie_portname::cxmpie_portname(
		std::string const& portname) :
				cxmpie(XMPIET_PORTNAME, sizeof(struct xmp_ie_portname_t))
{
	xmpie_generic = somem();
	set_portname(portname);
}


cxmpie_portname::cxmpie_portname(
		uint8_t *buf, size_t buflen) :
				cxmpie(buf, buflen)
{
	xmpie_generic = somem();
	if (buflen < sizeof(struct xmp_ie_portname_t))
		throw eXmpIeInval();
	unpack(buf, buflen);
}


cxmpie_portname::cxmpie_portname(
		cxmpie_portname const& elem) :
				cxmpie(elem)
{
	*this = elem;
}


cxmpie_portname&
cxmpie_portname::operator= (
		cxmpie_portname const& elem)
{
	if (this == &elem)
		return *this;

	cxmpie::operator= (elem);

	xmpie_generic = somem();

	return *this;
}


cxmpie_portname::cxmpie_portname(
		cxmpie const& elem) :
				cxmpie(elem)
{
	*this = elem;
}


cxmpie_portname&
cxmpie_portname::operator= (
		cxmpie const& elem)
{
	if (this == &elem)
		return *this;

	if (XMPIET_PORTNAME != elem.get_type())
		throw eXmpIeInval();
	if (elem.length() < sizeof(struct xmp_ie_portname_t))
		throw eXmpIeInval();

	cxmpie::operator= (elem);

	xmpie_generic = somem();

	return *this;
}


cxmpie_portname::~cxmpie_portname()
{

}


uint8_t*
cxmpie_portname::resize(
		size_t len)
{
	return (xmpie_generic = cxmpie::resize(len));
}


size_t
cxmpie_portname::length() const
{
	return sizeof(struct xmp_ie_portname_t);
}


void
cxmpie_portname::pack(
		uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpIeInval();
	cxmpie::pack(buf, length());
}


void
cxmpie_portname::unpack(
		uint8_t* buf, size_t buflen)
{
	if (buflen < sizeof(struct xmp_ie_portname_t))
		throw eXmpIeInval();
	cxmpie::unpack(buf, buflen);
	xmpie_generic = somem();
}


std::string
cxmpie_portname::get_portname() const
{
	return std::string(xmpie_portname->portname, strnlen(xmpie_portname->portname, XMPIE_PORTNAME_SIZE));
}


void
cxmpie_portname::set_portname(
		std::string const& portname)
{
	size_t len = (portname.length() < XMPIE_PORTNAME_SIZE) ? portname.length() : XMPIE_PORTNAME_SIZE;
	memset(xmpie_portname->portname, 0, XMPIE_PORTNAME_SIZE);
	memcpy(xmpie_portname->portname, portname.c_str(), len);
}


