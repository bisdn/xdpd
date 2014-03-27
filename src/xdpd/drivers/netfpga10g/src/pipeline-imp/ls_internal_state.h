/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LS_INTERNAL_STATE_H_
#define LS_INTERNAL_STATE_H_

#include "../config.h"
#include "../io/datapacket_storage.h"

/**
* @file ls_internal_state.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @brief Implements the internal (platform state) logical switch for the NetFGPA
* state
*/

#define PROCESSING_MAX_LSI_THREADS 16

typedef struct logical_switch_internals {
	//Packet storage pointer 
	xdpd::gnu_linux::datapacket_storage* storage;
}logical_switch_internals_t;

#endif /* LS_INTERNAL_STATE_H_ */
