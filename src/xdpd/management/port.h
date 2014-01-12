/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PORT_H
#define PORT_H 

#include <rofl/common/cerror.h>

/**
* @file port.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
*
* @brief Port abstraction file.
*/

using namespace rofl;

namespace xdpd {


//Port specific exceptions
class ePortBase	: public cerror {};	//Base class for all port related exceptions
class ePortUnknownError : public ePortBase {};
//TODO: add extra exceptions here...

/**
* @brief Device port abstraction
*
* @ingroup cmm_mgmt
*/
class port {

public:

	//Constructor
	port(std::string& _name):name(_name){};

	/*
	* Public APIs to abstract and protect fwd_module
	*/

	//
	//Getters 
	//

	/**
	* Get the interface name
	*/
	std::string get_name(void);

	/**
	* Get meta-state up/down
	*/
	bool get_admin_state(void);
	
	//TODO: Add more fine-grained "up/down states" here...

	//
	// Actions
	//

	
	/**
	* Set the port administratively up (meta up) 
	*/
	void up(void);
	
	/**
	* Set the port administratively down (meta down) 
	*/
	void down(void);
		
	//TODO: Add more fine-grained "up/down actions" here...
		
	
private:
	/**
	* @brief System's interface name
	*
	* This is the only 
	*/
	std::string name;

};

}// namespace xdpd 

#endif /* PORT_H_ */
