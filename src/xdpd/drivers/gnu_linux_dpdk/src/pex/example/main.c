/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "main.h"
#include "init.h"

int main(int argc, char *argv[])
{
	//Check for root privileges 
	if(geteuid() != 0)
	{
		fprintf(stderr,"ERROR: Root permissions are required to run %s\n",argv[0]);	
		exit(EXIT_FAILURE);	
	}

	uint32_t lcore;
	int ret;

	/* Init EAL */
	ret = rte_eal_init(argc, argv);

	fflush(stdout);
	if (ret < 0)
		return -1;
	argc -= ret;
	argv += ret;

	if(init(argc, argv) < 0)
		return -1;

	rte_eal_mp_remote_launch(do_pex, NULL, CALL_MASTER);
	
	//In this version, the PEX uses just one lcores.. Other potential lcores are no
	//used
	RTE_LCORE_FOREACH_SLAVE(lcore) 
	{ 
		if (rte_eal_wait_lcore(lcore)/*Wait until an lcore finishes its job.*/ < 0) 
		{
			return -1;
		}
	}

	return 0;
}
