/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _DATAPACKET_NETFPGA_
#define _DATAPACKET_NETFPGA_

#include "of_buffer.h"

/**
* @file datapacket_netfpga.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NetFPGA datapacket structure
*/

typedef struct datapacket_netfpga{

	//Buffer got from the kernel
	netfpga_of_buffer_t buffer;	

	//Reference back to the datapacket pipeline
	datapacket_t* pipeline_pkt;

}datapacket_netfpga_t;




#endif //DATAPACKET_NETFPGA
