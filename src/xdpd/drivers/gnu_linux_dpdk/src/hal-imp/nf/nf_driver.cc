#include <rofl.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/wait.h>

#include <rofl/datapath/hal/nf/nf_driver.h>
#include <rofl/common/utils/c_logger.h>

#include "../../io/datapacket_storage.h"
#include "../../io/iface_manager.h"

using namespace xdpd::gnu_linux;

hal_result_t hal_driver_nf_create_nf_port(const char *nf_name, const char *nf_port_name, port_type_t nf_port_type)
{
	if((nf_port_type != PORT_TYPE_NF_SHMEM) && (nf_port_type != PORT_TYPE_NF_EXTERNAL))
	{
		return HAL_FAILURE;
	}

	//create the port and initialize the structure for the pipeline
	if(iface_manager_create_nf_port(nf_name, nf_port_name, nf_port_type) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}
	
	return HAL_SUCCESS;
}

hal_result_t hal_driver_nf_destroy_nf_port(const char *nf_port_name)
{
	//create the port and initialize the structure for the pipeline
	if(iface_manager_destroy_nf_port(nf_port_name) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}

	return HAL_SUCCESS;
}

