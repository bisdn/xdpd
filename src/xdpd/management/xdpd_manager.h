/*
 * xdpd_manager.h
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#ifndef XDPD_MANAGER_H_
#define XDPD_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>
#ifdef __cplusplus
}
#endif

#include "rofl/common/csocket.h"
#include "switch_manager.h"
#include "port_manager.h"

namespace xdpd {
namespace mgmt {

class xdpd_manager :
		public ciosrv,
		public csocket_owner
{
	csocket					socket;		// listening socket
	std::string				udp_addr;	// binding address
	uint16_t				udp_port;	// listening UDP port

#define MGMT_PORT_UDP_ADDR	"127.0.0.1"
#define MGMT_PORT_UDP_PORT	8444

private:

    static xdpd_manager				*sxdpdmanager;

	/**
	 *
	 */
	xdpd_manager(
			std::string const& udp_addr = std::string(MGMT_PORT_UDP_ADDR),
			uint16_t udp_port = MGMT_PORT_UDP_PORT);

	/**
	 *
	 */
	xdpd_manager(
			xdpd_manager const& mgr);

	/**
	 *
	 */
	virtual
	~xdpd_manager();

public:

    /**
     *
     */
	static xdpd_manager&
	get_instance();

protected:

	/*
	 * overloaded from ciosrv
	 */

	virtual void
	handle_timeout(
			int opaque);

protected:

	/*
	 * overloaded from csocket_owner
	 */

	virtual void
	handle_accepted(csocket *socket, int newsd, caddress const& ra) {};

	virtual void
	handle_connected(csocket *socket, int sd) {};

	virtual void
	handle_connect_refused(csocket *socket, int sd) {};

	virtual void
	handle_read(csocket *socket, int sd);

	virtual void
	handle_closed(csocket *socket, int sd) {};

};

}; // end of namespace mgmt
}; // end of namespace xdpd



#endif /* XDPD_MANAGER_H_ */
