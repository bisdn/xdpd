/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_PACKET_IO_
#define _NETFPGA_PACKET_IO_

#include <inttypes.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>

//C++ extern C
ROFL_BEGIN_DECLS

//Process a packet_in from the kernel on a specific port
void netpfga_io_read_from_port(switch_port_t* port);

//C++ extern C
ROFL_END_DECLS

#endif //NETFPGA_PACKET_IO
