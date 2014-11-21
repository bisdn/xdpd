#ifndef CXMPIE_LSIXLSI_H_
#define CXMPIE_LSIXLSI_H_

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

class cxmpie_lsixlsi : public cxmpie {
	union {
		uint8_t *xmpu_generic;
		struct xmp_ie_lsixlsi_t *xmpu_lsixlsi;
	} xmpie_xmpu;

#define xmpie_generic	xmpie_xmpu.xmpu_generic
#define xmpie_lsixlsi	xmpie_xmpu.xmpu_lsixlsi

public:

	/**
	 *
	 */
	cxmpie_lsixlsi();

	/**
	 *
	 */
	cxmpie_lsixlsi(uint8_t *buf, size_t buflen);

	/**
	 *
	 */
	cxmpie_lsixlsi(cxmpie_lsixlsi const& elem);

	/**
	 *
	 */
	cxmpie_lsixlsi&
	operator=(cxmpie_lsixlsi const& elem);

	/**
	 *
	 */
	cxmpie_lsixlsi(cxmpie const& elem);

	/**
	 *
	 */
	cxmpie_lsixlsi&
	operator=(cxmpie const& elem);

	/**
	 *
	 */
	virtual
	~cxmpie_lsixlsi();

public:

	/**
	 *
	 */
	virtual uint8_t*
	resize(size_t len);

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

	uint64_t
	get_dpid1() const
	{
		return be64toh(xmpie_lsixlsi->dpid1);
	}

	void
	set_dpid1(uint64_t dpid)
	{
		xmpie_lsixlsi->dpid1 = htobe64(dpid);
	}

	uint32_t
	get_portno1() const
	{
		return be32toh(xmpie_lsixlsi->portno1);
	}

	void
	set_portno1(uint32_t portno)
	{
		xmpie_lsixlsi->portno1 = htobe32(portno);
	}

	uint64_t
	get_dpid2() const
	{
		return be64toh(xmpie_lsixlsi->dpid2);
	}

	void
	set_dpid2(uint64_t dpid)
	{
		xmpie_lsixlsi->dpid2 = htobe64(dpid);
	}

	uint32_t
	get_portno2() const
	{
		return be32toh(xmpie_lsixlsi->portno2);
	}

	void
	set_portno2(uint32_t portno)
	{
		xmpie_lsixlsi->portno2 = htobe32(portno);
	}

public:

	friend std::ostream&
	operator<<(std::ostream& os, cxmpie_lsixlsi const& elem)
	{
		os << dynamic_cast<cxmpie const&>(elem);
		os << rofl::indent(2) << "<cxmpie-lsixlsi FIXME";
		os << ">" << std::endl;
		return os;
	}
};

} // end of namespace protocol
} // end of namespace mgmt
} // end of namespace xdpd

#endif /* CXMPIE_LSIXLSI_H_ */
