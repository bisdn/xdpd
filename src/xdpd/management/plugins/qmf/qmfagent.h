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
#include <qmf/posix/EventNotifier.h>
#else
#include <qmf/EventNotifier.h>
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
	virtual void init(void);

	virtual std::vector<rofl::coption> get_options(void){
		std::vector<rofl::coption> vec;

		//Add -q and -x arguments
		vec.push_back(coption(true, REQUIRED_ARGUMENT, 'q', QMF_BROKER_URL_OPT, "qmf broker address", std::string("127.0.0.1")));
		
		vec.push_back(coption(true, REQUIRED_ARGUMENT, 'x', QMF_XDPD_ID_OPT, "qmf xdpd ID", std::string("xdpd-0")));

		vec.push_back(coption(true, REQUIRED_ARGUMENT, 'a', QMF_SSL_CA_FILE_OPT, "QMF ssl-ca-file", std::string("./ca.pem")));
		vec.push_back(coption(true, REQUIRED_ARGUMENT, 'r', QMF_SSL_CERT_OPT, "QMF ssl-certificate (PEM encoded)", std::string("./cert.pem")));
		vec.push_back(coption(true, REQUIRED_ARGUMENT, 'k', QMF_SSL_PRIVATEKEY_OPT, "QMF ssl-private-key (PEM encoded)", std::string("./key.pem")));
		vec.push_back(coption(true, REQUIRED_ARGUMENT, 'w', QMF_SSL_PSWDFILE_OPT, "QMF ssl-password-file", std::string("./password.txt")));
		vec.push_back(coption(true, REQUIRED_ARGUMENT, 'm', QMF_SSL_VERIFY_MODE, "QMF ssl-verify-mode (PEER|NONE)", std::string("NONE")));
		vec.push_back(coption(true, REQUIRED_ARGUMENT, 'y', QMF_SSL_VERIFY_DEPTH, "QMF ssl-verify-depth", std::string("1")));
		vec.push_back(coption(true, REQUIRED_ARGUMENT, 'i', QMF_SSL_CIPHERS, "QMF ssl-ciphers", std::string("EECDH+ECDSA+AESGCM EECDH+aRSA+AESGCM EECDH+ECDSA+SHA256 EECDH+aRSA+RC4 EDH+aRSA EECDH RC4 !aNULL !eNULL !LOW !3DES !MD5 !EXP !PSK !SRP !DSS")));

		return vec;
	};

	virtual std::string get_name(){
		return std::string("qmf_agent");
	}

protected:

	/**
	 *
	 */
	virtual void
	handle_timeout(int opaque, void *data = (void*)0);

	/**
	 *
	 */
	virtual void
	handle_revent(int fd);

private:

	static const std::string QMF_BROKER_URL_OPT;
	static const std::string QMF_XDPD_ID_OPT;
	static const std::string QMF_SSL_CA_FILE_OPT;
	static const std::string QMF_SSL_CERT_OPT;
	static const std::string QMF_SSL_PRIVATEKEY_OPT;
	static const std::string QMF_SSL_PSWDFILE_OPT;
	static const std::string QMF_SSL_VERIFY_MODE;
	static const std::string QMF_SSL_VERIFY_DEPTH;
	static const std::string QMF_SSL_CIPHERS;

	std::string s_cafile;
	std::string s_certificate;
	std::string s_private_key;
	std::string s_pswdfile;
	std::string s_verify_mode;
	std::string s_verify_depth;
	std::string s_ciphers;

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
