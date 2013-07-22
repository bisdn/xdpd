/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LS_INTERNAL_STATE_H_
#define LS_INTERNAL_STATE_H_

#include "../util/ringbuffer_c_wrapper.h"
#include "../io/datapacket_storage_c_wrapper.h"

/**
* @file ls_internal_state.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
*/

typedef struct logical_switch_internals {
	ringbuffer_handle_p ringbuffer; //ringbuffer_handle is a pointer!
	datapacket_store_handle store_handle;
}logical_switch_internals_t;

#endif /* LS_INTERNAL_STATE_H_ */
