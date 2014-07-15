/*
 * xmp.cc
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#include "xmp.h"

using namespace rofl; 
using namespace xdpd::mgmt::protocol;



xmp::xmp() :
		socket(NULL),
		socket_type(rofl::csocket::SOCKET_TYPE_PLAIN),
		fragment(NULL),
		msg_bytes_read(0)
{
	socket_params = rofl::csocket::get_default_params(rofl::csocket::SOCKET_TYPE_PLAIN);

	socket_params.set_param(rofl::csocket::PARAM_KEY_LOCAL_HOSTNAME).set_string(MGMT_PORT_UDP_ADDR);
	socket_params.set_param(rofl::csocket::PARAM_KEY_LOCAL_PORT).set_string(MGMT_PORT_UDP_PORT);
	socket_params.set_param(rofl::csocket::PARAM_KEY_DOMAIN).set_string("inet");
	socket_params.set_param(rofl::csocket::PARAM_KEY_TYPE).set_string("stream");
	socket_params.set_param(rofl::csocket::PARAM_KEY_PROTOCOL).set_string("tcp");
}

xmp::~xmp()
{
	if (socket) {
		socket->close();
		delete socket;
	}
	for (std::set<rofl::csocket*>::iterator
			it = workers.begin(); it != workers.end(); ++it) {
		(*(*it)).close();
		delete (*it);
	}
}

void
xmp::init()
{
	if (socket) {
		socket->close();
		delete socket;
		socket = (rofl::csocket*)0;
	}
	(socket = rofl::csocket::csocket_factory(socket_type, this))->listen(socket_params);
}

void
xmp::handle_listen(
		rofl::csocket& socket,
		int newsd)
{
	rofl::logging::info << "[xdpd][plugin][xmp]" << __PRETTY_FUNCTION__ << std::endl;
	rofl::logging::debug << "socket: " << socket;

	rofl::csocket* worker;
	(worker = rofl::csocket::csocket_factory(socket_type, this))->accept(socket_params, newsd);

	rofl::logging::debug << "worker: " << (*worker);
}

void
xmp::handle_accepted(
		rofl::csocket& socket)
{
	rofl::logging::info << "[xdpd][plugin][xmp]" << __PRETTY_FUNCTION__ << ": " << socket << std::endl;
	rofl::logging::debug << socket;

	rofl::csocket* worker = &socket;
	workers.insert(worker);
}

void
xmp::handle_accept_refused(
		rofl::csocket& socket)
{
	rofl::logging::info << "[xdpd][plugin][xmp]" << __PRETTY_FUNCTION__ << std::endl;

	rofl::csocket* worker = &socket;
	delete worker;
}

void
xmp::handle_closed(
		rofl::csocket& socket)
{
	rofl::logging::info << "[xdpd][plugin][xmp]" << __PRETTY_FUNCTION__ << std::endl;
	rofl::logging::debug << socket;
	rofl::csocket* worker = &socket;
	workers.erase(worker);
	delete worker;
}

void
xmp::handle_timeout(
		int opaque, void *data)
{
	rofl::logging::info << "[xdpd][plugin][xmp]" << __PRETTY_FUNCTION__ << std::endl;
	switch (opaque) {
	default:
		;;
	}
}

void
xmp::handle_read(
		csocket& socket)
{
	rofl::logging::info << "[xdpd][plugin][xmp]" << __PRETTY_FUNCTION__ << std::endl;
	rofl::logging::debug << socket;


	try {

		if (0 == fragment) {
			fragment = new rofl::cmemory(sizeof(struct xmp_header_t));
			msg_bytes_read = 0;
		}

		while (true) {
			uint16_t msg_len = 0;

			// how many bytes do we have to read?
			if (msg_bytes_read < sizeof(struct xmp_header_t)) {
				msg_len = sizeof(struct xmp_header_t);
			} else {
				struct xmp_header_t *header = (struct xmp_header_t*)fragment->somem();
				msg_len = be16toh(header->len);
			}

			// sanity check: 8 <= msg_len <= 2^16
			if (msg_len < sizeof(struct xmp_header_t)) {
				logging::warn << "[xdpd][plugin][xmp] received message with invalid length field, closing socket." << std::endl;
				socket.close();
				workers.erase(&socket);
				return;
			}

			// resize msg buffer, if necessary
			if (fragment->memlen() < msg_len) {
				fragment->resize(msg_len);
			}

			// read from socket more bytes, at most "msg_len - msg_bytes_read"
			int nbytes = socket.recv((void*)(fragment->somem() + msg_bytes_read), msg_len - msg_bytes_read);

			msg_bytes_read += nbytes;

			// minimum message length received, check completeness of message
			if (fragment->memlen() >= sizeof(struct xmp_header_t)) {
				struct xmp_header_t *header = (struct xmp_header_t*)fragment->somem();
				uint16_t msg_len = be16toh(header->len);

				// ok, message was received completely
				if (msg_len == msg_bytes_read) {
					rofl::cmemory *mem = fragment;
					fragment = NULL; // just in case, we get an exception from parse_message()
					msg_bytes_read = 0;

					cxmpmsg msg(mem->somem(), msg_len);

					switch (header->type) {
					case XMPT_REQUEST:
					{
						handle_request(socket, msg);
					}
						break;
					case XMPT_REPLY:
					case XMPT_REPLY_MULTIPART:
					case XMPT_NOTIFICATION:
					default:
					{
						rofl::logging::error
								<< "[xdpd][plugin][xmp] unknown message rcvd"
								<< std::endl;
					}
						;
					}

					return;
				}
			}
		}

	} catch (rofl::eSocketRxAgain& e) {

		// more bytes are needed, keep pointer to msg in "fragment"
		rofl::logging::debug << "[xdpd][plugin][xmp] read again" << std::endl;

	} catch (rofl::eSysCall& e) {

		rofl::logging::warn << "[xdpd][plugin][xmp] closing socket: " << e << std::endl;

		if (fragment) {
			delete fragment; fragment = (rofl::cmemory*)0;
		}

		// close socket, as it seems, we are out of sync
		socket.close();

	} catch (rofl::RoflException& e) {

		rofl::logging::warn << "[xdpd][plugin][xmp] dropping invalid message: " << e << std::endl;

		if (fragment) {
			delete fragment; fragment = (rofl::cmemory*)0;
		}

		// close socket, as it seems, we are out of sync
		socket.close();
	}
}


void
xmp::handle_request(csocket& socket, cxmpmsg& msg)
{
	rofl::logging::info<< "[xdpd][plugin][xmp] rcvd message:" << std::endl << msg;

	if (not msg.get_xmpies().has_ie_command()) {
		rofl::logging::error << "[xdpd][plugin][xmp] rcvd xmp request without -COMMAND- IE, dropping message." << std::endl;
		return;
	}

	switch (msg.get_xmpies().get_ie_command().get_command()) {
	case XMPIEMCT_PORT_ATTACH: {
		handle_port_attach(msg);
	} break;
	case XMPIEMCT_PORT_DETACH: {
		handle_port_detach(msg);
	} break;
	case XMPIEMCT_PORT_ENABLE: {
		handle_port_enable(msg);
	} break;
	case XMPIEMCT_PORT_DISABLE: {
		handle_port_disable(msg);
	} break;
	case XMPIEMCT_PORT_LIST: {
		handle_port_list(socket, msg);
	} break;
	case XMPIEMCT_NONE:
	default: {
		rofl::logging::error << "[xdpd][plugin][xmp] rcvd xmp request with unknown command:"
				<< (int)msg.get_xmpies().get_ie_command().get_command() << ", dropping message." << std::endl;
		return;
	};
	}
}


void
xmp::handle_port_attach(
		cxmpmsg& msg)
{
	std::string portname;
	uint64_t dpid = 0;

	try {
		if (not msg.get_xmpies().has_ie_portname()) {
			rofl::logging::error << "[xdpd][plugin][xmp] rcvd xmp Port-Attach request without -PORTNAME- IE, dropping message." << std::endl;
			return;
		}

		if (not msg.get_xmpies().has_ie_dpid()) {
			rofl::logging::error << "[xdpd][plugin][xmp] rcvd xmp Port-Attach request without -DPID- IE, dropping message." << std::endl;
			return;
		}

		portname = msg.get_xmpies().get_ie_portname().get_portname();
		dpid = msg.get_xmpies().get_ie_dpid().get_dpid();

		unsigned int of_port_num = 0;
		port_manager::attach_port_to_switch(dpid, portname, &of_port_num);
		rofl::logging::error << "[xdpd][plugin][xmp] attached port:" << portname
				<< " to dpid:" << (unsigned long long)dpid << " "
				<< " port-no:" << of_port_num << std::endl;

	} catch(eOfSmDoesNotExist& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] attaching port:" << portname
				<< " to dpid:" << (unsigned long long)dpid << " failed, LSI does not exist" << std::endl;

	} catch(ePmInvalidPort& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] attaching port:" << portname
				<< " to dpid:" << (unsigned long long)dpid << " failed (ePmInvalidPort)" << std::endl;

	} catch(ePmUnknownError& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] attaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed (ePmUnknownError)" << std::endl;

	} catch(eOfSmGeneralError& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] attaching port:" << portname
				<< " to dpid:" << (unsigned long long)dpid << " failed (eOfSmGeneralError)" << std::endl;

	} catch (...) {
		rofl::logging::error << "[xdpd][plugin][xmp] attaching port:" << portname
				<< " to dpid:" << (unsigned long long)dpid << " failed" << std::endl;

	}
}


void
xmp::handle_port_detach(
		cxmpmsg& msg)
{
	std::string portname;
	uint64_t dpid = 0;

	try {
		if (not msg.get_xmpies().has_ie_portname()) {
			rofl::logging::error << "[xdpd][plugin][xmp] rcvd xmp Port-Detach request without -PORTNAME- IE, dropping message." << std::endl;
			return;
		}

		if (not msg.get_xmpies().has_ie_dpid()) {
			rofl::logging::error << "[xdpd][plugin][xmp] rcvd xmp Port-Detach request without -DPID- IE, dropping message." << std::endl;
			return;
		}

		portname = msg.get_xmpies().get_ie_portname().get_portname();
		dpid = msg.get_xmpies().get_ie_dpid().get_dpid();

		port_manager::detach_port_from_switch(dpid, portname);
		rofl::logging::error << "[xdpd][plugin][xmp] detached port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << std::endl;

	} catch(eOfSmDoesNotExist& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] detaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed, LSI does not exist (eOfSmDoesNotExist)" << std::endl;

	} catch(ePmInvalidPort& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] detaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed, port does not exist (ePmInvalidPort)" << std::endl;

	} catch(ePmPortNotAttachedError& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] detaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed, port does not exist (ePmPortNotAttachedError)" << std::endl;

	} catch(ePmUnknownError& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] detaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed, port does not exist (ePmUnknownError)" << std::endl;

	} catch(eOfSmGeneralError& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] detaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed (eOfSmGeneralError)" << std::endl;

	} catch (...) {
		rofl::logging::error << "[xdpd][plugin][xmp] detaching port:" << portname
				<< " from dpid:" << (unsigned long long)dpid << " failed." << std::endl;

	}
}


void
xmp::handle_port_enable(
		cxmpmsg& msg)
{
	std::string portname;

	try {
		if (not msg.get_xmpies().has_ie_portname()) {
			rofl::logging::error << "[xdpd][plugin][xmp] rcvd xmp Port-Bring-Up request without -PORTNAME- IE, dropping message." << std::endl;
			return;
		}

		portname = msg.get_xmpies().get_ie_portname().get_portname();

		port_manager::bring_up(portname);
		rofl::logging::error << "[xdpd][plugin][xmp] brought port:" << portname <<" up"<< std::endl;

	} catch(ePmInvalidPort& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] bringing port:" << portname << " up failed (ePmInvalidPort)" << std::endl;

	} catch(ePmUnknownError& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] bringing port:" << portname << " up failed (ePmUnknownError)" << std::endl;

	} catch(eOfSmGeneralError& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] bringing port:" << portname << " up failed (eOfSmGeneralError)" << std::endl;

	} catch (...) {
		rofl::logging::error << "[xdpd][plugin][xmp] bringing port:" << portname << " up failed" << std::endl;

	}
}


void
xmp::handle_port_disable(
		cxmpmsg& msg)
{
	std::string portname;

	try {
		if (not msg.get_xmpies().has_ie_portname()) {
			rofl::logging::error << "[xdpd][plugin][xmp] rcvd xmp Port-Bring-Down request without -PORTNAME- IE, dropping message." << std::endl;
			return;
		}

		portname = msg.get_xmpies().get_ie_portname().get_portname();

		port_manager::bring_down(portname);
		rofl::logging::error << "[xdpd][plugin][xmp] brought port:" << portname <<" down"<< std::endl;

	} catch(ePmInvalidPort& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] bringing port:" << portname << " down failed (ePmInvalidPort)" << std::endl;

	} catch(ePmUnknownError& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] bringing port:" << portname << " down failed (ePmUnknownError)" << std::endl;

	} catch(eOfSmGeneralError& e) {
		rofl::logging::error << "[xdpd][plugin][xmp] bringing port:" << portname << " down failed (eOfSmGeneralError)" << std::endl;

	} catch (...) {
		rofl::logging::error << "[xdpd][plugin][xmp] bringing port:" << portname << " down failed" << std::endl;

	}
}


void
xmp::handle_port_list(csocket& socket, cxmpmsg& msg)
{
	rofl::logging::info<< "[xdpd][plugin][xmp] " << __PRETTY_FUNCTION__ << std::endl;
	rofl::logging::debug << "class socket: " << socket << std::endl;

	bool query_all_dp = true;
	uint64_t dpid = 0;

	if (msg.get_xmpies().has_ie_dpid()) {
		query_all_dp = false;
		dpid = msg.get_xmpies().get_ie_dpid().get_dpid();
		rofl::logging::debug << "[xdpd][plugin][xmp] only ports of dpid=" << dpid << std::endl;
	}

	cxmpmsg reply(XMP_VERSION, XMPT_REPLY_MULTIPART);
	reply.set_xid(msg.get_xid());

	// get all ports
	std::list<std::string> all_ports = port_manager::list_available_port_names();

	for (std::list<std::string>::const_iterator iter = all_ports.begin(); iter != all_ports.end(); ++iter) {
		rofl::logging::debug << "[xdpd][plugin][xmp] check port " << *iter << std::endl;
		port_snapshot snapshot;
		try {
			port_manager::get_port_info((*iter), snapshot);
		} catch(ePmInvalidPort &e) {
			// skip if port was removed in this short time
			rofl::logging::error << "[xdpd][plugin][xmp] failed to retrieve snapshot of port" << *iter << std::endl;
			continue;
		}

		// only ports attached to dpid?
		if (not query_all_dp && snapshot.attached_sw_dpid != dpid) {
			rofl::logging::debug << "[xdpd][plugin][xmp] skip port " << *iter << std::endl;
			continue;
		}

		reply.get_xmpies().set_ie_multipart().push_back(new cxmpie_portname(snapshot.name));
	}

	rofl::logging::debug << "[xdpd][plugin][xmp] sending: " << reply;

	cmemory *mem = new cmemory(reply.length());
	reply.pack(mem->somem(), mem->memlen());
	socket.send(mem);
}
