/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NF_PORT_MANAGER_H
#define NF_PORT_MANAGER_H 

#include <string>
#include <list>
#include <pthread.h>

#include <rofl/common/croflexception.h>
#include <rofl/common/utils/c_logger.h>
#include "port_manager.h"

/**
* @file nf_port_manager.h
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief NF management API file.
*/

namespace xdpd {

class eNFPMmBase			: public rofl::RoflException {};
class eNFPMmInvalidNF			: public eNFPMmBase {};
class eNFPMmNotsupportedByDriver	: public eNFPMmBase {};
class eNFPMmUnknownError 		: public eNFPMmBase {};


/**
* @brief NF port management API.
*
* The NF manager API is a C++ interface that can be consumed
* by the add-on management modules for general NF management operations.
* @ingroup cmm_mgmt
*/
class nf_port_manager {

public:
	/*
	* This section contains the API to manage NF. This includes NF creation, NF removal,
	* and NF reconfiguration
	*/
	
	//
	//NF operations
	//
	
	/**
	* @name create_nf_port
	* @brief Create a NF port named nf_name
	*
	* @param nf_name				Name of the NF associated with this port
	* @param nf_port_name			Name of the NF port to be created
	* @param nf_type				Type of the NF to be created
	*/
	static void create_nf_port(std::string& nf_name, std::string& nf_port_name, port_type_t nf_type);
	
	/**
	* @name destroy_nf_nf
	* @brief Destroy a NF port named nf_port_name
	*
	* @param nf_port_name		Name of the NF to be destroyed
	*/
	static void destroy_nf_port(std::string& nf_port_name);

	//TODO: reconfigure a NF
	
	// [+] Add more here..
	
private:
	static pthread_mutex_t mutex;	
	
};

}// namespace xdpd 

#endif /* NF_PORT_MANAGER_H_ */
