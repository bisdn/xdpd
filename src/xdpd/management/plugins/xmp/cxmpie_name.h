/*
 * cxmpie_name.h
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#ifndef CXMPIE_NAME_H_
#define CXMPIE_NAME_H_

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

class cxmpie_name :
		public cxmpie
{
	union {
		uint8_t						*xmpu_generic;
		struct xmp_ie_name_t		*xmpu_name;
	} xmpie_xmpu;

#define xmpie_generic	xmpie_xmpu.xmpu_generic
#define xmpie_name		xmpie_xmpu.xmpu_name

public:

	/**
	 *
	 */
	cxmpie_name(enum xmpie_type_t type,	std::string const& portname = std::string(""));

	/**
	 *
	 */
	cxmpie_name(
			uint8_t *buf, size_t buflen);

	/**
	 *
	 */
	cxmpie_name(
			cxmpie_name const& elem);

	/**
	 *
	 */
	cxmpie_name&
	operator= (
			cxmpie_name const& elem);

	/**
	 *
	 */
	cxmpie_name(
			cxmpie const& elem);

	/**
	 *
	 */
	cxmpie_name&
	operator= (
			cxmpie const& elem);

	/**
	 *
	 */
	virtual
	~cxmpie_name();

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
	get_name() const;

	/**
	 *
	 */
	void
	set_name(std::string const& name);

public:

	friend std::ostream&
	operator<< (std::ostream& os, cxmpie_name const& elem) {
		os << dynamic_cast<cxmpie const&>( elem );
		os << rofl::indent(2) << "<cxmpie-portname ";
			os << "portname:" << elem.get_name() << " ";
		os << ">" << std::endl;
		return os;
	};
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd

#endif /* CXMPIE_NAME_H_ */
