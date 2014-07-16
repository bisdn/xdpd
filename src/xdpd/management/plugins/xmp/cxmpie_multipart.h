/*
 * cxmpie_multipart.h
 *
 *  Created on: Jul 15, 2014
 *      Author: tobi
 */

#ifndef CXMPIE_MULTIPART_H_
#define CXMPIE_MULTIPART_H_

#include <inttypes.h>
#include <stddef.h>
#include <deque>

#include "cxmpie.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class cxmpie_multipart : public cxmpie {
	union {
		uint8_t						*xmpu_generic;
		struct xmp_ie_header_t		*xmpu_multipart;
	} xmpie_xmpu;

#define xmpie_generic	xmpie_xmpu.xmpu_generic
#define xmpie_multipart	xmpie_xmpu.xmpu_multipart

public:
	cxmpie_multipart();
	~cxmpie_multipart();

	cxmpie_multipart(cxmpie_multipart const &elem);

	cxmpie_multipart(cxmpie const& elem);

	cxmpie_multipart&
	operator=(cxmpie_multipart const &elem);

	void
	push_back(cxmpie *ie);

	virtual size_t
	length() const;

	virtual void
	pack(uint8_t *buf, size_t buflen);

	virtual void
	unpack(uint8_t *buf, size_t buflen);

	friend std::ostream&
	operator<< (std::ostream& os, cxmpie_multipart const& elem) {
		os << dynamic_cast<cxmpie const&>( elem );
		os << rofl::indent(2) << "<cxmpie-multipart";
		os << " #IE=" << elem.ies.size();
		os << ">" << std::endl;
		rofl::indent::inc(2);
		for(std::deque<cxmpie*>::const_iterator iter = elem.ies.begin(); iter != elem.ies.end(); ++iter) {
			os << **iter;
		}

		return os;
	};

private:
	void
	copy_ies(std::deque<cxmpie*> const &ies);

	cxmpie*
	bakery(cxmpie *ref);

	void
	clear();

	std::deque<cxmpie*> ies;

};

} /* namespace protocol */
} /* namespace mgmt */
} /* namespace xdpd */

#endif /* CXMPIE_MULTIPART_H_ */
