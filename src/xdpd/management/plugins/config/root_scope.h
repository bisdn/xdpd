/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ROOT_SCOPE_H
#define ROOT_SCOPE_H 

#include <iostream>
#include <libconfig.h++> 
#include <rofl/common/croflexception.h>
#include "scope.h"

/**
* @file root_scope.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Root node of the configuration 
* 
*/

namespace xdpd {

class root_scope : public scope {
	
public:
	root_scope();
	virtual ~root_scope();
		
private:

};

class config_scope : public scope {
	
public:
	config_scope();
	virtual ~config_scope();
		
private:

};


}// namespace xdpd 

#endif /* ROOT_SCOPE_H_ */


