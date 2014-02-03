/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MONITORING_MANAGER_H
#define MONITORING_MANAGER_H 

#include <list>
#include <rofl.h>
#include <rofl/common/cerror.h>
#include <rofl/datapath/afa/fwd_module.h>

/**
* @file monitoring_manager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Monitoring API file.
*/

using namespace rofl;

namespace xdpd {


//Monitoring manager exceptions
class eMonitoringBase		: public cerror {};	// base error class for all monitoring_manager related errors
class eMonitoringUnknownError	: public eMonitoringBase {};

/**
* @brief Monitoring management API.
*
* The device manager API is a C++ interface that can be consumed
* by the add-on management modules for general device(interface) management operations.
* @ingroup cmm_mgmt
*/
class monitoring_manager {

public:
	/*
	* This section contains the API to manage devices. This includes system device management and
	* device attachment to logical switchs
	*/

	//TODO
	static monitoring_snapshot_state_t* get_monitoring_snapshot(void);

private:
	
};

}// namespace xdpd 

#endif /* MONITORING_MANAGER_H_ */
