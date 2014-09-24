/*
 * cxmpie_lsiinfo.cc
 *
 *  Created on: Jul 21, 2014
 *      Author: tobi
 */

#include "cxmpie_lsiinfo.h"

#include <stddef.h>

#include "../../snapshots/switch_snapshot.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

cxmpie_lsiinfo::cxmpie_lsiinfo() :
		cxmpie(XMPIET_LSIINFO, sizeof(struct xmp_ie_lsiinfo_t))
{
}

cxmpie_lsiinfo::cxmpie_lsiinfo(xdpd::openflow_switch_snapshot const &snapshot) :
		cxmpie(XMPIET_LSIINFO, sizeof(struct xmp_ie_lsiinfo_t))
{
	xmpie_generic = somem();
	set_lsiname(snapshot.name);
	set_dpid(snapshot.dpid);
	set_capabilities(snapshot.capabilities);
	set_num_of_buffers(snapshot.num_of_buffers);
	set_max_ports(512); // fixme not yet exported
	set_max_tables(snapshot.num_of_tables);
}

cxmpie_lsiinfo::cxmpie_lsiinfo(uint8_t* buf, size_t buflen) :
		cxmpie(buf, buflen)
{
}

cxmpie_lsiinfo::cxmpie_lsiinfo(cxmpie const& elem) :
		cxmpie(elem)
{
	*this = elem;
}

cxmpie_lsiinfo::cxmpie_lsiinfo(cxmpie_lsiinfo const& elem) :
		cxmpie(elem)
{
	*this = elem;
}

cxmpie_lsiinfo&
cxmpie_lsiinfo::operator =(cxmpie_lsiinfo const& elem)
{
	if (this == &elem) return *this;

	cxmpie::operator=(elem);

	xmpie_generic = somem();

	return *this;
}
cxmpie_lsiinfo&
cxmpie_lsiinfo::operator=(cxmpie const& elem)
{
	if (this == &elem) return *this;

	if (XMPIET_LSIINFO != elem.get_type()) throw eXmpIeInval();
	if (elem.length() < sizeof(struct xmp_ie_name_t)) throw eXmpIeInval();

	cxmpie::operator=(elem);

	xmpie_generic = somem();

	return *this;
}

cxmpie_lsiinfo::~cxmpie_lsiinfo()
{
	// TODO Auto-generated destructor stub
}

uint8_t*
cxmpie_lsiinfo::resize(size_t len)
{
	return (xmpie_generic = cxmpie::resize(len));
}

size_t
cxmpie_lsiinfo::length() const
{
	return sizeof(struct xmp_ie_lsiinfo_t);
}

void
cxmpie_lsiinfo::pack(uint8_t* buf, size_t buflen)
{
	if (buflen < length()) throw eXmpIeInval();
	cxmpie::pack(buf, length());
}

void
cxmpie_lsiinfo::unpack(uint8_t* buf, size_t buflen)
{
	if (buflen < sizeof(struct xmp_ie_lsiinfo_t)) throw eXmpIeInval();
	cxmpie::unpack(buf, buflen);
	xmpie_generic = somem();
}

std::string
cxmpie_lsiinfo::get_lsiname() const
{
	return std::string(xmpie_lsiinfo->lsiname,
			strnlen(xmpie_lsiinfo->lsiname, XMPIE_NAME_SIZE));
}

void
cxmpie_lsiinfo::set_lsiname(const std::string& lsiname)
{
	size_t len =
			(lsiname.length() < XMPIE_NAME_SIZE) ?
					lsiname.length() : XMPIE_NAME_SIZE;
	memset(xmpie_lsiinfo->lsiname, 0, XMPIE_NAME_SIZE);
	memcpy(xmpie_lsiinfo->lsiname, lsiname.c_str(), len);
}

uint64_t
cxmpie_lsiinfo::get_dpid() const
{
	return be64toh(xmpie_lsiinfo->dpid);
}

void
cxmpie_lsiinfo::set_dpid(uint64_t dpid)
{
	xmpie_lsiinfo->dpid = htobe64(dpid);
}

uint32_t
cxmpie_lsiinfo::get_capabilities() const
{
	return be32toh(xmpie_lsiinfo->capabilities);
}

void
cxmpie_lsiinfo::set_capabilities(uint32_t capabilities)
{
	xmpie_lsiinfo->capabilities = htobe32(capabilities);
}

uint32_t
cxmpie_lsiinfo::get_num_of_buffers() const
{
	return be32toh(xmpie_lsiinfo->num_of_buffers);
}

void
cxmpie_lsiinfo::set_num_of_buffers(uint32_t num_of_buffers)
{
	xmpie_lsiinfo->num_of_buffers = htobe32(num_of_buffers);
}

uint32_t
cxmpie_lsiinfo::get_max_ports() const
{
	return be32toh(xmpie_lsiinfo->max_ports);
}

void
cxmpie_lsiinfo::set_max_ports(uint32_t max_ports)
{
	xmpie_lsiinfo->max_ports = htobe32(max_ports);
}

uint8_t
cxmpie_lsiinfo::get_max_tables() const
{
	return xmpie_lsiinfo->max_tables;
}

void
cxmpie_lsiinfo::set_max_tables(uint8_t max_tables)
{
	xmpie_lsiinfo->max_tables = max_tables;
}

} /* namespace protocol */
} /* namespace mgmt */
} /* namespace xdpd */
