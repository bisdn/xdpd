/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_OF_BUFFER_
#define _NETFPGA_OF_BUFFER_

#include <inttypes.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/switch_port.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include "../util/compiler_assert.h"

/**
* @file of_buffer.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NetFPGA OF buffer definition and operations 
*/

//Defines the buffer structure exchanged between 
//Kernel driver and user-space xDPD
//Comes from ofpbuf adaptation in git://github.com/eastzone/openflow.git 

typedef struct netfpga_of_buffer{

	void* base;			/* First byte of area malloc()'d area. */
	size_t allocated;		/* Number of bytes allocated. */

	void* data;			/* First byte actually in use. */
	size_t size;			/* Number of bytes in use. */
	
	void *l2;			/* Link-level header. */
	void *l3;			/* Network-level header. */
	void *l4;			/* Transport-level header. */
	void *l7;			/* Application data. */
	struct netfpga_of_buffer *next;	/* Next in a list of ofpbufs. */
	void *private;			/* Private pointer for use by owner. */

}netfpga_of_buffer_t;

COMPILER_ASSERT( OF_BUFFER_SIZE, ( sizeof(netfpga_of_buffer_t) == 32) );


//C++ extern C
ROFL_BEGIN_DECLS

//C++ extern C
ROFL_END_DECLS

#endif //NETFPGA_OF_BUFFER
