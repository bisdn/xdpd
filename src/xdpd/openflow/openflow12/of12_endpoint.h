/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
* @file of12_endpoint.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
*
* @brief OF1.2 endpoint implementation
*/

#ifndef OF12_ENDPOINT_H
#define OF12_ENDPOINT_H 

#include <rofl/platform/unix/csyslog.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include "../openflow_switch.h"
#include "../of_endpoint.h"
#include "../../management/switch_manager.h"

using namespace rofl;

namespace xdpd {

/**
* @brief of12_endpoint is an OpenFlow 1.2 OF agent implementation
* @ingroup cmm_of
**/
class of12_endpoint : public of_endpoint {
	

public:

	//Main constructor
	of12_endpoint(
			openflow_switch* sw,
			int reconnect_start_timeout,
			caddress const& controller_addr = caddress(AF_INET, "127.0.0.1", 6633),
			caddress const& binding_addr = caddress(AF_INET, "0.0.0.0", 0)) throw (eOfSmErrorOnCreation);

	/**
	 *
	 */
	afa_result_t
	process_packet_in(
			uint8_t table_id,
			uint8_t reason,
			uint32_t in_port,
			uint32_t buffer_id,
			uint8_t* pkt_buffer,
			uint32_t buf_len,
			uint16_t total_len,
			packet_matches_t* matches);


	/**
	 *
	 */
	afa_result_t
	process_flow_removed(
			uint8_t reason,
			of1x_flow_entry *removed_flow_entry);

	/*
	* Port notifications
	*/

	virtual	afa_result_t notify_port_add(switch_port_t* port);
	
	virtual afa_result_t notify_port_delete(switch_port_t* port);

	virtual afa_result_t notify_port_status_changed(switch_port_t* port);



private:

	/* *
	 ** This section is in charge of the handling of the OF messages
	 ** comming from the cofctl(OF endpoints). These are version specific
	 ** and must be implemented by the derived class (ofXX_dphcl) 
	 **/

	/** Handle OF features request. To be overwritten by derived class.
	 *
	 * OF FEATURES.requests are handled by the crofbase base class in method
	 * crofbase::send_features_reply(). However,
	 * this method handle_features_request() may be overloaded by a derived class to get a notification
	 * upon reception of a FEATURES.request from the controlling entity.
	 * Default behaviour is to remove the packet from the heap.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack OF packet received from controlling entity.
	 */
	virtual void
	handle_features_request(crofctl& ctl, cofmsg_features_request& msg, uint8_t aux_id = 0);

	/** Handle OF get-config request. To be overwritten by derived class.
	 *
	 * Called from within crofbase::fe_down_get_config_request().
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param ctrl cofdpath instance from whom the GET-CONFIG.request was received.
	 * @pack OF GET-CONFIG.request packet received from controller
	 */
	virtual void
	handle_get_config_request(crofctl& ctl, cofmsg_get_config_request& msg, uint8_t aux_id = 0);

	/**
	 *
	 */
	virtual void
	handle_desc_stats_request(crofctl& ctl, cofmsg_desc_stats_request& msg, uint8_t aux_id = 0);


	/**
	 *
	 */
	virtual void
	handle_table_stats_request(crofctl& ctl, cofmsg_table_stats_request& msg, uint8_t aux_id = 0);


	/**
	 *
	 */
	virtual void
	handle_port_stats_request(crofctl& ctl, cofmsg_port_stats_request& msg, uint8_t aux_id = 0);


	/**
	 *
	 */
	virtual void
	handle_flow_stats_request(crofctl& ctl, cofmsg_flow_stats_request& msg, uint8_t aux_id = 0);


	/**
	 *
	 */
	virtual void
	handle_aggregate_stats_request(crofctl& ctl, cofmsg_aggr_stats_request& msg, uint8_t aux_id = 0);


	/**
	 *
	 */
	virtual void
	handle_queue_stats_request(crofctl& ctl, cofmsg_queue_stats_request& msg, uint8_t aux_id = 0);


	/**
	 *
	 */
	virtual void
	handle_group_stats_request(crofctl& ctl, cofmsg_group_stats_request& msg, uint8_t aux_id = 0);


	/**
	 *
	 */
	virtual void
	handle_group_desc_stats_request(crofctl& ctl, cofmsg_group_desc_stats_request& msg, uint8_t aux_id = 0);


	/**
	 *
	 */
	virtual void
	handle_group_features_stats_request(crofctl& ctl, cofmsg_group_features_stats_request& msg, uint8_t aux_id = 0);


	/**
	 *
	 */
	virtual void
	handle_experimenter_stats_request(crofctl& ctl, cofmsg_experimenter_stats_request& msg, uint8_t aux_id = 0);

	/** Handle OF packet-out messages. To be overwritten by derived class.
	 *
	 * Called upon reception of a PACKET-OUT.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack PACKET-OUT.message packet received from controller.
	 */
	virtual void
	handle_packet_out(crofctl& ctl, cofmsg_packet_out& msg, uint8_t aux_id = 0);

	/** Handle OF barrier request. To be overwritten by derived class.
	 *
	 * Called upon reception of a BARRIER.request from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack BARRIER.request packet received from controller.
	 */
	virtual void
	handle_barrier_request(crofctl& ctl, cofmsg_barrier_request& msg, uint8_t aux_id = 0);

	/** Handle OF flow-mod message. To be overwritten by derived class.
	 *
	 * Called upon reception of a FLOW-MOD.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack FLOW-MOD.message packet received from controller.
	 */
	virtual void
	handle_flow_mod(crofctl& ctl, cofmsg_flow_mod& msg, uint8_t aux_id = 0);

	/** Handle OF group-mod message. To be overwritten by derived class.
	 *
	 * Called upon reception of a GROUP-MOD.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack GROUP-MOD.message packet received from controller.
	 */
	virtual void
	handle_group_mod(crofctl& ctl, cofmsg_group_mod& msg, uint8_t aux_id = 0);

	/** Handle OF table-mod message. To be overwritten by derived class.
	 *
	 * Called upon reception of a TABLE-MOD.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack TABLE-MOD.message packet received from controller.
	 */
	virtual void
	handle_table_mod(crofctl& ctl, cofmsg_table_mod& msg, uint8_t aux_id = 0);

	/** Handle OF port-mod message. To be overwritten by derived class.
	 *
	 * Called upon reception of a PORT-MOD.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack PORT-MOD.message packet received from controller.
	 */
	virtual void
	handle_port_mod(crofctl& ctl, cofmsg_port_mod& msg, uint8_t aux_id = 0);

	/** Handle OF set-config message. To be overwritten by derived class.
	 *
	 * Called upon reception of a SET-CONFIG.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack SET-CONFIG.message packet received from controller.
	 */
	virtual void
	handle_set_config(crofctl& ctl, cofmsg_set_config& msg, uint8_t aux_id = 0);

	/** Handle OF queue-get-config request. To be overwritten by derived class.
	 *
	 * Called upon reception of a QUEUE-GET-CONFIG.reply from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a QUEUE-GET-CONFIG.reply was received
	 * @param pack QUEUE-GET-CONFIG.reply packet received from datapath
	 */
	virtual void
	handle_queue_get_config_request(crofctl& ctl, cofmsg_queue_get_config_request& msg, uint8_t aux_id = 0);

	/** Handle OF experimenter message. To be overwritten by derived class.
	 *
	 * Called upon reception of a VENDOR.message from the controlling entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param pack VENDOR.message packet received from controller.
	 */
	virtual void
	handle_experimenter_message(crofctl& ctl, cofmsg_experimenter& msg, uint8_t aux_id = 0);

	/** Handle new ctrl
	 *
	 * Called upon creation of a new cofctrl instance.
	 *
	 * @param ctrl new cofctrl instance
	 */
	virtual void
	handle_ctrl_open(crofctl *ctrl);

	/** Handle close event on ctrl
	 *
	 * Called upon deletion of a cofctrl instance
	 *
	 * @param ctrl cofctrl instance to be deleted
	 */
	virtual void
	handle_ctrl_close(crofctl *ctrl);


	/**
	 * @name 	flow_mod_add
	 * @brief 	Add a flow mod received from controller to flow-table
	 *
	 * This method adds a new flow-table-entry to the flow-table of
	 * thw logical switch.
	 *
	 * @param ctl Pointer to cofctl instance representing the controller from whom we received the flow-mod
	 * @param pack Pointer to cofpacket instance storing the flow-mod command received. This must not be freed at the end
	 * of this method!
	 * @return void
	 */
	void
	flow_mod_add(
			crofctl& ctl,
			cofmsg_flow_mod& pack);



	/**
	 * @name 	flow_mod_modify
	 * @brief 	Modify a flow mod received from controller to flow-table
	 *
	 * This method adds a new flow-table-entry to the flow-table of
	 * thw logical switch.
	 *
	 * @param ctl Pointer to cofctl instance representing the controller from whom we received the flow-mod
	 * @param pack Pointer to cofpacket instance storing the flow-mod command received. This must not be freed at the end
	 * of this method!
	 * @return void
	 */
	void
	flow_mod_modify(
			crofctl& ctl,
			cofmsg_flow_mod& pack,
			bool strict);


	/**
	 * @name 	flow_mod_delete
	 * @brief 	Add a flow mod received from controller to flow-table
	 *
	 * This method adds a new flow-table-entry to the flow-table of
	 * thw logical switch.
	 *
	 * @param ctl Pointer to cofctl instance representing the controller from whom we received the flow-mod
	 * @param pack Pointer to cofpacket instance storing the flow-mod command received. This must not be freed at the end
	 * of this method!
	 * @return void
	 */
	void
	flow_mod_delete(
			crofctl& ctl,
			cofmsg_flow_mod& pack,
			bool strict);


};

}// namespace rofl

#endif /* OF12_ENDPOINT_H_ */
