/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BUFFERPOOL_C_WRAPPER_H_
#define BUFFERPOOL_C_WRAPPER_H_

#include <stdint.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>

/**
* @file bufferpool_c_wrapper.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief C wrapper for the bufferpool 
*
*/

//C++ extern C
ROFL_BEGIN_DECLS

//Wrapper for 
void bufferpool_release_buffer_wrapper(datapacket_t* pkt); 

datapacket_t* bufferpool_get_buffer_wrapper(void); 

void bufferpool_init_wrapper(long long unsigned int capacity);

void bufferpool_destroy_wrapper(void);

//C++ extern C
ROFL_END_DECLS

#endif /* BUFFERPOOL_C_WRAPPER_H_ */
