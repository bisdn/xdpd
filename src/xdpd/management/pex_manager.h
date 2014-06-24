/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PEX_MANAGER_H
#define PEX_MANAGER_H 

#include <string>
#include <list>
#include <pthread.h>

#include <rofl/common/croflexception.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/hal/pex/pex_driver.h>

/**
* @file pex_manager.h
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief PEX management API file.
*/

namespace xdpd {

class ePexmBase			: public rofl::RoflException {};
class ePexmInvalidPEX	: public ePexmBase {};
class ePexmUnknownError : public ePexmBase {};


/**
* @brief PEX port management API.
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
 	 * @name pex_port_exists
	 * @brief Check if the PEX port named pex_port_name already exists
	 * 
	 * @param pex_port_name		Name of the PEX port to be checked
	 */
	static bool pex_port_exists(std::string& pex_port_name);

	/**
	* @name list_available_PEX_names
	* @brief List the names of the PEX available
	*/
	static std::list<std::string> list_available_pex_port_names(void);


	//
	//PEX operations
	//
	
	/**
	* @name create_pex_port
	* @brief Create a PEX port named pex_name
	*
	* @param pex_name				Name of the PEX associated with this port
	* @param pex_port_name			Name of the PEX port to be created
	* @param pex_type				Type of the PEX to be created
	*/
	static void create_pex_port(std::string& pex_name, std::string& pex_port_name, PexType pexType);
	
	/**
	* @name destroy_pex_pex
	* @brief Destroy a PEX port named pex_port_name
	*
	* @param pex_port_name		Name of the PEX to be destroyed
	*/
	static void destroy_pex_port(std::string& pex_port_name);

	//TODO: reconfigure a PEX
	
	// [+] Add more here..
	
private:
	static pthread_mutex_t mutex;	
	
};

}// namespace xdpd 

#endif /* PEX_MANAGER_H_ */
