/*
 * qmfagent.cc
 *
 * Copyright (C) 2013 BISDN GmbH <andi@bisdn.de>
 *
 *
 *  Created on: 26.07.2013
 *      Author: andreas
 */

#include <qmfagent.h>

using namespace rofl;

qmfagent* qmfagent::qmf_agent = (qmfagent*)0;


qmfagent&
qmfagent::get_instance(std::string const& broker_url)
{
	if (0 == qmfagent::qmf_agent) {
		qmfagent::qmf_agent = new qmfagent(broker_url);
	}
	return *(qmfagent::qmf_agent);
}



qmfagent::qmfagent(
		std::string const& broker_url) :
		broker_url(broker_url),
		qmf_package("de.bisdn.xdpd")
{
	connection = qpid::messaging::Connection(broker_url, "{reconnect:True}");
	connection.open();

	session = qmf::AgentSession(connection, "{interval:10}");
	session.setVendor("bisdn.de");
	session.setProduct("xdpd");
	session.open();

	notifier = qmf::posix::EventNotifier(session);

	set_qmf_schema();

	register_filedesc_r(notifier.getHandle());
}



qmfagent::qmfagent(qmfagent const& agent)
{

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

    // node
    sch_xdpd = qmf::Schema(qmf::SCHEMA_TYPE_DATA, qmf_package, "xdpd");
    sch_xdpd.addProperty(qmf::SchemaProperty("dpid", qmf::SCHEMA_DATA_INT));

    qmf::SchemaMethod lsiCreateMethod("lsiCreate", "{desc:'add LSI'}");
    lsiCreateMethod.addArgument(qmf::SchemaProperty("dpid", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("dpname", 	qmf::SCHEMA_DATA_STRING, 	"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ofversion",qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ntables", 	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ctlaf",	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ctladdr", 	qmf::SCHEMA_DATA_STRING, 	"{dir:IN}"));
    lsiCreateMethod.addArgument(qmf::SchemaProperty("ctlport", 	qmf::SCHEMA_DATA_INT, 		"{dir:IN}"));
    sch_xdpd.addMethod(lsiCreateMethod);

    qmf::SchemaMethod lsiDestroyMethod("lsiDestroy", "{desc:'destroy LSI'}");
    lsiDestroyMethod.addArgument(qmf::SchemaProperty("dpid", 	qmf::SCHEMA_DATA_INT, 		"{dir:INOUT}"));
    sch_xdpd.addMethod(lsiDestroyMethod);

    session.registerSchema(sch_exception);
    session.registerSchema(sch_xdpd);
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

		event.addReturnArgument("dpid", dpid);

		int ma_list[256] = { 0 };
		rofl::caddress caddr(ctlaf, ctladdr.c_str(), ctlport);
		rofl::switch_manager::create_switch((of_version_t)of_version, dpid, dpname, ntables, ma_list, caddr);

		session.methodSuccess(event);

		return true;

	} catch (rofl::eOfSmExists& e) {
		session.raiseException(event, "LSI creation failed: already exists");

	} catch (rofl::eOfSmErrorOnCreation& e) {
		session.raiseException(event, "LSI creation failed: internal error");

	} catch (rofl::eOfSmVersionNotSupported& e) {
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

		rofl::switch_manager::destroy_switch(dpid);

		session.methodSuccess(event);

		return true;

	} catch (rofl::eOfSmDoesNotExist& e) {
		session.raiseException(event, "LSI creation failed: dpid does not exist");

	}
	return false;
}



