/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * cxmpie_portinfo.h
 *
 *  Created on: Jul 21, 2014
 *      Author: tobi
 */

#ifndef CXMPIE_PORTINFO_H_
#define CXMPIE_PORTINFO_H_

#include <inttypes.h>
#include "cxmpie.h"
#include "xdpd_mgmt_protocol.h"

namespace xdpd {

class port_snapshot;

namespace mgmt {
namespace protocol {

class cxmpie_portinfo : public cxmpie {

	union {
		uint8_t *xmpu_generic;
		struct xmp_ie_portinfo_t *xmpu_portinfo;
	} xmpie_xmpu;

#define xmpie_generic	xmpie_xmpu.xmpu_generic
#define xmpie_portinfo	xmpie_xmpu.xmpu_portinfo

public:
	cxmpie_portinfo();

	cxmpie_portinfo(xdpd::port_snapshot const &snapshot);

	cxmpie_portinfo(uint8_t *buf, size_t buflen);

	cxmpie_portinfo(cxmpie const& elem);

	cxmpie_portinfo(cxmpie_portinfo const& elem);

	cxmpie_portinfo&
	operator=(cxmpie_portinfo const& elem);

	cxmpie_portinfo&
	operator=(cxmpie const& elem);

	~cxmpie_portinfo();

	virtual uint8_t*
	resize(size_t len);

	virtual size_t
	length() const;

	virtual void
	pack(uint8_t *buf, size_t buflen);

	virtual void
	unpack(uint8_t *buf, size_t buflen);

	uint32_t
	get_port_num() const;

	void
	set_port_num(uint32_t port_num);

	std::string
	get_portname() const;

	void
	set_portname(std::string const& portname);

	uint32_t
	get_features_current() const;

	void
	set_features_current(uint32_t);

	uint32_t
	get_features_supported() const;

	void
	set_features_supported(uint32_t);

	uint32_t
	get_features_advertised_peer() const;

	void
	set_features_advertised_peer(uint32_t);

	uint64_t
	get_current_rate() const;

	void
	set_current_rate(uint64_t rate);

	uint64_t
	get_current_rate_max() const;

	void
	set_current_rate_max(uint64_t rate);

	uint32_t
	get_state() const;

	void
	set_state(uint32_t);

	friend std::ostream&
	operator<<(std::ostream& os, cxmpie_portinfo const& elem)
	{
		os << dynamic_cast<cxmpie const&>(elem);
		os << rofl::indent(2) << "<cxmpie-portinfo ";
		os << "todo...";
		os << ">" << std::endl;
		return os;
	}
	;
};

} /* namespace protocol */
} /* namespace mgmt */
} /* namespace xdpd */

#endif /* CXMPIE_PORTINFO_H_ */
