/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LS_INTERNAL_STATE_H_
#define LS_INTERNAL_STATE_H_

#include "../config.h"
#include "../util/circular_queue.h"
#include "../io/datapacket_storage.h"

/**
* @file ls_internal_state.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
* @brief Implements the internal (platform state) logical switch
* state
*/

namespace xdpd {
namespace gnu_linux {

#define PROCESSING_MAX_LSI_THREADS 16

typedef struct logical_switch_internals {
	//Input queues
	circular_queue<datapacket_t, PROCESSING_INPUT_QUEUE_SLOTS>* input_queues[PROCESSING_MAX_LSI_THREADS];

	//PKT_IN queue
	circular_queue<datapacket_t, PROCESSING_PKT_IN_QUEUE_SLOTS>* pkt_in_queue; 
	
	//Packet storage pointer 
	datapacket_storage* storage;
}logical_switch_internals_t;

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* LS_INTERNAL_STATE_H_ */
