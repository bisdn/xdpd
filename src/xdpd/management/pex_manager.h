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
 	 * @name pex_exists
	 * @brief Check if the PEX named pex_name already exists
	 * 
	 * @param pex_name	Name of the PEX to be checked
	 */
	static bool pex_exists(std::string& pex_name);

	/**
	* @name list_available_PEX_names
	* @brief List the names of the PEX available
	*/
	static std::list<std::string> list_available_pex_names(void);


	//
	//PEX operations
	//
	
	/**
	* @name create_pex
	* @brief Create a PEX named pex_name
	*
	* @param pex_name				Name of the PEX to be created
	* @param pex_type				Type of the PEX to be created
	* @param path					Path of the PEX excutable (may be not needed)
	*/
	static void create_pex(std::string& pex_name, PexType pexType, std::string& path);
	
	/**
	* @name destroy_pex
	* @brief Destroy a PEX named pex_name
	*
	* @param pex_name	Name of the PEX to be destroyed
	*/
	static void destroy_pex(std::string& pex_name);

	//TODO: reconfigure a PEX
	
	// [+] Add more here..
	
private:
	static pthread_mutex_t mutex;	
	
};

}// namespace xdpd 

#endif /* PEX_MANAGER_H_ */
