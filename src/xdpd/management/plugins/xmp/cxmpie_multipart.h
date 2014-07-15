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
		os << rofl::indent(2) << "<cxmpie-multipart ";
			os << " not yet implemented ";
		os << ">" << std::endl;
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
