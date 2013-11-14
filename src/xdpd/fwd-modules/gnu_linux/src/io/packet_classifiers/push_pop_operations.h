#ifndef _PUSH_POP_OPERATIONS_X86_
#define _PUSH_POP_OPERATIONS_X86_

#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/common/datapacket.h>

rofl_result_t pkt_push(datapacket_t* pkt, uint8_t* push_point, unsigned int num_of_bytes, unsigned int offset);
rofl_result_t pkt_pop(datapacket_t* pkt, uint8_t* pop_point, unsigned int num_of_bytes, unsigned int offset);



#endif //_PUSH_POP_OPERATIONS_X86_