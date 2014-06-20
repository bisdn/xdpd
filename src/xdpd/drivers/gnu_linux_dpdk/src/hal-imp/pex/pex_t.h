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
//IVANO - FIXME: I think that this structure is quite useless

typedef struct{
	const char *pex_port_name;
	PexType pexType;
}pex_port_t;

//pex_port_t pex_port[PORT_MANAGER_MAX_PORTS];

#endif //PEX_T_H
