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
