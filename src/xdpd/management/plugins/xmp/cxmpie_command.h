/*
 * cxmpie_command.h
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#ifndef CXMPIE_COMMAND_H_
#define CXMPIE_COMMAND_H_

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

class cxmpie_command :
		public cxmpie
{
	union {
		uint8_t						*xmpu_generic;
		struct xmp_ie_command_t		*xmpu_command;
	} xmpie_xmpu;

#define xmpie_generic	xmpie_xmpu.xmpu_generic
#define xmpie_command	xmpie_xmpu.xmpu_command

public:

	/**
	 *
	 */
	cxmpie_command(
			uint32_t command = 0);

	/**
	 *
	 */
	cxmpie_command(
			uint8_t *buf, size_t buflen);

	/**
	 *
	 */
	cxmpie_command(
			cxmpie_command const& elem);

	/**
	 *
	 */
	cxmpie_command&
	operator= (
			cxmpie_command const& elem);

	/**
	 *
	 */
	cxmpie_command(
			cxmpie const& elem);

	/**
	 *
	 */
	cxmpie_command&
	operator= (
			cxmpie const& elem);

	/**
	 *
	 */
	virtual
	~cxmpie_command();

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
	uint32_t
	get_command() const { return be32toh(xmpie_command->cmd); };

	/**
	 *
	 */
	void
	set_command(uint32_t command) { xmpie_command->cmd = htobe32(command); };

public:

	friend std::ostream&
	operator<< (std::ostream& os, cxmpie_command const& elem) {
		os << dynamic_cast<cxmpie const&>( elem );
		os << rofl::indent(2) << "<cxmpie-command ";
			os << "command:" << (unsigned int)elem.get_command() << " ";
		os << ">" << std::endl;
		return os;
	};
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd

#endif /* CXMPIE_COMMAND_H_ */
