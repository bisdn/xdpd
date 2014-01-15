/*
 * cxmpie.cc
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#include "cxmpie.h"

using namespace xdpd::mgmt::protocol;

cxmpie::cxmpie(
		uint16_t type, uint16_t len) :
				cmemory(len)
{
	xmpie_generic = somem();
	if (len < sizeof(struct xmp_ie_header_t))
		throw eXmpIeInval();
	set_type(type);
	set_length(len);
}


cxmpie::cxmpie(
		uint8_t *buf, size_t buflen) :
				cmemory(buf, buflen)
{
	xmpie_generic = somem();
	if (buflen < sizeof(struct xmp_ie_header_t))
		throw eXmpIeInval();
	unpack(buf, buflen);
}


cxmpie::cxmpie(
		cxmpie const& elem)
{
	*this = elem;
}


cxmpie&
cxmpie::operator= (
		cxmpie const& elem)
{
	if (this == &elem)
		return *this;

	cmemory::operator= (elem);

	xmpie_generic = somem();

	return *this;
}


cxmpie::~cxmpie()
{

}


uint8_t*
cxmpie::resize(
		size_t len)
{
	return (xmpie_generic = cmemory::resize(len));
}


size_t
cxmpie::length() const
{
	return cmemory::memlen();
}


void
cxmpie::pack(
		uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpIeInval();
	set_length(length());
	cmemory::pack(buf, length());
}


void
cxmpie::unpack(
		uint8_t *buf, size_t buflen)
{
	if (buflen < sizeof(struct xmp_ie_header_t))
		throw eXmpIeInval();
	struct xmp_ie_header_t *hdr = (struct xmp_ie_header_t*)buf;
	size_t len = (buflen < be16toh(hdr->len)) ? buflen : be16toh(hdr->len);
	cmemory::unpack(buf, len);
	xmpie_generic = somem();
}


