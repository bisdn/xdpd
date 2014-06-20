#include <rofl.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/wait.h>

#include <rofl/datapath/hal/pex/pex_driver.h>
#include <rofl/common/utils/c_logger.h>

#include "../../io/datapacket_storage.h"
#include "../../io/iface_manager.h"
#include "pex_t.h"

using namespace xdpd::gnu_linux;

hal_result_t hal_driver_pex_create_pex_port(const char *pex_name, const char *pex_port_name, PexType pexType)
{
	if((pexType != DPDK_SECONDARY) && (pexType != DPDK_KNI))
	{
		return HAL_FAILURE;
	}

	//create the port and initialize the structure for the pipeline
	if(port_manager_create_pex_port(pex_name, pex_port_name, (pexType == DPDK_SECONDARY)? PORT_TYPE_PEX_DPDK_SECONDARY : PORT_TYPE_PEX_DPDK_KNI) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}
	
	//Fill the structure associated to the PEX port
//	pex_port[pex_id-1].pex_port_name = pex_port_name;
//	pex_port[pex_id-1].pexType = pexType;

	return HAL_SUCCESS;
}

hal_result_t hal_driver_pex_destroy_pex_port(const char *pex_port_name)
{
	//create the port and initialize the structure for the pipeline
	if(port_manager_destroy_pex_port(pex_port_name) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}

	//IVANO - TODO: cleanup the field in pex_t

	return HAL_SUCCESS;
}

hal_result_t hal_driver_pex_start_pex_port(uint32_t pex_port_id)
{
//	if(pex_port[pex_port_id].pexType == DPDK_SECONDARY || pex_port[pex_port_id].pexType == DPDK_KNI)
//	{
		//In this case, noting to do
		return HAL_SUCCESS;
//	}
//	else
//	{
//		ROFL_ERR(DRIVER_NAME"[port_manager] Only DPDK and KNI PEX are currenty supported\n");
 //       return HAL_FAILURE;	
//	}
}

hal_result_t hal_driver_pex_stop_pex_port(uint32_t pex_port_id)
{

//	if(pex_port[pex_port_id].pexType == DPDK_SECONDARY || pex_port[pex_port_id].pexType == DPDK_KNI)
//	{
		//In this case, noting to do
		return HAL_SUCCESS;
//	}
//	else
//	{
//		ROFL_ERR(DRIVER_NAME"[port_manager] Only DPDK PEX are currenty supported\n");
 //       return HAL_FAILURE;	
//	}
}

