#include <rofl.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/wait.h>

#include <rofl/datapath/hal/pex/pex_driver.h>
#include <rofl/common/utils/c_logger.h>

#include "../../io/datapacket_storage.h"
#include "../../io/port_manager.h"
#include "pex_t.h"

using namespace xdpd::gnu_linux;

bool hal_driver_pex_exists(const char *pex_name)
{
	//The PEX and the port used to exchange pkts with that PEX has the same name
	return physical_switch_get_port_by_name(pex_name) != NULL;
}

//TODO: complete this method
pex_name_list_t* hal_driver_get_all_pex_names()
{
	assert(0 && "Not implemented yet");
	return NULL;
}

hal_result_t hal_driver_pex_create_pex(const char *pex_name, const char *path, uint32_t core_mask, uint32_t num_memory_channels, uint32_t lcore_id)
{
	//create the port and initialize the structure for the pipeline
	if(port_manager_create_pex_port(pex_name) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}
	
	//Fill the structure associated to the PEX
	pex[pex_id-1].pex_name = pex_name;
	pex[pex_id-1].path = path;
	pex[pex_id-1].core_mask = core_mask;
	pex[pex_id-1].num_memory_channels = num_memory_channels;
	pex[pex_id-1].lcore_id = lcore_id;

	return HAL_SUCCESS;
}

hal_result_t hal_driver_pex_destroy_pex(const char *pex_name)
{
	//create the port and initialize the structure for the pipeline
	if(port_manager_destroy_pex_port(pex_name) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}

	//IVANO - TODO: cleanup the field in pex_t

	return HAL_SUCCESS;
}

hal_result_t hal_driver_pex_start_pex(uint32_t pex_id)
{
	pid_t pid = fork();

	char core_mask[100], num_memory_channels[100], lcore_id[100];
	sprintf(core_mask,"%d",pex[pex_id].core_mask);
	sprintf(num_memory_channels,"%d",pex[pex_id].num_memory_channels);
	sprintf(lcore_id,"%d",pex[pex_id].lcore_id);

    if(pid >= 0) // fork was successful
    {
        if(pid == 0)
        {
        	//PEX process - change the executable into the PEX
        	execl(pex[pex_id].path,"pex","-c",core_mask,"-n",num_memory_channels,"--proc-type=secondary","--","--l",lcore_id,"--p",pex[pex_id].pex_name,(char*)0);
        }
                
        pex[pex_id].pid = pid;  
    }
    else // fork failed
    {
        ROFL_ERR(DRIVER_NAME"[port_manager] Unable to run a new process\n");
        return HAL_FAILURE;
    }
	
	return HAL_SUCCESS;
}

hal_result_t hal_driver_pex_stop_pex(uint32_t pex_id)
{

	kill(pex[pex_id].pid, SIGINT);
	
	waitpid(pex[pex_id].pid, NULL, 0);
	
	return HAL_SUCCESS;
}

