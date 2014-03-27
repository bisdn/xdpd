/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PROCESSINGMANAGER_H
#define PROCESSINGMANAGER_H 

#include <pthread.h>
#include <vector>
#include <map>
#include <rofl.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include "../util/safevector.h" 
#include "../io/ports/ioport.h"
#include "ls_internal_state.h"
#include "../config.h" 

/**
* @file processingmanager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief In charge of processing (Openflow pipeline)
* threads; e.g. launching and stopping them.
* 
*/

namespace xdpd {
namespace gnu_linux {

/**
* @brief Processing manager. Creates/destroys the processing threads associated with a logical switch instance.
*
* @ingroup fm_gnu_linux_processing
*/
class processingmanager{ 

public:

	static rofl_result_t create_rx_pgs(of_switch_t* sw); 
	static rofl_result_t destroy_rx_pgs(of_switch_t* sw); 
	static int get_rx_pg_index_rr(of_switch_t* sw, ioport* port); 

private:
	//Handle mutual exclusion over the ls_processing_groups 
	static pthread_mutex_t mutex;
};


}// namespace xdpd::gnu_linux 
}// namespace xdpd

#endif /* PROCESSINGMANAGER_H_ */
