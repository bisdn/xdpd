#include "cxmpie_lsixlsi.h"

using namespace xdpd::mgmt::protocol;

cxmpie_lsixlsi::cxmpie_lsixlsi() :
		cxmpie(XMPIET_LSIXLSI, sizeof(struct xmp_ie_lsixlsi_t))
{
	xmpie_generic = somem();
	// fixme init vars and change constructor?
}

cxmpie_lsixlsi::cxmpie_lsixlsi(uint8_t *buf, size_t buflen) :
		cxmpie(buf, buflen)
{
	xmpie_generic = somem();
	if (buflen < sizeof(struct xmp_ie_lsixlsi_t)) throw eXmpIeInval();
	unpack(buf, buflen);
}

cxmpie_lsixlsi::cxmpie_lsixlsi(cxmpie_lsixlsi const& elem) :
		cxmpie(elem)
{
	*this = elem;
}

cxmpie_lsixlsi&
cxmpie_lsixlsi::operator=(cxmpie_lsixlsi const& elem)
{
	if (this == &elem) return *this;

	cxmpie::operator=(elem);

	xmpie_generic = somem();

	return *this;
}

cxmpie_lsixlsi::cxmpie_lsixlsi(cxmpie const& elem) :
		cxmpie(elem)
{
	*this = elem;
}

cxmpie_lsixlsi&
cxmpie_lsixlsi::operator=(cxmpie const& elem)
{
	if (this == &elem) return *this;

	if (XMPIET_LSIXLSI != elem.get_type()) throw eXmpIeInval();
	if (elem.length() < sizeof(struct xmp_ie_lsixlsi_t)) throw eXmpIeInval();

	cxmpie::operator=(elem);

	xmpie_generic = somem();

	return *this;
}

cxmpie_lsixlsi::~cxmpie_lsixlsi()
{

}

uint8_t*
cxmpie_lsixlsi::resize(size_t len)
{
	return (xmpie_generic = cxmpie::resize(len));
}

size_t
cxmpie_lsixlsi::length() const
{
	return sizeof(struct xmp_ie_lsixlsi_t);
}

void
cxmpie_lsixlsi::pack(uint8_t *buf, size_t buflen)
{
	if (buflen < length()) throw eXmpIeInval();
	cxmpie::pack(buf, length());
}

void
cxmpie_lsixlsi::unpack(uint8_t* buf, size_t buflen)
{
	if (buflen < sizeof(struct xmp_ie_lsixlsi_t)) throw eXmpIeInval();
	cxmpie::unpack(buf, buflen);
	xmpie_generic = somem();
}
