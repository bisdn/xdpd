/*
 * cxmpie_dpid.h
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#ifndef CXMPIE_DPID_H_
#define CXMPIE_DPID_H_

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

class cxmpie_dpid :
		public cxmpie
{
	union {
		uint8_t						*xmpu_generic;
		struct xmp_ie_dpid_t		*xmpu_dpid;
	} xmpie_xmpu;

#define xmpie_generic	xmpie_xmpu.xmpu_generic
#define xmpie_dpid		xmpie_xmpu.xmpu_dpid

public:

	/**
	 *
	 */
	cxmpie_dpid(
			uint64_t dpid = 0);

	/**
	 *
	 */
	cxmpie_dpid(
			uint8_t *buf, size_t buflen);

	/**
	 *
	 */
	cxmpie_dpid(
			cxmpie_dpid const& elem);

	/**
	 *
	 */
	cxmpie_dpid&
	operator= (
			cxmpie_dpid const& elem);

	/**
	 *
	 */
	cxmpie_dpid(
			cxmpie const& elem);

	/**
	 *
	 */
	cxmpie_dpid&
	operator= (
			cxmpie const& elem);

	/**
	 *
	 */
	virtual
	~cxmpie_dpid();

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
	uint64_t
	get_dpid() const { return be64toh(xmpie_dpid->dpid); };

	/**
	 *
	 */
	void
	set_dpid(uint64_t dpid) { xmpie_dpid->dpid = htobe64(dpid); };

public:

	friend std::ostream&
	operator<< (std::ostream& os, cxmpie_dpid const& elem) {
		os << dynamic_cast<cxmpie const&>( elem );
		os << rofl::indent(2) << "<cxmpie-dpid ";
			os << "dpid:" << (unsigned long long)elem.get_dpid() << " ";
		os << ">" << std::endl;
		return os;
	};
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd

#endif /* CXMPIE_DPID_H_ */
