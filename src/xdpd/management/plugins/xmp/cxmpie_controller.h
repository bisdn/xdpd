/*
 * cxmpie_controller.h
 *
 *  Created on: Sep 26, 2014
 *      Author: tobi
 */

#ifndef CXMPIE_CONTROLLER_H_
#define CXMPIE_CONTROLLER_H_

#include "inttypes.h"

#include "cxmpie.h"
#include "xdpd_mgmt_protocol.h"

#include "rofl/common/caddress.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class controller
{
public:
	std::string		id;	// in yang its a int:uri, hence length is unspecified; todo do we need this?
	std::string		proto;

	uint8_t			ip_domain;
	rofl::caddress 	address;
	uint16_t 		port;
};


class cxmpie_controller :
		public cxmpie
{
	union {
		uint8_t						*xmpu_generic;
		struct xmp_ie_controller_t	*xmpu_controller;
	} xmpie_xmpu;

#define xmpie_generic		xmpie_xmpu.xmpu_generic
#define xmpie_controller	xmpie_xmpu.xmpu_controller

public:

	cxmpie_controller();

	/**
	 *
	 */
	cxmpie_controller(const class controller &c);

	/**
	 *
	 */
	cxmpie_controller(
			uint8_t *buf, size_t buflen);

	/**
	 *
	 */
	cxmpie_controller(
			cxmpie_controller const& elem);

	/**
	 *
	 */
	cxmpie_controller&
	operator= (
			cxmpie_controller const& elem);

	/**
	 *
	 */
	cxmpie_controller(
			cxmpie const& elem);

	/**
	 *
	 */
	cxmpie_controller&
	operator= (
			cxmpie const& elem);

	/**
	 *
	 */
	virtual
	~cxmpie_controller();

public:

	/**
	 *
	 */
	virtual uint8_t*
	resize(
			size_t len);

	/**
	 *
	 */
	virtual size_t
	length() const;

	/**
	 *
	 */
	virtual void
	pack(
			uint8_t *buf, size_t buflen);

	/**
	 *
	 */
	virtual void
	unpack(
			uint8_t *buf, size_t buflen);

public:

	std::string
	get_id() const { return std::string(xmpie_controller->id); };

	void
	set_id(std::string const &id) { strncpy(xmpie_controller->id, id.c_str(), sizeof(xmpie_controller->id - 1)); };

	std::string
	get_proto() const { return std::string(xmpie_controller->proto); };

	void
	set_proto(std::string const &proto) { strncpy(xmpie_controller->proto, proto.c_str(), sizeof(xmpie_controller->proto - 1)); };

	uint8_t
	get_ip_domain() const { return xmpie_controller->ip_domain; };

	void
	set_ip_domain(const uint8_t ip_domain) { xmpie_controller->ip_domain = ip_domain; };

	const rofl::caddress_in
	get_ip_address() const;

	void
	set_ip_address(const uint8_t ip_domain, const rofl::caddress& address);

	uint16_t
	get_port() const { return be16toh(xmpie_controller->port); };

	void
	set_port(const uint16_t port) { xmpie_controller->port = htobe16(port); };

public:

	friend std::ostream&
	operator<< (std::ostream& os, cxmpie_controller const& elem) {
		os << dynamic_cast<cxmpie const&>( elem );
		os << rofl::indent(2) << "<cxmpie-controller ";
		os << ">" << std::endl;
		return os;
	};
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd


#endif /* CXMPIE_CONTROLLER_H_ */
