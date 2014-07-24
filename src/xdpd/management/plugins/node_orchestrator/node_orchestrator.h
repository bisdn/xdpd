#ifndef NODE_ORCHESTRATOR_PLUGIN_H_
#define NODE_ORCHESTRATOR_PLUGIN_H_	1

#pragma once

#include "../../plugin_manager.h"
#include "../../port_manager.h"
#include "../../pex_manager.h"
#include "../../switch_manager.h"
#include "../../system_manager.h"

#include <rofl/common/logging.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include "../../../openflow/openflow_switch.h"
#include "LSI.h"
#include "orchestrator_constants.h"
#include "message_handler.h"

#include <list>
#include <map>

using namespace std;

/**
* @file node_orchestrator.h
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief Plugin that receives commands from the node orchestrator
*/

class openflow_switch;

namespace xdpd {

class eConfParamNotFound: public rofl::RoflException {};

/**
* @brief Plugin that receives command from the node orchestrator
* @ingroup cmm_mgmt_plugins
*/
class NodeOrchestrator :
	public plugin,
	public rofl::ciosrv,
	public rofl::csocket_owner
{
	rofl::csocket*			socket;			// listening socket
	rofl::cparams			socket_params;
	
friend class MessageHandler;
	
public:

	NodeOrchestrator();

	virtual ~NodeOrchestrator();

	virtual void init(void);
	
	virtual vector<rofl::coption> get_options(void);

	virtual std::string get_name(void)
	{
		return std::string(PLUGIN_NAME);
	};

protected:
	/*
	*	Methods implementing commands from the orchestrator
	*/
	
	static LSI createLSI(list<string> phyPorts, string controllerAddress, string controllerPort);
	static void destroyLSI(uint64_t dpid);
	
	static unsigned int attachPhyPort(uint64_t dpid, string port);
	
	static unsigned int createNfPort(uint64_t dpid, string NfName, string NfPortName, PexType type);
	static bool destroyNfPort(uint64_t dpid, string NfPortName, bool detach = true);

	static pair<unsigned int, unsigned int> createVirtualLink(uint64_t dpid_a,uint64_t dpid_b);
	
	//Used both the detach a physical port and to destroy a virtual link
	static bool detachPort(uint64_t dpid, uint64_t portID, bool vlink);
	static bool detachPort(uint64_t dpid, string port, bool vlink);
	
	static list<string> discoverPhyPorts();
	
	static uint64_t nextDpid;

protected:

	/*
	 * overloaded from csocket_owner
	 */

	virtual void handle_listen(rofl::csocket& socket, int newsd);

	virtual void handle_accepted(rofl::csocket& socket);
	
	virtual void handle_write(rofl::csocket& socket);

	virtual void handle_read(rofl::csocket& socket);

	virtual void handle_accept_refused(rofl::csocket& socket) {}
	virtual void handle_connected(rofl::csocket& socket) {}
	virtual void handle_connect_refused(rofl::csocket& socket) {}
	virtual void handle_connect_failed(csocket& socket){}
	virtual void handle_closed(rofl::csocket& socket) {}
};

}// namespace xdpd 

#endif /* NODE_ORCHESTRATOR_PLUGIN_H_ */


