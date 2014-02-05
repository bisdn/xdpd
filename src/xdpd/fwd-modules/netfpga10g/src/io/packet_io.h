/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_PACKET_IO_
#define _NETFPGA_PACKET_IO_

#include <inttypes.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <pcap.h>

#include <rofl/datapath/afa/openflow/openflow1x/of1x_fwd_module.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_statistics.h>

#include "../netfpga/ports.h"
#include "bufferpool.h"
#include "datapacket_storage.h"
#include "../pipeline-imp/ls_internal_state.h"

//C++ extern C
ROFL_BEGIN_DECLS

//Process a packet_in from the kernel on a specific port
void netpfga_io_read_from_port(switch_port_t* port);

//C++ extern C
ROFL_END_DECLS

#endif //NETFPGA_PACKET_IO
