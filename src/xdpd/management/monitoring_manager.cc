/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "monitoring_manager.h"
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;
using namespace rofl;


monitoring_snapshot_state_t* monitoring_manager::get_monitoring_snapshot(uint64_t last_rev){
	return hal_driver_get_monitoring_snapshot(last_rev);
}
