/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PUSH_POP_OPERATIONS_X86_
#define _PUSH_POP_OPERATIONS_X86_

#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/common/datapacket.h>

rofl_result_t pkt_push(datapacket_t* pkt, uint8_t* push_point, unsigned int num_of_bytes, unsigned int offset);
rofl_result_t pkt_pop(datapacket_t* pkt, uint8_t* pop_point, unsigned int num_of_bytes, unsigned int offset);



#endif //_PUSH_POP_OPERATIONS_X86_