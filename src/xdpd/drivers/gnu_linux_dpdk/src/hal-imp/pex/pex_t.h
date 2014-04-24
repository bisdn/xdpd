/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PEX_T_H
#define PEX_T_H

/**
* @file pex_driver.h
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief For each PEX associated with an existing PEX port, this struct 
*		contains information about the the process that is running/will 
*		run the PEX
*/
typedef struct{
	const char *pex_name;
	const char *path;
	uint32_t core_mask;
	uint32_t num_memory_channels;
	uint32_t lcore_id;
	pid_t pid;
}pex_t;

pex_t pex[PORT_MANAGER_MAX_PORTS];

#endif //PEX_T_H
