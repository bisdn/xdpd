#include <rofl_datapath.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/wait.h>

#include <rofl/common/utils/c_logger.h>

#include "nf_extensions.h" 
#include "../io/datapacket_storage.h"
#include "../io/iface_manager.h"

using namespace xdpd::gnu_linux;

#if defined(DPDK_PATCHED_KNI)
static bool kni_inited = false;
#endif //DPDK_PATCHED_KNI

hal_result_t hal_driver_dpdk_nf_create_nf_port(const char *nf_name, const char *nf_port_name, port_type_t nf_port_type)
{
	if((nf_port_type != PORT_TYPE_NF_SHMEM) && (nf_port_type != PORT_TYPE_NF_EXTERNAL))
	{
		return HAL_FAILURE;
	}

#if defined(DPDK_PATCHED_KNI)
	//TODO: rte_kni_init() should be moved back to hal_driver_init(), but
	//for that the DPDK KNI kernel module should take into account that 
	//the number of KNI interfaces to bootstrap the kthread/s 

	//Initialize KNI subsystem if 
	if(nf_port_type == PORT_TYPE_NF_EXTERNAL && !kni_inited){
		rte_kni_init(GNU_LINUX_DPDK_MAX_KNI_IFACES);
		kni_inited = true;
	}
#endif

	//create the port and initialize the structure for the pipeline
	if(iface_manager_create_nf_port(nf_name, nf_port_name, nf_port_type) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}
	
	return HAL_SUCCESS;
}

hal_result_t hal_driver_dpdk_nf_destroy_nf_port(const char *nf_port_name)
{
	//create the port and initialize the structure for the pipeline
	if(iface_manager_destroy_nf_port(nf_port_name) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}

	return HAL_SUCCESS;
}

