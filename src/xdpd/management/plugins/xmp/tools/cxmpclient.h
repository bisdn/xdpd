/*
 * cxmpclient.h
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#ifndef CXMPCLIENT_H_
#define CXMPCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>
#ifdef __cplusplus
}
#endif

#include "rofl/common/ciosrv.h"
#include "rofl/common/cmemory.h"
#include "rofl/common/csocket.h"
#include "../cxmpmsg.h"
#include "cxmpobserver.h"

namespace xdpd {
namespace mgmt {
namespace protocol {

class cxmpclient :
	public rofl::ciosrv,
	public rofl::csocket_owner
{
	enum cxmpclient_event_t {
		WANT_SEND = 1,
	};

	enum cxmpclient_timer_t {
		TIMER_XMPCLNT_EXIT 		= 1,
	};

	rofl::csocket*		socket;
	rofl::cparams		socket_params;
	rofl::csockaddr		dest;
	rofl::cmemory*		mem;
	rofl::cmemory*		fragment;
	unsigned int		msg_bytes_read;
	cxmpobserver *observer;

	bool auto_exit;
	const int exit_timeout;

public:

	/**
	 *
	 */
	cxmpclient();

	/**
	 *
	 */
	virtual
	~cxmpclient();

public:

	void
	port_list();

	void
	port_list(uint64_t dpid);

	void
	port_info();

	void
	lsi_list();

	/**
	 *
	 */
	void
	port_attach(
			uint64_t dpid, std::string const& portname);

	/**
	 *
	 */
	void
	port_detach(
			uint64_t dpid, std::string const& portname);

	/**
	 *
	 */
	void
	port_enable(
			std::string const& portname);

	/**
	 *
	 */
	void
	port_disable(
			std::string const& portname);

	void
	handle_reply(cxmpmsg& msg);

	void
	register_observer(cxmpobserver *observer);

	bool
	is_auto_exit() const
	{
		return auto_exit;
	}

	void
	set_auto_exit(bool autoExit)
	{
		auto_exit = autoExit;
	}

protected:

	virtual void
	handle_timeout(int opaque, void *data = (void*)0);

protected:

	virtual void
	handle_event(rofl::cevent const& ev);

	virtual void
	handle_listen(rofl::csocket& socket, int newsd) {};

	virtual void
	handle_accepted(rofl::csocket& socket) {};

	virtual void
	handle_accept_refused(rofl::csocket& socket) {};

	virtual void
	handle_connected(rofl::csocket& socket);

	virtual void
	handle_connect_refused(rofl::csocket& socket);

	virtual void
	handle_connect_failed(rofl::csocket& socket);

	virtual void
	handle_write(rofl::csocket& socket);

	virtual void
	handle_read(rofl::csocket& socket);

	virtual void
	handle_closed(rofl::csocket& socket);

	void
	handle_send();
};

}; // end of namespace protocol
}; // end of namespace mgmt
}; // end of namespace xdpd



#endif /* CXMPCLIENT_H_ */
