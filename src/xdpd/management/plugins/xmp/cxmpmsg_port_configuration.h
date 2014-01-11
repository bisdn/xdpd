/*
 * cxmpmsg_port_configuration.h
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#ifndef CXMPMSG_PORT_CONFIGURATION_H_
#define CXMPMSG_PORT_CONFIGURATION_H_

#include <string>

#include "cxmpmsg.h"
#include "xdpd_mgmt_protocol.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class cxmpmsg_port_configuration :
		public cxmpmsg
{
	union {
		uint8_t								*xmpu_generic;
		struct xmp_msg_port_configuration_t	*xmpu_port_configuration;
	} xmp_xmpu;

#define xmp_generic				xmp_xmpu.xmpu_generic
#define xmp_port_configuration	xmp_xmpu.xmpu_port_configuration

public:

	cxmpmsg_port_configuration(
			uint8_t msg_type, std::string const& portname, uint32_t config);

	cxmpmsg_port_configuration(
			uint8_t *buf, size_t buflen);

	cxmpmsg_port_configuration(
			cxmpmsg_port_configuration const& msg);

	cxmpmsg_port_configuration&
	operator= (cxmpmsg_port_configuration const& msg);

	virtual
	~cxmpmsg_port_configuration();

	virtual uint8_t*
	resize(size_t len);

	virtual size_t
	length() const;

	void
	pack(uint8_t *buf, size_t buflen);

	void
	unpack(uint8_t *buf, size_t buflen);

public:

	std::string
	get_portname() const;

	void
	set_portname(std::string const& portname);

	uint32_t
	get_config() const;

	void
	set_config(uint32_t config);
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd

#endif /* CXMPMSG_PORT_CONFIGURATION_H_ */
