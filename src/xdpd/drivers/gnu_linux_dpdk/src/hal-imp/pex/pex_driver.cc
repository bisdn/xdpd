#include <rofl.h>
#include <assert.h>
#include <rofl/datapath/hal/pex/pex_driver.h>
#include <rofl/common/utils/c_logger.h>

#include "../../io/datapacket_storage.h"
#include "../../io/port_manager.h"

using namespace xdpd::gnu_linux;

bool hal_driver_pex_exists(const char *pex_name)
{
	//The PEX and the port used to exchange pkts with that PEX has the same name
	return physical_switch_get_port_by_name(pex_name) != NULL;
}

//TODO: complete this method
pex_name_list_t* hal_driver_get_all_pex_names()
{
	return NULL;
}

//TODO: complete this method
hal_result_t hal_driver_pex_create_pex(const char *pex_name, uint32_t core_mask, uint32_t num_memory_channels)
{
	//create the port and initialize the structure for the pipeline
	if(port_manager_create_pex_port(pex_name) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}
	
	//create secondary process
	
	//int ret = system("~/Desktop/prova.sh");

	//std::cout << "----> " << ret << std::endl;	

	return HAL_SUCCESS;
}

//TODO: complete this method
hal_result_t hal_driver_pex_destroy_pex(const char *pex_name)
{
	//create the port and initialize the structure for the pipeline
	if(port_manager_destroy_pex_port(pex_name) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}
	
	// destroy secondary process

	return HAL_SUCCESS;
}

