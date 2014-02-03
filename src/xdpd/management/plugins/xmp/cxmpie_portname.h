/*
 * cxmpie_portname.h
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#ifndef CXMPIE_PORTNAME_H_
#define CXMPIE_PORTNAME_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>
#ifdef __cplusplus
}
#endif

#include "cxmpie.h"
#include "xdpd_mgmt_protocol.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class cxmpie_portname :
		public cxmpie
{
	union {
		uint8_t						*xmpu_generic;
		struct xmp_ie_portname_t	*xmpu_portname;
	} xmpie_xmpu;

#define xmpie_generic	xmpie_xmpu.xmpu_generic
#define xmpie_portname	xmpie_xmpu.xmpu_portname

public:

	/**
	 *
	 */
	cxmpie_portname(
			std::string const& portname = std::string(""));

	/**
	 *
	 */
	cxmpie_portname(
			uint8_t *buf, size_t buflen);

	/**
	 *
	 */
	cxmpie_portname(
			cxmpie_portname const& elem);

	/**
	 *
	 */
	cxmpie_portname&
	operator= (
			cxmpie_portname const& elem);

	/**
	 *
	 */
	cxmpie_portname(
			cxmpie const& elem);

	/**
	 *
	 */
	cxmpie_portname&
	operator= (
			cxmpie const& elem);

	/**
	 *
	 */
	virtual
	~cxmpie_portname();

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

	/**
	 *
	 */
	std::string
	get_portname() const;

	/**
	 *
	 */
	void
	set_portname(std::string const& portname);

public:

	friend std::ostream&
	operator<< (std::ostream& os, cxmpie_portname const& elem) {
		os << dynamic_cast<cxmpie const&>( elem );
		os << rofl::indent(2) << "<cxmpie-portname ";
			os << "portname:" << elem.get_portname() << " ";
		os << ">" << std::endl;
		return os;
	};
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd

#endif /* CXMPIE_PORTNAME_H_ */
