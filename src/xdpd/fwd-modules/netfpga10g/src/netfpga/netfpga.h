/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NETFPGA_H_
#define _NETFPGA_H_

#include <inttypes.h>
#include <stbool.h>
#include <rofl.h>

/**
* @file netfga.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief NetFPGA basic abstractions 
*/

typedef struct netfpga_dev_info{

	//File descriptor for the netfpga
	int fd;

}netfpga_dev_info_t;

/**
* @brief   Initializes the netfpga shared state, including appropiate state of registers and bootstrap.
*/
rofl_result_t netfpga_init(void);

/**
* @brief Destroys state of the netfpga, and restores it to the original state (state before init) 
*/
rofl_result_t netfpga_destroy(void);

#endif //NETFPGA_H
