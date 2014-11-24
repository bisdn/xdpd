/*
 * cxmpclient.cc
 *
 *  Created on: 12.01.2014
 *      Author: andreas
 */

#include "cxmpclient.h"

using namespace xdpd::mgmt::protocol;

cxmpclient::cxmpclient() :
		socket(NULL),
		dest(AF_INET, "127.0.0.1", 8444),
		mem(NULL),
		fragment(NULL),
		msg_bytes_read(0),
		observer(NULL),
		auto_exit(true),
		exit_timeout(5)
{
	socket = rofl::csocket::csocket_factory(rofl::csocket::SOCKET_TYPE_PLAIN, this);

	socket_params = rofl::csocket::get_default_params(rofl::csocket::SOCKET_TYPE_PLAIN);

	socket_params.set_param(rofl::csocket::PARAM_KEY_REMOTE_HOSTNAME).set_string("127.0.0.1");
	socket_params.set_param(rofl::csocket::PARAM_KEY_REMOTE_PORT).set_string("8444");
	socket_params.set_param(rofl::csocket::PARAM_KEY_DOMAIN).set_string("inet");
	socket_params.set_param(rofl::csocket::PARAM_KEY_TYPE).set_string("stream");
	socket_params.set_param(rofl::csocket::PARAM_KEY_PROTOCOL).set_string("tcp");

	socket->connect(socket_params);
}

cxmpclient::~cxmpclient()
{
	delete socket;

	if (mem) {
		delete mem;
	}
}

void
cxmpclient::handle_connected(rofl::csocket& socket)
{
	rofl::logging::debug << __PRETTY_FUNCTION__ << std::endl;
	assert(this->socket == &socket);

	if (mem) {
		notify(WANT_SEND);
	}

	if (auto_exit) {
		register_timer(TIMER_XMPCLNT_EXIT, exit_timeout);
	}
}

void
cxmpclient::handle_connect_refused(rofl::csocket& socket)
{
	rofl::logging::debug << __PRETTY_FUNCTION__ << std::endl;
	exit(-1);
}

void
cxmpclient::handle_connect_failed(rofl::csocket& socket)
{
	rofl::logging::debug << __PRETTY_FUNCTION__ << std::endl;
	exit(-1);
}

void
cxmpclient::handle_read(rofl::csocket& socket)
{
	rofl::logging::info << "[xmpclient]" << __PRETTY_FUNCTION__ << std::endl;

	try {

		if (NULL == fragment) {
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
				rofl::logging::warn << "[xmpclient] received message with invalid length field, closing socket." << std::endl;
				socket.close();
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
					case XMPT_REPLY:
					{
						handle_reply(msg);
					} break;
					case XMPT_ERROR:
					{
						handle_error(msg);
					} break;
					case XMPT_NOTIFICATION:
					case XMPT_REQUEST:
					default:
					{
						rofl::logging::error
								<< "[xmpclient] unexpected message rcvd"
								<< std::endl;
					}
						;
					}

					delete mem;
					return;
				}
			}
		}

	} catch (rofl::eSocketRxAgain& e) {

		// more bytes are needed, keep pointer to msg in "fragment"
		rofl::logging::debug << "[xmpclient] read again" << std::endl;

	} catch (rofl::eSysCall& e) {

		rofl::logging::warn << "[xmpclient] closing socket: " << e << std::endl;

		if (fragment) {
			delete fragment; fragment = (rofl::cmemory*)0;
		}

		// close socket, as it seems, we are out of sync
		socket.close();

	} catch (rofl::RoflException& e) {

		rofl::logging::warn << "[xmpclient] dropping invalid message: " << e << std::endl;

		if (fragment) {
			delete fragment; fragment = (rofl::cmemory*)0;
		}

		// close socket, as it seems, we are out of sync
		socket.close();
	}
}

void
cxmpclient::handle_write(rofl::csocket& socket)
{
	rofl::logging::debug << __PRETTY_FUNCTION__ << std::endl;
}

void
cxmpclient::handle_closed(rofl::csocket& socket)
{
	rofl::logging::debug << __PRETTY_FUNCTION__ << std::endl;
}

void
cxmpclient::handle_event(rofl::cevent const& ev)
{
	switch (ev.cmd) {
		case WANT_SEND:
			handle_send();
			break;
		case WANT_EXIT:
			ciosrv::stop();
			break;
		default:
			break;
	}
}

void
cxmpclient::handle_timeout(
		int opaque, void *data)
{
	rofl::logging::debug << __PRETTY_FUNCTION__ << std::endl;

	switch (opaque) {
	case TIMER_XMPCLNT_EXIT: {
		ciosrv::stop();
	} break;
	default: {

	};
	}
}

void
cxmpclient::handle_send()
{
	if(mem)	{
		socket->send(mem);
		mem = NULL;
	}
}

void
cxmpclient::send_message(cxmpmsg &msg)
{
	if(this->mem) {
		delete this->mem;
	}

	mem = new rofl::cmemory(msg.length());
	msg.pack(mem->somem(), mem->memlen());

	if (socket->is_established()) {
		notify(WANT_SEND);
	}
}

void
cxmpclient::handle_reply(cxmpmsg& msg)
{
	rofl::logging::info << "[xdpd][plugin][xmp] rcvd message:" << std::endl << msg;

	if (NULL != observer) {
		rofl::logging::info << "[xdpd][plugin][xmp] call observer:" << std::endl;
		observer->notify(msg);
	}
}

void
cxmpclient::handle_error(cxmpmsg& msg)
{
	rofl::logging::error << "[xdpd][plugin][xmp] rcvd error message:" << std::endl << msg;

	if (NULL != observer) {
		rofl::logging::info << "[xdpd][plugin][xmp] call observer:" << std::endl;
		observer->notify(msg);
	}
}

void
cxmpclient::register_observer(cxmpobserver *observer)
{
	rofl::logging::info << "[xdpd][plugin][xmp] register observer:" << observer << std::endl;
	assert(observer);
	this->observer = observer;
}

void
cxmpclient::terminate_client()
{
	notify(WANT_EXIT);
}

/* commands */

void
cxmpclient::port_list()
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);
	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_LIST);

	rofl::logging::debug << "[xmpclient] sending Port-List request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::port_list(uint64_t dpid)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);
	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_LIST);
	msg.get_xmpies().add_ie_dpid().set_dpid(dpid);

	rofl::logging::debug << "[xmpclient] sending Port-List request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::port_info()
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);
	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_INFO);

	rofl::logging::debug << "[xmpclient] sending Port-Info request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::port_attach(uint64_t dpid, std::string const& portname)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);

	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_ATTACH);
	msg.get_xmpies().add_ie_portname().set_name(portname);
	msg.get_xmpies().add_ie_dpid().set_dpid(dpid);

	rofl::logging::debug << "[xmpclient] sending Port-Attach request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::port_detach(uint64_t dpid, std::string const& portname)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);

	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_DETACH);
	msg.get_xmpies().add_ie_portname().set_name(portname);
	msg.get_xmpies().add_ie_dpid().set_dpid(dpid);

	rofl::logging::debug << "[xmpclient] sending Port-Detach request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::port_enable(std::string const& portname)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);

	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_ENABLE);
	msg.get_xmpies().add_ie_portname().set_name(portname);

	rofl::logging::debug << "[xmpclient] sending Port-Enable request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::port_disable(std::string const& portname)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);

	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_PORT_DISABLE);
	msg.get_xmpies().add_ie_portname().set_name(portname);

	rofl::logging::debug << "[xmpclient] sending Port-Disable request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::lsi_list()
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);
	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_LSI_LIST);

	rofl::logging::debug << "[xmpclient] sending Lsi-List request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::lsi_info()
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);
	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_LSI_INFO);

	rofl::logging::debug << "[xmpclient] sending Lsi-Info request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::lsi_create(uint64_t dpid, std::string const& lsi_name, const std::list<class xdpd::mgmt::protocol::controller>& controller)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);
	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_LSI_CREATE);
	msg.get_xmpies().add_ie_dpid().set_dpid(dpid);
	msg.get_xmpies().add_ie_lsiname().set_name(lsi_name);

	assert(controller.size()); // enforce at least one controller

	for (std::list<class xdpd::mgmt::protocol::controller>::const_iterator iter = controller.begin(); iter != controller.end(); ++iter) {
		msg.get_xmpies().set_ie_multipart().push_back(new cxmpie_controller(*iter));
	}

	rofl::logging::debug << "[xmpclient] sending Lsi-Create request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::lsi_destroy(const uint64_t dpid)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);
	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_LSI_DESTROY);
	msg.get_xmpies().add_ie_dpid().set_dpid(dpid);

	rofl::logging::debug << "[xmpclient] sending Lsi-Destroy request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::lsi_connect_to_controller(uint64_t dpid, const std::list<class xdpd::mgmt::protocol::controller>& controller)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);
	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_LSI_CONTROLLER_CONNECT);
	msg.get_xmpies().add_ie_dpid().set_dpid(dpid);

	assert(controller.size());  // enforce at least one controller

	for (std::list<class xdpd::mgmt::protocol::controller>::const_iterator iter = controller.begin(); iter != controller.end(); ++iter) {
		msg.get_xmpies().set_ie_multipart().push_back(new cxmpie_controller(*iter));
	}

	rofl::logging::debug << "[xmpclient] sending Lsi-Connect-To-Controller request:" << std::endl << msg;
	send_message(msg);
}

void
cxmpclient::lsi_cross_connect(const uint64_t dpid1, const uint64_t port_no1, const uint64_t dpid2, const uint64_t port_no2)
{
	cxmpmsg msg(XMP_VERSION, XMPT_REQUEST);
	msg.get_xmpies().add_ie_command().set_command(XMPIEMCT_LSI_XCONNNECT);
	msg.get_xmpies().set_ie_lsixlsi().set_dpid1(dpid1);
	msg.get_xmpies().set_ie_lsixlsi().set_portno1(port_no1);
	msg.get_xmpies().set_ie_lsixlsi().set_dpid2(dpid2);
	msg.get_xmpies().set_ie_lsixlsi().set_portno2(port_no2);

	rofl::logging::debug << "[xmpclient] sending Lsi-Cross-Connect request:" << std::endl << msg;
	send_message(msg);
}
