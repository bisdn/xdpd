/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OF_ENDPOINT_H
#define OF_ENDPOINT_H 

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <pthread.h>
#include <stdio.h>
#include <limits.h>
#include <endian.h>
#include <string.h>
#include <time.h>

#include <stdexcept>

#include <rofl/common/crofbase.h>
#include <rofl/datapath/afa/afa.h>
//#include "openflow_switch.h"

/**
* @file of_endpoint.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief of_endpoint is a version-agnostic abstractio of the openflow agent
* 
*/

using namespace rofl;

namespace xdpd {

/*
* This is an abstract class. TODO change the name of crofbase to of_endpoint or something similar, and get rid of this
*/

//Fwd declaration
class openflow_switch;

class of_endpoint : public crofbase {

public:

	of_endpoint(uint32_t supported_ofp_versions = (1 << OFP12_VERSION)) : crofbase(supported_ofp_versions), sw(NULL) {};

	virtual ~of_endpoint() {};

	/*
	* Port notifications
	*/

	virtual afa_result_t notify_port_add(switch_port_t* port)=0;
	
	virtual afa_result_t notify_port_delete(switch_port_t* port)=0;

	virtual afa_result_t notify_port_status_changed(switch_port_t* port)=0;



protected:
	
	//Switch to which the endpoint belongs to
	openflow_switch* sw;	
};

}// namespace rofl

#endif /* OF_ENDPOINT_H_ */
