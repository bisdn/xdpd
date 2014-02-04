/*
 * cxmpmsg.h
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#ifndef CXMPMSG_H_
#define CXMPMSG_H_

#include "rofl/common/croflexception.h"
#include "rofl/common/logging.h"
#include "rofl/common/cmemory.h"
#include "xdpd_mgmt_protocol.h"
#include "cxmpies.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class eXmpBase 		: public rofl::RoflException {};
class eXmpInval		: public eXmpBase {};

class cxmpmsg :
	public rofl::cmemory
{
	union {
		uint8_t					*xmpu_generic;
		struct xmp_header_t		*xmpu_header;
	} xmp_xmpu;

#define xmp_generic xmp_xmpu.xmpu_generic
#define xmp_header 	xmp_xmpu.xmpu_header

	cxmpies						xmpies;

public:

	/**
	 *
	 * @param version
	 * @param type
	 * @param len
	 * @param xid
	 */
	cxmpmsg(
			uint8_t version, uint8_t type, uint16_t len = sizeof(struct xmp_header_t), uint32_t xid = 0);


	/**
	 *
	 * @param buf
	 * @param buflen
	 */
	cxmpmsg(
			uint8_t *buf, size_t buflen);


	/**
	 *
	 * @param msg
	 */
	cxmpmsg(
			cxmpmsg const& msg);

	/**
	 *
	 * @param msg
	 * @return
	 */
	cxmpmsg&
	operator= (
			cxmpmsg const& msg);

	/**
	 *
	 */
	virtual
	~cxmpmsg();

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
	pack(uint8_t *buf, size_t buflen);

	/**
	 *
	 */
	virtual void
	unpack(uint8_t *buf, size_t buflen);

public:

	/**
	 *
	 * @return
	 */
	uint8_t
	get_version() const { return xmp_header->version; };

	/**
	 *
	 */
	void
	set_version(uint8_t version) { xmp_header->version = version; };

	/**
	 *
	 * @return
	 */
	uint8_t
	get_type() const { return xmp_header->type; };

	/**
	 *
	 */
	void
	set_type(uint8_t type) { xmp_header->type = type; };

	/**
	 *
	 */
	uint16_t
	get_length() const { return be16toh(xmp_header->len); };

	/**
	 *
	 */
	void
	set_length(uint16_t len) { xmp_header->len = htobe16(len); };

	/**
	 *
	 */
	uint32_t
	get_xid() const { return be32toh(xmp_header->xid); };

	/**
	 *
	 */
	void
	set_xid(uint32_t xid) { xmp_header->xid = htobe32(xid); };

	/**
	 *
	 */
	cxmpies&
	get_xmpies() { return xmpies; };

public:

	friend std::ostream&
	operator<< (std::ostream& os, cxmpmsg const& msg) {
		os << rofl::indent(0) << "<cxmpmsg ";
		os << "version:" 	<< (int)msg.get_version() 	<< " ";
		os << "type:" 		<< (int)msg.get_type() 		<< " ";
		os << "length:" 	<< (int)msg.get_length() 	<< " ";
		os << "xid:0x" 		<< std::hex << (int)msg.get_xid() << std::dec << " ";
		os << ">" << std::endl;
		rofl::indent i(2);
		os << msg.xmpies;
	return os;
	};
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd


#endif /* MGMT_MESSAGE_H_ */


