/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * cxmpie_command.cc
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#include "cxmpie_command.h"

using namespace xdpd::mgmt::protocol;

cxmpie_command::cxmpie_command(
		uint32_t command) :
				cxmpie(XMPIET_COMMAND, sizeof(struct xmp_ie_command_t))
{
	xmpie_generic = somem();
	set_command(command);
}


cxmpie_command::cxmpie_command(
		uint8_t *buf, size_t buflen) :
				cxmpie(buf, buflen)
{
	xmpie_generic = somem();
	if (buflen < sizeof(struct xmp_ie_command_t))
		throw eXmpIeInval();
	unpack(buf, buflen);
}


cxmpie_command::cxmpie_command(
		cxmpie_command const& elem) :
				cxmpie(elem)
{
	*this = elem;
}


cxmpie_command&
cxmpie_command::operator= (
		cxmpie_command const& elem)
{
	if (this == &elem)
		return *this;

	cxmpie::operator= (elem);

	xmpie_generic = somem();

	return *this;
}


cxmpie_command::cxmpie_command(
		cxmpie const& elem) :
				cxmpie(elem)
{
	*this = elem;
}


cxmpie_command&
cxmpie_command::operator= (
		cxmpie const& elem)
{
	if (this == &elem)
		return *this;

	if (XMPIET_COMMAND != elem.get_type())
		throw eXmpIeInval();
	if (elem.length() < sizeof(struct xmp_ie_command_t))
		throw eXmpIeInval();

	cxmpie::operator= (elem);

	xmpie_generic = somem();

	return *this;
}


cxmpie_command::~cxmpie_command()
{

}


uint8_t*
cxmpie_command::resize(
		size_t len)
{
	return (xmpie_generic = cxmpie::resize(len));
}


size_t
cxmpie_command::length() const
{
	return sizeof(struct xmp_ie_command_t);
}


void
cxmpie_command::pack(
		uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpIeInval();
	cxmpie::pack(buf, length());
}


void
cxmpie_command::unpack(
		uint8_t* buf, size_t buflen)
{
	if (buflen < sizeof(struct xmp_ie_command_t))
		throw eXmpIeInval();
	cxmpie::unpack(buf, buflen);
	xmpie_generic = somem();
}


