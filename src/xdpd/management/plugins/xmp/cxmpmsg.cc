/*
 * cxmpmsg.cc
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#include "cxmpmsg.h"

using namespace xdpd::mgmt::protocol;

cxmpmsg::cxmpmsg(
		uint8_t version, uint8_t type, uint16_t len, uint32_t xid) :
				cmemory(len)
{
	if (len < sizeof(struct xmp_header_t))
		throw eXmpInval();

	xmp_generic = somem();

	set_version(version);
	set_type(type);
	set_length(len);
	set_xid(xid);
}



cxmpmsg::cxmpmsg(
		uint8_t *buf, size_t buflen) :
				cmemory((size_t)buflen)
{
	unpack(buf, buflen);
}



cxmpmsg::cxmpmsg(
		cxmpmsg const& msg)
{
	*this = msg;
}



cxmpmsg&
cxmpmsg::operator= (
		cxmpmsg const& msg)
{
	if (this == &msg)
		return *this;

	cmemory::operator= (msg);
	xmpies = msg.xmpies;

	xmp_generic = somem();

	return *this;
}



cxmpmsg::~cxmpmsg()
{

}



uint8_t*
cxmpmsg::resize(
		size_t len)
{
	return (xmp_generic = cmemory::resize(len));
}



size_t
cxmpmsg::length() const
{
	return (sizeof(struct xmp_header_t) + xmpies.length());
}



void
cxmpmsg::pack(uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpInval();

	set_length(length());

	memcpy(buf, cmemory::somem(), sizeof(struct xmp_header_t));

	xmpies.pack(buf + sizeof(struct xmp_header_t), xmpies.length());
}



void
cxmpmsg::unpack(uint8_t *buf, size_t buflen)
{
	xmpies.clear();
	if (buflen < sizeof(struct xmp_header_t))
		throw eXmpInval();
	assign(buf, buflen);
	xmp_generic = somem();
	if (buflen > sizeof(struct xmp_header_t)) {
		xmpies.unpack(buf + sizeof(struct xmp_header_t), buflen - sizeof(struct xmp_header_t));
	}
}



