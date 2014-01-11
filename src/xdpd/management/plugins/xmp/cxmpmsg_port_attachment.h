/*
 * cxmpmsg_port_attachment.h
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#ifndef CXMPMSG_PORT_ATTACHMENT_H_
#define CXMPMSG_PORT_ATTACHMENT_H_

#include <string>

#include "cxmpmsg.h"
#include "xdpd_mgmt_protocol.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class cxmpmsg_port_attachment :
		public cxmpmsg
{
	union {
		uint8_t								*xmpu_generic;
		struct xmp_msg_port_attachment_t	*xmpu_port_attachment;
	} xmp_xmpu;

#define xmp_generic			xmp_xmpu.xmpu_generic
#define xmp_port_attachment	xmp_xmpu.xmpu_port_attachment

public:

	cxmpmsg_port_attachment(
			uint8_t msg_type, std::string const& portname, uint64_t dpid);

	cxmpmsg_port_attachment(
			uint8_t *buf, size_t buflen);

	cxmpmsg_port_attachment(
			cxmpmsg_port_attachment const& msg);

	cxmpmsg_port_attachment&
	operator= (cxmpmsg_port_attachment const& msg);

	virtual
	~cxmpmsg_port_attachment();

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

	uint64_t
	get_dpid() const;

	void
	set_dpid(uint64_t dpid);
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd

#endif /* CXMPMSG_PORT_ATTACHMENT_H_ */
