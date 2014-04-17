/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PEX_MANAGER_H
#define PEX_MANAGER_H 

#include <string>
#include <list>

/**
* @file pex_manager.h
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief PEX management API file.
*/

namespace xdpd {

/**
* @brief Port management API.
*
* The PEX manager API is a C++ interface that can be consumed
* by the add-on management modules for general PEX management operations.
* @ingroup cmm_mgmt
*/
class pex_manager {

public:
	/*
	* This section contains the API to manage PEX. This includes PEX creation, PEX removal,
	* and PEX reconfiguration
	*/

	//
	// Basic
	//

 	/**
	 * @brief Check if the PEX named pex_name already exists
	 */
	static bool pex_exists(std::string& pex_name);

	/**
	* @brief List the names of the PEX available. 
	*/
	static std::list<std::string> list_available_PEX_names(void);


	//
	//PEX operations
	//
	
	/**
	* @brief Create a PEX named pex_name
	*/
	static bool create_pex(std::string& pex_name);
	
	/**
	* @brief Destroy a PEX named pex_name
	*/
	static bool destroy_pex(std::string& pex_name);

	//TODO: reconfigure a PEX

};

}// namespace xdpd 

#endif /* PEX_MANAGER_H_ */
