/*
 * xmp.h
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#ifndef XDPD_MANAGER_H_
#define XDPD_MANAGER_H_

#include <inttypes.h>
#include <rofl/common/csocket.h>

#include "../../switch_manager.h"
#include "../../port_manager.h"
#include "../../plugin_manager.h"

#include "cxmpmsg.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class xmp :
		public rofl::ciosrv,
		public rofl::csocket_owner,
		public plugin
{
	rofl::csocket*			socket;			// listening socket
	rofl::cparams			socket_params;

#define MGMT_PORT_UDP_ADDR	"127.0.0.1"
#define MGMT_PORT_UDP_PORT	"8444"

public:

	xmp();

	virtual ~xmp();

	virtual void init();

	virtual std::string get_name(void){
		return std::string("xmp");
	};

protected:

	/*
	 * overloaded from ciosrv
	 */

	virtual void
	handle_timeout(
			int opaque, void *data = (void*)0);

protected:

	/*
	 * overloaded from csocket_owner
	 */

	virtual void
	handle_listen(rofl::csocket& socket, int newsd) {};

	virtual void
	handle_accepted(rofl::csocket& socket) {};

	virtual void
	handle_accept_refused(rofl::csocket& socket) {};

	virtual void
	handle_connected(rofl::csocket& socket) {};

	virtual void
	handle_connect_refused(rofl::csocket& socket) {};

	virtual void
	handle_connect_failed(rofl::csocket& socket) {};

	virtual void
	handle_write(rofl::csocket& socket) {};

	virtual void
	handle_read(rofl::csocket& socket);

	virtual void
	handle_closed(rofl::csocket& socket) {};

private:

	void
	handle_request(
			cxmpmsg& msg);

	void
	handle_port_attach(
			cxmpmsg& msg);

	void
	handle_port_detach(
			cxmpmsg& msg);

	void
	handle_port_enable(
			cxmpmsg& msg);

	void
	handle_port_disable(
			cxmpmsg& msg);
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd



#endif /* XDPD_MANAGER_H_ */
