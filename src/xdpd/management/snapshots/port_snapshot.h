/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PORT_SNAPSHOT_H
#define PORT_SNAPSHOT_H 

#include <string> 
#include <list> 
#include <rofl.h>
#include <rofl/datapath/pipeline/switch_port.h>

/**
* @file port_snapshot.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief C++ port snapshot
*/

namespace xdpd {


class port_queue_snapshot{

public:


};

/**
* @brief C++ switch snapshot 
* @ingroup cmm_mgmt
*/
class port_snapshot {


public:	

	

	
	/**
	* Port queues
	*/
	std::list<port_queue_snapshot> queues;	
};


}// namespace xdpd

#endif /* PORT_SNAPSHOT_H_ */
