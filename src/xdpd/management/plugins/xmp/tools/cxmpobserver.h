/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * cxmpobserver.h
 *
 *  Created on: Jul 17, 2014
 *      Author: tobi
 */

#ifndef CXMPOBSERVER_H_
#define CXMPOBSERVER_H_

#include "../cxmpmsg.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class cxmpobserver {
public:
	virtual
	~cxmpobserver();

	virtual void
	notify(const cxmpmsg &msg) = 0;
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd




#endif /* CXMPOBSERVER_H_ */
