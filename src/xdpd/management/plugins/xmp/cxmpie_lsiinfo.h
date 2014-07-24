/*
 * cxmpie_lsiinfo.h
 *
 *  Created on: Jul 21, 2014
 *      Author: tobi
 */

#ifndef CXMPIE_LSIINFO_H_
#define CXMPIE_LSIINFO_H_

#include <inttypes.h>
#include "cxmpie.h"
#include "xdpd_mgmt_protocol.h"

namespace xdpd {

class openflow_switch_snapshot;

namespace mgmt {
namespace protocol {

class cxmpie_lsiinfo : public cxmpie {

	union {
		uint8_t					*xmpu_generic;
		struct xmp_ie_lsiinfo_t	*xmpu_lsiinfo;
	} xmpie_xmpu;

#define xmpie_generic	xmpie_xmpu.xmpu_generic
#define xmpie_lsiinfo	xmpie_xmpu.xmpu_lsiinfo

public:
	cxmpie_lsiinfo();

	cxmpie_lsiinfo(xdpd::openflow_switch_snapshot const &snapshot);

	cxmpie_lsiinfo(uint8_t *buf, size_t buflen);

	cxmpie_lsiinfo(cxmpie const& elem);

	cxmpie_lsiinfo(cxmpie_lsiinfo const& elem);

	cxmpie_lsiinfo&
	operator=(cxmpie_lsiinfo const& elem);

	cxmpie_lsiinfo&
	operator=(cxmpie const& elem);

	~cxmpie_lsiinfo();

	virtual uint8_t*
	resize(size_t len);

	virtual size_t
	length() const;

	virtual void
	pack(uint8_t *buf, size_t buflen);

	virtual void
	unpack(uint8_t *buf, size_t buflen);

	std::string
	get_lsiname() const;

	void
	set_lsiname(std::string const& lsiname);

	uint32_t
	get_capabilities() const;

	void
	set_capabilities(uint32_t capabilities);

	uint32_t
	get_num_of_buffers() const;

	void
	set_num_of_buffers(uint32_t);

	uint32_t
	get_max_ports() const;

	void
	set_max_ports(uint32_t);

	uint8_t
	get_max_tables() const;

	void
	set_max_tables(uint8_t);

	friend std::ostream&
	operator<<(std::ostream& os, cxmpie_lsiinfo const& elem)
	{
		os << dynamic_cast<cxmpie const&>(elem);
		os << rofl::indent(2) << "<cxmpie-lsiinfo ";
		os << "todo...";
		os << ">" << std::endl;
		return os;
	}
	;
};

} /* namespace protocol */
} /* namespace mgmt */
} /* namespace xdpd */

#endif /* CXMPIE_LSIINFO_H_ */
