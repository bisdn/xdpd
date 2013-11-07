/*
 * qmfagent.cc
 *
 * Copyright (C) 2013 BISDN GmbH <andi@bisdn.de>
 *
 *
 *  Created on: 26.07.2013
 *      Author: andreas
 */

#include "qmfagent.h"

using namespace xdpd;
using namespace rofl;


qmfagent::qmfagent():qmf_package("de.bisdn.xdpd")
{
	//Do nothing
}
		

void qmfagent::init(int argc, char** argv)
{
	cunixenv env_parser(argc, argv);
 
	//Add additional arguments
	env_parser.add_option(
		coption(true, REQUIRED_ARGUMENT, 'q', "qmfaddr", "qmf broker address",
		std::string("127.0.0.1")));

	env_parser.add_option(
		coption(true, REQUIRED_ARGUMENT, 'x', "qmfXdpdID", "qmf xdpd ID",
		std::string("xdpd-0")));

	//Parse
	env_parser.parse_args();
	
	//Recover
	broker_url = env_parser.get_arg('q');
	xdpd_id = env_parser.get_arg('x');

	connection = qpid::messaging::Connection(broker_url, "{reconnect:True}");
	connection.open();

	session = qmf::AgentSession(connection, "{interval:10}");
	session.setVendor("bisdn.de");
	session.setProduct("xdpd");
	session.open();

	notifier = qmf::posix::EventNotifier(session);

	set_qmf_schema();

	// create single qxdpd instance
	qxdpd.data = qmf::Data(sch_xdpd);
	qxdpd.data.setProperty("xdpdID", xdpd_id);
	std::stringstream name("xdpd");
	qxdpd.addr = session.addData(qxdpd.data, name.str());

	register_filedesc_r(notifier.getHandle());
}



qmfagent::~qmfagent()
{
	session.close();
	connection.close();
}



void
qmfagent::handle_timeout(int opaque)
{
	switch (opaque) {
	default: {

	} break;
	}
}



void
qmfagent::handle_revent(int fd)
{
	qmf::AgentEvent event;
	while (session.nextEvent(event, qpid::messaging::Duration::IMMEDIATE)) {
		switch (event.getType()) {
		case qmf::AGENT_METHOD: {
			bool running = method(event);
			(void)running;
		} break;
		default: {
			// do nothing
		} break;
		}
	}
}






void
qmfagent::set_qmf_schema()
{
	// exception
	sch_exception = qmf::Schema(qmf::SCHEMA_TYPE_DATA, qmf_package, "exception");
	sch_exception.addProperty(qmf::SchemaProperty("whatHappened", 	qmf::SCHEMA_DATA_STRING));
	sch_exception.addProperty(qmf::SchemaProperty("howBad", 		qmf::SCHEMA_DATA_INT));
	sch_exception.addProperty(qmf::SchemaProperty("details", 		qmf::SCHEMA_DATA_MAP));

    // xdpd
    sch_xdpd = qmf::Schema(qmf::SCHEMA_TYPE_DATA, qmf_package, "xdpd");
	sch_xdpd.addProperty(qmf::SchemaProperty("xdpdID", 	qmf::SCHEMA_DATA_STRING));

    qmf::SchemaMethod lsiCreateMethod("lsiCreate", "{desc:'add LSI'}");
    lsiCreateMethod.addArgument(qmf::SchemaProperty("dpid", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("dpname", 	qmf::SCHEMA_DATA_STRING, 	"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ofversion",qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ntables", 	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ctlaf",	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ctladdr", 	qmf::SCHEMA_DATA_STRING, 	"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ctlport", 	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("reconnect",qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    sch_xdpd.addMethod(lsiCreateMethod);

    qmf::SchemaMethod lsiDestroyMethod("lsiDestroy", "{desc:'destroy LSI'}");
    lsiDestroyMethod.addArgument(qmf::SchemaProperty("dpid", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    sch_xdpd.addMethod(lsiDestroyMethod);

    qmf::SchemaMethod lsiCreateVirtualLinkMethod("lsiCreateVirtualLink", "{desc:'create a virtual link between two LSIs'}");
    lsiCreateVirtualLinkMethod.addArgument(qmf::SchemaProperty("dpid1", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    lsiCreateVirtualLinkMethod.addArgument(qmf::SchemaProperty("dpid2", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    lsiCreateVirtualLinkMethod.addArgument(qmf::SchemaProperty("devname1",	qmf::SCHEMA_DATA_STRING, 	"{dir:OUT}"));
    lsiCreateVirtualLinkMethod.addArgument(qmf::SchemaProperty("devname2",	qmf::SCHEMA_DATA_STRING, 	"{dir:OUT}"));
    sch_xdpd.addMethod(lsiCreateVirtualLinkMethod);



    // lsi
    sch_lsi = qmf::Schema(qmf::SCHEMA_TYPE_DATA, qmf_package, "lsi");
    sch_lsi.addProperty(qmf::SchemaProperty("xdpdID", 	qmf::SCHEMA_DATA_STRING));
    sch_lsi.addProperty(qmf::SchemaProperty("dpid", qmf::SCHEMA_DATA_INT));
    sch_lsi.addProperty(qmf::SchemaProperty("dpname", 	qmf::SCHEMA_DATA_STRING));
    sch_lsi.addProperty(qmf::SchemaProperty("ofversion",qmf::SCHEMA_DATA_INT));
    sch_lsi.addProperty(qmf::SchemaProperty("ntables", 	qmf::SCHEMA_DATA_INT));
    sch_lsi.addProperty(qmf::SchemaProperty("ctlaf",	qmf::SCHEMA_DATA_INT));
    sch_lsi.addProperty(qmf::SchemaProperty("ctladdr", 	qmf::SCHEMA_DATA_STRING));
    sch_lsi.addProperty(qmf::SchemaProperty("ctlport", 	qmf::SCHEMA_DATA_INT));
    sch_lsi.addProperty(qmf::SchemaProperty("reconnect",qmf::SCHEMA_DATA_INT));

    qmf::SchemaMethod portAttachMethod("portAttach", "{desc:'attach port'}");
    portAttachMethod.addArgument(qmf::SchemaProperty("dpid", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    portAttachMethod.addArgument(qmf::SchemaProperty("devname",	qmf::SCHEMA_DATA_STRING, 	"{dir:INOUT}"));
    portAttachMethod.addArgument(qmf::SchemaProperty("portno", 	qmf::SCHEMA_DATA_INT, 		"{dir:OUT}"));
    sch_lsi.addMethod(portAttachMethod);

    qmf::SchemaMethod portDetachMethod("portDetach", "{desc:'detach port'}");
    portDetachMethod.addArgument(qmf::SchemaProperty("dpid", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    portDetachMethod.addArgument(qmf::SchemaProperty("devname",	qmf::SCHEMA_DATA_STRING, 	"{dir:IN}"));
    sch_lsi.addMethod(portDetachMethod);

    qmf::SchemaMethod ctlConnectMethod("ctlConnect", "{desc:'connect to controller'}");
    ctlConnectMethod.addArgument(qmf::SchemaProperty("dpid", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    ctlConnectMethod.addArgument(qmf::SchemaProperty("ctlaf",	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    ctlConnectMethod.addArgument(qmf::SchemaProperty("ctladdr",	qmf::SCHEMA_DATA_STRING, 	"{dir:IN}"));
    ctlConnectMethod.addArgument(qmf::SchemaProperty("ctlport",	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    sch_lsi.addMethod(ctlConnectMethod);

    qmf::SchemaMethod ctlDisconnectMethod("ctlDisconnect", "{desc:'disconnect from controller'}");
    ctlDisconnectMethod.addArgument(qmf::SchemaProperty("dpid", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    ctlDisconnectMethod.addArgument(qmf::SchemaProperty("ctlaf",	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    ctlDisconnectMethod.addArgument(qmf::SchemaProperty("ctladdr", 	qmf::SCHEMA_DATA_STRING, 	"{dir:IN}"));
    ctlDisconnectMethod.addArgument(qmf::SchemaProperty("ctlport", 	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    sch_lsi.addMethod(ctlDisconnectMethod);



    session.registerSchema(sch_exception);
    session.registerSchema(sch_xdpd);
    session.registerSchema(sch_lsi);
}



bool
qmfagent::method(qmf::AgentEvent& event)
{
	std::string const& name = event.getMethodName();
	(void)name;

	try {

		if (name == "lsiCreate") {
			return methodLsiCreate(event);
		}
		else if (name == "lsiDestroy") {
			return methodLsiDestroy(event);
		}
		else if (name == "portAttach") {
			return methodPortAttach(event);
		}
		else if (name == "portDetach") {
			return methodPortDetach(event);
		}
		else if (name == "ctlConnect") {
			return methodCtlConnect(event);
		}
		else if (name == "ctlDisconnect") {
			return methodCtlDisconnect(event);
		}
		else if (name == "lsiCreateVirtualLink") {
			return methodLsiCreateVirtualLink(event);
		}

		else {
			session.raiseException(event, "command not found");
		}

	} catch (std::exception const& e) {

		std::cerr << "EXCEPTION: " << e.what() << std::endl;
		session.raiseException(event, e.what());
		throw;
	}

	return true;
}



bool
qmfagent::methodLsiCreate(qmf::AgentEvent& event)
{
	try {
		int of_version			= event.getArguments()["ofversion"].asInt32();
		uint64_t dpid 			= event.getArguments()["dpid"].asUint64();
		std::string dpname 		= event.getArguments()["dpname"].asString();
		unsigned int ntables	= event.getArguments()["ntables"].asUint32();
		int ctlaf				= event.getArguments()["ctlaf"].asInt32();
		std::string ctladdr 	= event.getArguments()["ctladdr"].asString();
		unsigned short ctlport	= event.getArguments()["ctlport"].asUint16();
		unsigned int reconnect  = event.getArguments()["reconnect"].asUint32();

		event.addReturnArgument("dpid", dpid);

		int ma_list[256] = { 0 };
		rofl::caddress caddr(ctlaf, ctladdr.c_str(), ctlport);
		xdpd::switch_manager::create_switch((of_version_t)of_version, dpid, dpname, ntables, ma_list, reconnect, caddr);

		// create QMF LSI object
		qLSIs[dpid].data = qmf::Data(sch_lsi);
		qLSIs[dpid].data.setProperty("xdpdID", xdpd_id);
		qLSIs[dpid].data.setProperty("dpid", dpid);
		qLSIs[dpid].data.setProperty("dpname", dpname);
		qLSIs[dpid].data.setProperty("ntables", ntables);
		qLSIs[dpid].data.setProperty("ctlaf", ctlaf);
		qLSIs[dpid].data.setProperty("ctladdr", ctladdr);
		qLSIs[dpid].data.setProperty("ctlport", ctlport);
		qLSIs[dpid].data.setProperty("reconnect", reconnect);

		std::stringstream name("lsi-"); name << xdpd_id << "-" << dpid;
		qLSIs[dpid].addr = session.addData(qLSIs[dpid].data, name.str());

		session.methodSuccess(event);

		return true;

	} catch (xdpd::eOfSmExists& e) {
		session.raiseException(event, "LSI creation failed: already exists");

	} catch (xdpd::eOfSmErrorOnCreation& e) {
		session.raiseException(event, "LSI creation failed: internal error");

	} catch (xdpd::eOfSmVersionNotSupported& e) {
		session.raiseException(event, "LSI creation failed: unsupported OpenFlow version");

	}
	return false;
}



bool
qmfagent::methodLsiDestroy(qmf::AgentEvent& event)
{
	try {
		uint64_t dpid 			= event.getArguments()["dpid"].asUint64();

		event.addReturnArgument("dpid", dpid);

		xdpd::switch_manager::destroy_switch(dpid);

		// destroy QMF LSI object
		session.delData(qLSIs[dpid].addr);
		qLSIs.erase(dpid);

		session.methodSuccess(event);

		return true;

	} catch (xdpd::eOfSmDoesNotExist& e) {
		session.raiseException(event, "LSI creation failed: dpid does not exist");

	}
	return false;
}




bool
qmfagent::methodPortAttach(qmf::AgentEvent& event)
{
	try {
		uint64_t dpid 			= event.getArguments()["dpid"].asUint64();
		std::string devname		= event.getArguments()["devname"].asString();

		uint32_t portno = 0;
		port_manager::attach_port_to_switch(dpid, devname, &portno);
		port_manager::enable_port(devname);

		// TODO: create QMF port object (if this is deemed useful one day ...)
		event.addReturnArgument("dpid", dpid);
		event.addReturnArgument("portno", portno);
		event.addReturnArgument("devname", devname);
		session.methodSuccess(event);

		return true;

	} catch (xdpd::eOfSmDoesNotExist& e) {
		session.raiseException(event, "port attachment failed: LSI does not exist");

	} catch (xdpd::eOfSmErrorOnCreation& e) {
		session.raiseException(event, "port attachment failed: physical port does not exist");

	} catch (xdpd::eOfSmGeneralError& e) {
		session.raiseException(event, "port attachment failed: internal error");

	} catch (...) {
		session.raiseException(event, "port attachment failed: internal error");
	}
	return false;
}



bool
qmfagent::methodPortDetach(qmf::AgentEvent& event)
{
	try {
		uint64_t dpid 			= event.getArguments()["dpid"].asUint64();
		std::string devname		= event.getArguments()["devname"].asString();

		xdpd::port_manager::disable_port(devname);
		xdpd::port_manager::detach_port_from_switch(dpid, devname);

		// TODO: destroy QMF port object (if this is deemed useful one day ...)
		event.addReturnArgument("dpid", dpid);
		session.methodSuccess(event);

		return true;

	} catch (xdpd::eOfSmDoesNotExist& e) {
		session.raiseException(event, "port detachment failed: LSI does not exist");

	} catch (xdpd::eOfSmErrorOnCreation& e) {
		session.raiseException(event, "port detachment failed: physical port does not exist");

	} catch (xdpd::eOfSmGeneralError& e) {
		session.raiseException(event, "port detachment failed: internal error");

	} catch (...) {
		session.raiseException(event, "port attachment failed: internal error");

	}
	return false;
}




bool
qmfagent::methodCtlConnect(qmf::AgentEvent& event)
{
	try {
		uint64_t dpid 			= event.getArguments()["dpid"].asUint64();
		int ctlaf				= event.getArguments()["ctlaf"].asInt32();
		std::string ctladdr 	= event.getArguments()["ctladdr"].asString();
		unsigned short ctlport	= event.getArguments()["ctlport"].asUint16();

		rofl::caddress controller_address(ctlaf, ctladdr.c_str(), ctlport);

		xdpd::switch_manager::rpc_connect_to_ctl(dpid, controller_address);

		// TODO: create QMF port object (if this is deemed useful one day ...)
		event.addReturnArgument("dpid", dpid);
		session.methodSuccess(event);

		return true;

	} catch (xdpd::eOfSmDoesNotExist& e) {
		session.raiseException(event, "controller connect failed: LSI does not exist");

	} catch (xdpd::eOfSmGeneralError& e) {
		session.raiseException(event, "controller connect failed: internal error");

	} catch (...) {
		session.raiseException(event, "controller connect failed: internal error");
	}
	return false;
}



bool
qmfagent::methodCtlDisconnect(qmf::AgentEvent& event)
{
	try {
		uint64_t dpid 			= event.getArguments()["dpid"].asUint64();
		int ctlaf				= event.getArguments()["ctlaf"].asInt32();
		std::string ctladdr 	= event.getArguments()["ctladdr"].asString();
		unsigned short ctlport	= event.getArguments()["ctlport"].asUint16();

		rofl::caddress controller_address(ctlaf, ctladdr.c_str(), ctlport);

		xdpd::switch_manager::rpc_disconnect_from_ctl(dpid, controller_address);

		// TODO: create QMF port object (if this is deemed useful one day ...)
		event.addReturnArgument("dpid", dpid);
		session.methodSuccess(event);

		return true;

	} catch (xdpd::eOfSmDoesNotExist& e) {
		session.raiseException(event, "controller disconnect failed: LSI does not exist");

	} catch (xdpd::eOfSmGeneralError& e) {
		session.raiseException(event, "controller disconnect failed: internal error");

	} catch (...) {
		session.raiseException(event, "controller disconnect failed: internal error");
	}
	return false;
}




bool
qmfagent::methodLsiCreateVirtualLink(qmf::AgentEvent& event)
{
	try {
		uint64_t dpid1 			= event.getArguments()["dpid1"].asUint64();
		uint64_t dpid2 			= event.getArguments()["dpid2"].asUint64();
		std::string devname1;
		std::string devname2;

		xdpd::port_manager::connect_switches(dpid1, devname1, dpid2, devname2);

		event.addReturnArgument("devname1", devname1);
		event.addReturnArgument("devname2", devname2);

		session.methodSuccess(event);

		return true;

	} catch (xdpd::eOfSmDoesNotExist& e) {
		session.raiseException(event, "VirtualLink creation failed: dpid does not exist");

	} catch (xdpd::eOfSmGeneralError& e) {
		session.raiseException(event, "VirtualLink creation failed: internal error");

	}
	return false;
}


