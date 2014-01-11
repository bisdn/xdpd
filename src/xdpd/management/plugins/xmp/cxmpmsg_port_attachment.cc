/*
 * cxmpmsg_port_attachment.cc
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#include "cxmpmsg_port_attachment.h"

using namespace xdpd::mgmt::protocol;

cxmpmsg_port_attachment::cxmpmsg_port_attachment(
		uint8_t msg_type, std::string const& portname, uint64_t dpid) :
				cxmpmsg(XMP_VERSION, msg_type, sizeof(struct xmp_msg_port_attachment_t), 0)
{
	xmp_generic = somem();
	set_portname(portname);
	set_dpid(dpid);
}


cxmpmsg_port_attachment::cxmpmsg_port_attachment(
		uint8_t *buf, size_t buflen) :
				cxmpmsg(buf, buflen)
{
	xmp_generic = somem();
}


cxmpmsg_port_attachment::cxmpmsg_port_attachment(
		cxmpmsg_port_attachment const& msg) :
				cxmpmsg(msg)
{
	*this = msg;
}


cxmpmsg_port_attachment&
cxmpmsg_port_attachment::operator= (
		cxmpmsg_port_attachment const& msg)
{
	if (this == &msg)
		return *this;

	cxmpmsg::operator= (msg);

	xmp_generic = somem();
	set_portname(msg.get_portname());
	set_dpid(msg.get_dpid());

	return *this;
}


cxmpmsg_port_attachment::~cxmpmsg_port_attachment()
{

}


uint8_t*
cxmpmsg_port_attachment::resize(size_t len)
{
	return (xmp_generic = cmemory::resize(len));
}


size_t
cxmpmsg_port_attachment::length() const
{
	return sizeof(struct xmp_msg_port_attachment_t);
}


void
cxmpmsg_port_attachment::pack(uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpInval();

	size_t len = (buflen < length()) ? buflen : length();

	memcpy(buf, somem(), len);
}


void
cxmpmsg_port_attachment::unpack(uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpInval();
	assign(buf, buflen);
	xmp_generic = somem();
}


std::string
cxmpmsg_port_attachment::get_portname() const
{
	return std::string(xmp_port_attachment->portname, 32);
}


void
cxmpmsg_port_attachment::set_portname(std::string const& portname)
{
	size_t len = (PORTNAMESIZE < portname.length()) ? PORTNAMESIZE : portname.length();
	memcpy(xmp_port_attachment->portname, portname.c_str(), len);
}


uint64_t
cxmpmsg_port_attachment::get_dpid() const
{
	return be64toh(xmp_port_attachment->dpid);
}


void
cxmpmsg_port_attachment::set_dpid(uint64_t dpid)
{
	xmp_port_attachment->dpid = htobe64(dpid);
}



