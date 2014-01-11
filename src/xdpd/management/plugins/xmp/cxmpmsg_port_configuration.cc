/* cxmpmsg_port_configuration.cc
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#include "cxmpmsg_port_configuration.h"

using namespace xdpd::mgmt::protocol;

cxmpmsg_port_configuration::cxmpmsg_port_configuration(
		uint8_t msg_type, std::string const& portname, uint32_t config) :
				cxmpmsg(XMP_VERSION, msg_type, sizeof(struct xmp_msg_port_configuration_t), 0)
{
	xmp_generic = somem();
	set_portname(portname);
	set_config(config);
}


cxmpmsg_port_configuration::cxmpmsg_port_configuration(
		uint8_t *buf, size_t buflen) :
				cxmpmsg(buf, buflen)
{
	xmp_generic = somem();
}


cxmpmsg_port_configuration::cxmpmsg_port_configuration(
		cxmpmsg_port_configuration const& msg) :
				cxmpmsg(msg)
{
	*this = msg;
}


cxmpmsg_port_configuration&
cxmpmsg_port_configuration::operator= (
		cxmpmsg_port_configuration const& msg)
{
	if (this == &msg)
		return *this;

	cxmpmsg::operator= (msg);

	xmp_generic = somem();
	set_portname(msg.get_portname());
	set_config(msg.get_config());

	return *this;
}


cxmpmsg_port_configuration::~cxmpmsg_port_configuration()
{

}


uint8_t*
cxmpmsg_port_configuration::resize(size_t len)
{
	return (xmp_generic = cmemory::resize(len));
}


size_t
cxmpmsg_port_configuration::length() const
{
	return sizeof(struct xmp_msg_port_configuration_t);
}


void
cxmpmsg_port_configuration::pack(uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpInval();

	size_t len = (buflen < length()) ? buflen : length();

	memcpy(buf, somem(), len);
}


void
cxmpmsg_port_configuration::unpack(uint8_t *buf, size_t buflen)
{
	if (buflen < length())
		throw eXmpInval();
	assign(buf, buflen);
	xmp_generic = somem();
}


std::string
cxmpmsg_port_configuration::get_portname() const
{
	return std::string(xmp_port_configuration->portname, 32);
}


void
cxmpmsg_port_configuration::set_portname(std::string const& portname)
{
	size_t len = (PORTNAMESIZE < portname.length()) ? PORTNAMESIZE : portname.length();
	memcpy(xmp_port_configuration->portname, portname.c_str(), len);
}


uint32_t
cxmpmsg_port_configuration::get_config() const
{
	return be32toh(xmp_port_configuration->config);
}


void
cxmpmsg_port_configuration::set_config(uint32_t config)
{
	xmp_port_configuration->config = htobe32(config);
}



