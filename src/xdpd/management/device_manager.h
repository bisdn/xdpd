/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H 

#include <list>
#include <rofl/datapath/afa/fwd_module.h>

/**
* @file device_manager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Device or chassis management API file.
*/

using namespace rofl;

namespace xdpd {


//Device manager exceptions
class eDeviceBase		: public cerror {};	// base error class for all device_manager related errors
class eDeviceUnknownError	: public eDeviceBase {};

/**
* @brief Device management API.
*
* The device manager API is a C++ interface that can be consumed
* by the add-on management modules for general device(interface) management operations.
* @ingroup cmm_mgmt
*/
class device_manager {

public:
	/*
	* This section contains the API to manage devices. This includes system device management and
	* device attachment to logical switchs
	*/

 	/**
	 * @brief Retrieves the chassis monitoring entity represented by the key
	 */
	static device get_chassis_monitoring_entity(std::string& key);

private:
	
};

}// namespace xdpd 

#endif /* DEVICE_MANAGER_H_ */
