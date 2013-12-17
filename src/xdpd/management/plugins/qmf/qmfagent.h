/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QMFAGENT_H_
#define QMFAGENT_H_ 

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
#ifdef WITH_QMF2_024
#include <qmf/EventNotifier.h>
#else
#include <qmf/posix/EventNotifier.h>
#endif
#include <qpid/types/Variant.h>

#include <rofl/common/ciosrv.h>
#include <rofl/common/crofbase.h>

#include "../../switch_manager.h"
#include "../../port_manager.h"
#include "../../plugin_manager.h"
#include "../../../openflow/openflow_switch.h"

/**
* @file qmfagent.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief QMF management plugin file
*/

namespace xdpd 
{

class eQmfAgentBase 		: public std::exception {};
class eQmfAgentInval		: public eQmfAgentBase {};
class eQmfAgentInvalSubcmd	: public eQmfAgentInval {};

/**
* @brief Qpid Management Framework (QMF) management plugin
*
* @description This plugin exposes xDPd's Port and Switch management APIs via a QMF broker
* @ingroup cmm_mgmt_plugins
*/
class qmfagent :
		public rofl::ciosrv,
		public plugin
{
public:

	qmfagent();
	~qmfagent();

private:

	std::string						broker_url;
	std::string						xdpd_id;

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
	bool
	methodCtlConnect(qmf::AgentEvent& event);

	/**
	 *
	 */
	bool
	methodCtlDisconnect(qmf::AgentEvent& event);

	/**
	 *
	 */
	bool
	methodLsiCreateVirtualLink(qmf::AgentEvent& event);

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
