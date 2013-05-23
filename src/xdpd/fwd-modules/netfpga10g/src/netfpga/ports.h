/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_PORTS_
#define _NETFPGA_PORTS_


#include <inttypes.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include "../util/compiler_assert.h"
#include "netfpga.h"

/**
* @file ports.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NetFPGA ports manipulation routines 
*/

#define NETFPGA_NUM_PORTS 4 
#define NETFPGA_INTERFACE_BASE_NAME "nf"
#define NETFPGA_INTERFACE_NAME_LEN 4 //nfX\0

//C++ extern C
ROFL_BEGIN_DECLS

/**
* @brief Discover NetFPGA physical ports 
*/
rofl_result_t netfpga_discover_ports(void);

/**
* @brief Attach all physical ports to switch 
*/
rofl_result_t netfpga_attach_ports(of_switch_t* sw);


//C++ extern C
ROFL_END_DECLS

#endif //NETFPGA_PORTS
