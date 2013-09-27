/*
 * qmfagent.h
 *
 * Copyright (C) 2013 BISDN GmbH <andi@bisdn.de>
 *
 *  Created on: 26.07.2013
 *      Author: andreas
 */

#ifndef QMFAGENT_H_
#define QMFAGENT_H_ 1

#include <inttypes.h>
#include <map>
#include <string>
#include <ostream>
#include <exception>

#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Duration.h>
#include <qmf/AgentSession.h>
#include <qmf/AgentEvent.h>
#include <qmf/Schema.h>
#include <qmf/SchemaProperty.h>
#include <qmf/SchemaMethod.h>
#include <qmf/Data.h>
#include <qmf/DataAddr.h>
#include <qmf/EventNotifier.h>
#include <qpid/types/Variant.h>

#include <rofl/common/ciosrv.h>
#include <rofl/common/crofbase.h>

#include "../../switch_manager.h"
#include "../../port_manager.h"
#include "../../plugin_manager.h"

namespace xdpd 
{

class eQmfAgentBase 		: public std::exception {};
class eQmfAgentInval		: public eQmfAgentBase {};
class eQmfAgentInvalSubcmd	: public eQmfAgentInval {};

class qmfagent :
		public rofl::ciosrv,
		public plugin
{
public:

	qmfagent();
	~qmfagent();

private:

	std::string						broker_url;

	qpid::messaging::Connection 	connection;
	qmf::AgentSession 				session;
	qmf::posix::EventNotifier		notifier;
	std::string						qmf_package;
	qmf::Schema						sch_exception;
	qmf::Schema						sch_xdpd;
	qmf::Schema						sch_lsi;

	struct qmf_data_t {
		qmf::Data 					data;
		qmf::DataAddr 				addr;
	};

	struct qmf_data_t				qxdpd;

	std::map<uint64_t, struct qmf_data_t>	qLSIs;

public:

	/**
	 *
	 */
	virtual void init(int argc, char** argv);

	virtual std::string get_name(){
		return std::string("qmf_agent");
	}

protected:

	/**
	 *
	 */
	virtual void
	handle_timeout(int opaque);

	/**
	 *
	 */
	virtual void
	handle_revent(int fd);

private:

	/**
	 *
	 */
	void
	set_qmf_schema();

	/**
	 *
	 */
	bool
	method(qmf::AgentEvent& event);

	/**
	 *
	 */
	bool
	methodLsiCreate(qmf::AgentEvent& event);

	/**
	 *
	 */
	bool
	methodLsiDestroy(qmf::AgentEvent& event);

	/**
	 *
	 */
	bool
	methodPortAttach(qmf::AgentEvent& event);

	/**
	 *
	 */
	bool
	methodPortDetach(qmf::AgentEvent& event);

	/**
	 *
	 */
	void
	create_switch();

	/**
	 *
	 */
	void
	destroy_switch();
};

}; // end of namespace

#endif /* QMFAGENT_H_ */
