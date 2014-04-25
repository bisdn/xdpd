/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "init.h"

// Prototypes

void usage(void);
int parse_command_line(int argc, char *argv[]);
void init_shared_resources(void);
void sig_handler(int received_signal);


// Functions

void usage(void)
{
	char message[]=	\

	"Usage:                                                                                   \n" \
	"  sudo ./pex -c core_mask -n memory_channels --proc-type=secondary --                    \n" \
	"             --l lcore_id --p name [--h]                                                 \n" \
	"                                                                                         \n" \
	"Parameters:                                                                              \n" \
	"  -c core_mask                                                                           \n" \
	"        Number of lcores used by the PEX. Note that currently only one lcore is actually \n" \
	"        used.                                                                            \n" \
	"  -n memory_channels	                                                                  \n" \
	"        Number of channels used by the PEX to access to the memory.                      \n" \
	"  --proc-type=secondary                                                                  \n" \
	"        The PEX must be executed as a DPDK secondary process.                            \n" \
	"  -l lcore_id                                                                            \n" \
	"        Identifier needed to support multiple PEX on the same core. It must be unique.   \n" \
	"  -p name                                                                                \n" \
	"        Name of the PEX. It must be unique.                                              \n" \
	"                                                                                         \n" \
	"Options:                                                                                 \n" \
	"  -h                                                                                     \n" \
	"        Print this help.                                                                 \n" \
	"                                                                                         \n" \
	"Example:                                                                                 \n" \
	"  sudo ./pex -c 1 -n 2 --proc-type=secondary -- --l 1 --p pex25                          \n\n";

	fprintf(stderr,"\n\n[%s] %s\n",GENERAL_NAME,message);
}

/**
* @brief Parses the command line used to run the PEX
*
* @param argc	Number of parameters in the command line (excluding those used
*				the EAL)
* @param argv	The command line (except the parameters used by the EAL)
*/
int parse_command_line(int argc, char *argv[])
{
	int opt, ret;
	char **argvopt;
	int option_index;
	char *prgname = argv[0];
	static struct option lgopts[] = {
		{"p", 1, 0, 0},
		{"l", 1, 0, 0},
		{"h", 0, 0, 0},
		{NULL, 0, 0, 0}
	};

	uint32_t arg_p = 0, arg_l = 0;

	argvopt = argv;

	while ((opt = getopt_long(argc, argvopt, "", lgopts, &option_index)) != EOF)
    {

		switch (opt)
		{
		/* long options */
		case 0:
			if (!strcmp(lgopts[option_index].name, "p"))/* PEX identifier */
			{
                arg_p = 1;
                strcpy(pex_params.pex_name,optarg);
                sprintf(module_name,"[%s][%s]",GENERAL_NAME,pex_params.pex_name);
   			}
   			else if (!strcmp(lgopts[option_index].name, "l"))/* lcore_id */
			{
			    unsigned int lcore_id;
				char lcore_id_string[NAME_LENGTH];
                arg_l = 1;
                strncpy(lcore_id_string,optarg,strlen(optarg));
                sscanf(lcore_id_string,"%u",&lcore_id);
                if(lcore_id >= RTE_MAX_LCORE)
                {
                    fprintf(stderr,"[%s] Incorrect value for --l argument\n",GENERAL_NAME);
					return -1;
				}
				RTE_PER_LCORE(_lcore_id) = lcore_id;
			}
			else if (!strcmp(lgopts[option_index].name, "h"))/* help */
   			{
   				return -1;
   			}
			break;

		default:
			return -1;
		}
	}

	/* Check that all mandatory arguments are provided */
	if (arg_p == 0 || arg_l == 0)
	{
		fprintf(stderr,"[%s] Not all mandatory arguments are present in the command line\n",GENERAL_NAME);
		return -1;
	}

	if (optind >= 0)
		argv[optind - 1] = prgname;

	ret = optind - 1;
	optind = 0; /* reset getopt lib */
	return ret;
}

/**
*	@brief 	The PEX attaches to the resources used to exchange 
*			packets with the xDPD
*/
void init_shared_resources(void)
{
	char queue_name[NAME_LENGTH];

	/*
	*	Connect to the rte_rings
	*/
	snprintf(queue_name, NAME_LENGTH, "%s-to-pex", pex_params.pex_name);
	pex_params.to_pex_queue = rte_ring_lookup(queue_name);
	if (pex_params.to_pex_queue == NULL)
	{
		fprintf(stderr,"%s Cannot get rte_ring '%s'\n", module_name,queue_name);
	    exit(1);
	}
	
	snprintf(queue_name, NAME_LENGTH, "%s-to-xdpd", pex_params.pex_name);
	pex_params.to_xdpd_queue = rte_ring_lookup(queue_name);
	if (pex_params.to_xdpd_queue == NULL)
	{
		fprintf(stderr,"%s Cannot get rte_ring '%s'\n",module_name, queue_name);
	    exit(1);
	}

	/*
	*	Connect to the POSIX named semaphore
	*/
	pex_params.semaphore = sem_open(pex_params.pex_name, O_CREAT, 0644, 0);
	if(pex_params.semaphore == SEM_FAILED)
	{
		fprintf(stderr,"%s Cannot get the semaphore '%s'\n",module_name, pex_params.pex_name);
	    exit(1);
	}

}

void sig_handler(int received_signal)
{
    if (received_signal == SIGINT)
    {
    	sem_unlink(pex_params.pex_name);
        exit(0);
    }
}

int init(int argc, char *argv[])
{
	if(parse_command_line(argc, argv) < 0)
	{
		usage();
		return -1;
	}
	
	fprintf(stdout,"%s **************************\n",module_name);	
	fprintf(stdout,"%s This is the init function of the example PEX. This will just send to xDPD those packets received from xDPD.\n",module_name);
	fprintf(stdout,"%s **************************\n\n",module_name);
	
    init_shared_resources();

	/*
	* Set the signal handler and global variables for termination
	*/
	signal(SIGINT, sig_handler);
	
	return 0;
}
