/*
 * cxmpie_portinfo.cc
 *
 *  Created on: Jul 21, 2014
 *      Author: tobi
 */

#include "cxmpie_portinfo.h"

#include <stddef.h>

#include "../../snapshots/port_snapshot.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

cxmpie_portinfo::cxmpie_portinfo() :
		cxmpie(XMPIET_PORTINFO, sizeof(struct xmp_ie_portinfo_t))
{
}

cxmpie_portinfo::cxmpie_portinfo(xdpd::port_snapshot const &snapshot) :
		cxmpie(XMPIET_PORTINFO, sizeof(struct xmp_ie_portinfo_t))
{
	xmpie_generic = somem();
	set_port_num(snapshot.of_port_num);
	set_portname(snapshot.name);
	set_features_current(snapshot.curr);
	set_features_supported(snapshot.supported);
	set_features_advertised_peer(snapshot.peer);
	set_current_rate(snapshot.curr_speed);
	set_current_rate_max(snapshot.curr_max_speed);
	set_state(snapshot.state);
}

cxmpie_portinfo::cxmpie_portinfo(uint8_t* buf, size_t buflen) :
		cxmpie(buf, buflen)
{
}

cxmpie_portinfo::cxmpie_portinfo(cxmpie const& elem) :
		cxmpie(elem)
{
	*this = elem;
}

cxmpie_portinfo::cxmpie_portinfo(cxmpie_portinfo const& elem) :
		cxmpie(elem)
{
	*this = elem;
}

cxmpie_portinfo&
cxmpie_portinfo::operator =(cxmpie_portinfo const& elem)
{
	if (this == &elem) return *this;

	cxmpie::operator=(elem);

	xmpie_generic = somem();

	return *this;
}
cxmpie_portinfo&
cxmpie_portinfo::operator=(cxmpie const& elem)
{
	if (this == &elem) return *this;

	if (XMPIET_PORTINFO != elem.get_type()) throw eXmpIeInval();
	if (elem.length() < sizeof(struct xmp_ie_name_t)) throw eXmpIeInval();

	cxmpie::operator=(elem);

	xmpie_generic = somem();

	return *this;
}

cxmpie_portinfo::~cxmpie_portinfo()
{
	// TODO Auto-generated destructor stub
}

uint8_t*
cxmpie_portinfo::resize(size_t len)
{
	return (xmpie_generic = cxmpie::resize(len));
}

size_t
cxmpie_portinfo::length() const
{
	return sizeof(struct xmp_ie_portinfo_t);
}

void
cxmpie_portinfo::pack(uint8_t* buf, size_t buflen)
{
	if (buflen < length()) throw eXmpIeInval();
	cxmpie::pack(buf, length());
}

void
cxmpie_portinfo::unpack(uint8_t* buf, size_t buflen)
{
	if (buflen < sizeof(struct xmp_ie_portinfo_t)) throw eXmpIeInval();
	cxmpie::unpack(buf, buflen);
	xmpie_generic = somem();
}

uint32_t
cxmpie_portinfo::get_port_num() const
{
	return be32toh(xmpie_portinfo->of_port_num);
}

void
cxmpie_portinfo::set_port_num(uint32_t port_num)
{
	xmpie_portinfo->of_port_num = htobe32(port_num);
}

std::string
cxmpie_portinfo::get_portname() const
{
	return std::string(xmpie_portinfo->portname,
			strnlen(xmpie_portinfo->portname, XMPIE_NAME_SIZE));
}

void
cxmpie_portinfo::set_portname(const std::string& portname)
{
	size_t len =
			(portname.length() < XMPIE_NAME_SIZE) ?
					portname.length() : XMPIE_NAME_SIZE;
	memset(xmpie_portinfo->portname, 0, XMPIE_NAME_SIZE);
	memcpy(xmpie_portinfo->portname, portname.c_str(), len);
}

uint32_t
cxmpie_portinfo::get_features_current() const
{
	return be32toh(xmpie_portinfo->feat_curr);
}

void
cxmpie_portinfo::set_features_current(uint32_t feat_curr)
{
	xmpie_portinfo->feat_curr = htobe32(feat_curr);
}

uint32_t
cxmpie_portinfo::get_features_supported() const
{
	return be32toh(xmpie_portinfo->feat_supported);
}

void
cxmpie_portinfo::set_features_supported(uint32_t feat_supported)
{
	xmpie_portinfo->feat_supported = htobe32(feat_supported);
}

uint32_t
cxmpie_portinfo::get_features_advertised_peer() const
{
	return be32toh(xmpie_portinfo->feat_peer);
}

void
cxmpie_portinfo::set_features_advertised_peer(uint32_t feat_peer)
{
	xmpie_portinfo->feat_peer = htobe32(feat_peer);
}

uint64_t
cxmpie_portinfo::get_current_rate() const
{
	return be64toh(xmpie_portinfo->curr_speed);
}

void
cxmpie_portinfo::set_current_rate(uint64_t rate)
{
	xmpie_portinfo->curr_speed = htobe64(rate);
}

uint64_t
cxmpie_portinfo::get_current_rate_max() const
{
	return be64toh(xmpie_portinfo->max_speed);
}

void
cxmpie_portinfo::set_current_rate_max(uint64_t rate)
{
	xmpie_portinfo->max_speed = htobe64(rate);
}

uint32_t
cxmpie_portinfo::get_state() const
{
	return be32toh(xmpie_portinfo->state);
}

void
cxmpie_portinfo::set_state(uint32_t state)
{
	xmpie_portinfo->state = htobe32(state);
}

} /* namespace protocol */
} /* namespace mgmt */
} /* namespace xdpd */
