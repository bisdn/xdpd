/*
 * cxmpie.h
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#ifndef CXMPIE_H_
#define CXMPIE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>
#ifdef __cplusplus
}
#endif

#include "rofl/common/croflexception.h"
#include "rofl/common/cmemory.h"
#include "xdpd_mgmt_protocol.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class eXmpIeBase 		: public rofl::RoflException {};
class eXmpIeInval		: public eXmpIeBase {};

class cxmpie :
		public rofl::cmemory
{
	union {
		uint8_t						*xmpu_generic;
		struct xmp_ie_header_t		*xmpu_header;
	} xmpie_xmpu;

#define xmpie_generic	xmpie_xmpu.xmpu_generic
#define xmpie_header	xmpie_xmpu.xmpu_header

public:

	/**
	 *
	 */
	cxmpie(
			uint16_t type, uint16_t len);

	/**
	 *
	 */
	cxmpie(
			uint8_t *buf, size_t buflen);

	/**
	 *
	 */
	cxmpie(
			cxmpie const& elem);

	/**
	 *
	 */
	cxmpie&
	operator= (
			cxmpie const& elem);

	/**
	 *
	 */
	virtual
	~cxmpie();

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
	uint16_t
	get_type() const { return be16toh(xmpie_header->type); };

	/**
	 *
	 */
	void
	set_type(uint16_t type) { xmpie_header->type = htobe16(type); };

	/**
	 *
	 */
	uint16_t
	get_length() const { return be16toh(xmpie_header->len); };

	/**
	 *
	 */
	void
	set_length(uint16_t len) { xmpie_header->len = htobe16(len); };

public:

	friend std::ostream&
	operator<< (std::ostream& os, cxmpie const& elem) {
		os << rofl::indent(0) << "<cxmpie ";
		os << "type:" 	<< elem.get_type() 		<< " ";
		os << "length:" << elem.get_length() 	<< " ";
		os << ">" << std::endl;
		return os;
	};
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd



#endif /* CXMPIE_H_ */
