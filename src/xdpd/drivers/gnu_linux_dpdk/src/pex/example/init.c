#include "init.h"

void sig_handler(int received_signal);

/**
*	@brief 	The PEX attaches to the resources used to exchange 
*			packets with the xDPD
*/
static void init_shared_resources(void)
{
	char queue_name[NAME_LENGTH];

	strcpy(pex_params.pex_name,"pex25"); //FIXME: this value MUST be received from command line

	/*
	*	Connect to the rte_rings
	*/
	snprintf(queue_name, NAME_LENGTH, "%s-up", pex_params.pex_name);
	pex_params.up_queue = rte_ring_lookup(queue_name);
	if (pex_params.up_queue == NULL)
	{
		//ROFL_ERR("["MODULE_NAME"] Cannot get rte_ring '%s'\n", queue_name);
	    exit(1);
	}
	
	snprintf(queue_name, NAME_LENGTH, "%s-down", pex_params.pex_name);
	pex_params.down_queue = rte_ring_lookup(queue_name);
	if (pex_params.down_queue == NULL)
	{
	//	ROFL_ERR("["MODULE_NAME"] Cannot get rte_ring '%s'\n", queue_name);
	    exit(1);
	}

	/*
	*	Connect to the POSIX named semaphore
	*/
	//TODO: is the flag correct?
	pex_params.semaphore = sem_open(pex_params.pex_name, O_CREAT, 0644, 0);
	if(pex_params.semaphore == SEM_FAILED)
	{
		//ROFL_ERR("["MODULE_NAME"] Cannot get the semaphore '%s'\n", pex_params.pex_name);
	    exit(1);
	}

    pex_params.send_ring_size = APP_DEFAULT_BURST_SIZE; //TODO: togliere e mettere costante
}



//TODO: complete this method
void sig_handler(int received_signal)
{
    if (received_signal == SIGINT)
    {
           exit(0);
    }
}

void init(void)
{
    init_shared_resources();
	/*
	* Set the signal handler and global variables for termination
	*/
	signal(SIGINT, sig_handler); //Console -> kill -2 pid
}
