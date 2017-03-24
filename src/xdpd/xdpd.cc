/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "management/system_manager.h"
#include "xdpd/common/utils/c_logger.h"

using namespace xdpd;

/*
 * xDPd startup routine 
 */
int main(int argc, char** argv){

	//Check for root privileges 
	if(geteuid() != 0){
		XDPD_ERR("ERROR: Root permissions are required to run %s\n",argv[0]);
		exit(EXIT_FAILURE);	
	}

	XDPD_INFO("[xdpd] Initializing system...\n");

	//Let system manager initialize all subsytems
	system_manager::init(argc, argv);
	
	XDPD_INFO("[xdpd] Goodbye!\n");
	exit(EXIT_SUCCESS);
}
