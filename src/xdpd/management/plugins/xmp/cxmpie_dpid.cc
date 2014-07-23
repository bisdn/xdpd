/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * cxmpie_dpid.cc
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#include "cxmpie_dpid.h"

using namespace xdpd::mgmt::protocol;

cxmpie_dpid::cxmpie_dpid(
		uint64_t dpid) :
				cxmpie(XMPIET_DPID, sizeof(struct xmp_ie_dpid_t))
{
	xmpie_generic = somem();
	set_dpid(dpid);
}


cxmpie_dpid::cxmpie_dpid(
		uint8_t *buf, size_t buflen) :
				cxmpie(buf, buflen)
{
	xmpie_generic = somem();
	if (buflen < sizeof(struct xmp_ie_dpid_t))
		throw eXmpIeInval();
	unpack(buf, buflen);
}


cxmpie_dpid::cxmpie_dpid(
		cxmpie_dpid const& elem) :
				cxmpie(elem)
{
	*this = elem;
}


cxmpie_dpid&
cxmpie_dpid::operator= (
		cxmpie_dpid const& elem)
{
	if (this == &elem)
		return *this;

	cxmpie::operator= (elem);

	xmpie_generic = somem();

	return *this;
}


cxmpie_dpid::cxmpie_dpid(
		cxmpie const& elem) :
				cxmpie(elem)
{
	*this = elem;
}


cxmpie_dpid&
cxmpie_dpid::operator= (
		cxmpie const& elem)
{
	if (this == &elem)
		return *this;

	if (XMPIET_DPID != elem.get_type())
		throw eXmpIeInval();
	if (elem.length() < sizeof(struct xmp_ie_dpid_t))
		throw eXmpIeInval();

	cxmpie::operator= (elem);

	xmpie_generic = somem();

	return *this;
}


cxmpie_dpid::~cxmpie_dpid()
{

}


uint8_t*
cxmpie_dpid::resize(
		size_t len)
{
	return (xmpie_generic = cxmpie::resize(len));
}


size_t
cxmpie_dpid::length() const
{
	return sizeof(struct xmp_ie_dpid_t);
}


void
cxmpie_dpid::pack(
		uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpIeInval();
	cxmpie::pack(buf, length());
}


void
cxmpie_dpid::unpack(
		uint8_t* buf, size_t buflen)
{
	if (buflen < sizeof(struct xmp_ie_dpid_t))
		throw eXmpIeInval();
	cxmpie::unpack(buf, buflen);
	xmpie_generic = somem();
}





