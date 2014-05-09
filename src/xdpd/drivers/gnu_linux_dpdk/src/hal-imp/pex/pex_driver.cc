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

hal_result_t hal_driver_pex_create_pex(const char *pex_name, PexType pexType, const char *path)
{
	if(pexType != DPDK)
	{
		return HAL_FAILURE;
	}

	//create the port and initialize the structure for the pipeline
	if(port_manager_create_pex_port(pex_name) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}
	
	//Fill the structure associated to the PEX
	pex[pex_id-1].pex_name = pex_name;
	pex[pex_id-1].pexType = pexType;
	pex[pex_id-1].path = path;

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
	if(pex[pex_id].pexType == DPDK)
	{
		//In this case, noting to do
		return HAL_SUCCESS;
	}
	else
	{
		ROFL_ERR(DRIVER_NAME"[port_manager] Only DPDK PEX are currenty supported\n");
        return HAL_FAILURE;	
	}
}

hal_result_t hal_driver_pex_stop_pex(uint32_t pex_id)
{

	if(pex[pex_id].pexType == DPDK)
	{
		//In this case, noting to do
		return HAL_SUCCESS;
	}
	else
	{
		ROFL_ERR(DRIVER_NAME"[port_manager] Only DPDK PEX are currenty supported\n");
        return HAL_FAILURE;	
	}
}

