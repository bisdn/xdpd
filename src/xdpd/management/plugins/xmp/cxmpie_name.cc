/*
 * cxmpie_name.cc
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#include "cxmpie_name.h"


using namespace xdpd::mgmt::protocol;

cxmpie_name::cxmpie_name(enum xmpie_type_t type,
		std::string const& name) :
				cxmpie(type, sizeof(struct xmp_ie_name_t))
{
	assert(XMPIET_LSINAME == type || XMPIET_PORTNAME == type);
	xmpie_generic = somem();
	set_name(name);
}


cxmpie_name::cxmpie_name(
		uint8_t *buf, size_t buflen) :
				cxmpie(buf, buflen)
{
	xmpie_generic = somem();
	if (buflen < sizeof(struct xmp_ie_name_t))
		throw eXmpIeInval();
	unpack(buf, buflen);
}


cxmpie_name::cxmpie_name(
		cxmpie_name const& elem) :
				cxmpie(elem)
{
	*this = elem;
}


cxmpie_name&
cxmpie_name::operator= (
		cxmpie_name const& elem)
{
	if (this == &elem)
		return *this;

	cxmpie::operator= (elem);

	xmpie_generic = somem();

	return *this;
}


cxmpie_name::cxmpie_name(
		cxmpie const& elem) :
				cxmpie(elem)
{
	*this = elem;
}


cxmpie_name&
cxmpie_name::operator= (
		cxmpie const& elem)
{
	if (this == &elem)
		return *this;

	if (not (XMPIET_PORTNAME == elem.get_type() || XMPIET_LSINAME == elem.get_type()))
		throw eXmpIeInval();
	if (elem.length() < sizeof(struct xmp_ie_name_t))
		throw eXmpIeInval();

	cxmpie::operator= (elem);

	xmpie_generic = somem();

	return *this;
}


cxmpie_name::~cxmpie_name()
{

}


uint8_t*
cxmpie_name::resize(
		size_t len)
{
	return (xmpie_generic = cxmpie::resize(len));
}


size_t
cxmpie_name::length() const
{
	return sizeof(struct xmp_ie_name_t);
}


void
cxmpie_name::pack(
		uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpIeInval();
	cxmpie::pack(buf, length());
}


void
cxmpie_name::unpack(
		uint8_t* buf, size_t buflen)
{
	if (buflen < sizeof(struct xmp_ie_name_t))
		throw eXmpIeInval();
	cxmpie::unpack(buf, buflen);
	xmpie_generic = somem();
}


std::string
cxmpie_name::get_name() const
{
	return std::string(xmpie_name->name, strnlen(xmpie_name->name, XMPIE_NAME_SIZE));
}


void
cxmpie_name::set_name(
		std::string const& name)
{
	size_t len = (name.length() < XMPIE_NAME_SIZE) ? name.length() : XMPIE_NAME_SIZE;
	memset(xmpie_name->name, 0, XMPIE_NAME_SIZE);
	memcpy(xmpie_name->name, name.c_str(), len);
}


