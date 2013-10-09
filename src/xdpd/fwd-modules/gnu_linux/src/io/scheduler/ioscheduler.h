/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSCHEDULER_H
#define IOSCHEDULER_H 

#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/switch_port.h>

/**
* @file ioscheduler.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief I/O scheduler base class which defines the way
* way I/O threads go through the ports for TX and RX.
*
*/

namespace xdpd {
namespace gnu_linux {

/**
* @brief I/O scheduler base class which defines the way
* way I/O threads go through the ports for TX and RX.
*
* @ingroup fm_gnu_linux_io_schedulers
*/
class ioscheduler{ 

public:
	/**
	* Main processing routine. It must always end with pthread_exit()
	*/
	static void* process_io(void* grp);


#ifdef DEBUG
	//Method to by-pass processing systems
	static void set_by_pass_processing(bool value);	
#endif

};

}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* IOSCHEDULER_H_ */
