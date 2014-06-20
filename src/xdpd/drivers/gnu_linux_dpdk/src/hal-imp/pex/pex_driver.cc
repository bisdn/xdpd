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
	
	return HAL_SUCCESS;
}

hal_result_t hal_driver_pex_destroy_pex_port(const char *pex_port_name)
{
	//create the port and initialize the structure for the pipeline
	if(port_manager_destroy_pex_port(pex_port_name) != ROFL_SUCCESS)
	{
		return HAL_FAILURE;
	}

	return HAL_SUCCESS;
}

hal_result_t hal_driver_pex_start_pex_port(const char *pex_port_name, port_type_t pex_port_type)
{
	if(pex_port_type != PORT_TYPE_PEX_DPDK_SECONDARY)
		return HAL_SUCCESS;
		
	if(pex_port_type != PORT_TYPE_PEX_DPDK_KNI)
	{
		struct ifreq ifr;
	
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd == -1)
		{
			ROFL_ERR(DRIVER_NAME"[pex_driver] Cannot bring up KNI PEX port\n");
			return HAL_FAILURE;
		}
		/* get interface name */
		strncpy(ifr.ifr_name, pex_port_name, IFNAMSIZ);
		
		/* Read interface flags */
		if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) 
		{
			ROFL_ERR(DRIVER_NAME"[pex_driver] Cannot bring up KNI PEX port\n");
			return HAL_FAILURE;
		}
	
		#ifdef ifr_flags
			# define IRFFLAGS       ifr_flags
		#else   /* Present on kFreeBSD */
			# define IRFFLAGS       ifr_flagshigh
		#endif
	
		// If interface is down, bring it up
		if (!(ifr.IRFFLAGS & IFF_UP)) 
		{
			ifr.IRFFLAGS |= IFF_UP;
			if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) 
			{
				ROFL_ERR(DRIVER_NAME"[pex_driver] Cannot bring up KNI PEX port\n");
				return HAL_FAILURE;
			}
		}
		
		return HAL_SUCCESS;
	}
	
	ROFL_ERR(DRIVER_NAME"[pex_driver] The port type must be DPDK_SECONDARY or DPDK_KNI\n");
	return HAL_FAILURE;
}

hal_result_t hal_driver_pex_stop_pex_port(const char *pex_port_name, port_type_t pex_port_type)
{

	if(pex_port_type != PORT_TYPE_PEX_DPDK_SECONDARY)
		return HAL_SUCCESS;
		
	if(pex_port_type != PORT_TYPE_PEX_DPDK_KNI)
	{
		struct ifreq ifr;
	
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd == -1)
		{
			ROFL_ERR(DRIVER_NAME"[pex_driver] Cannot bring down KNI PEX port\n");
			return HAL_FAILURE;
		}
		/* get interface name */
		strncpy(ifr.ifr_name, pex_port_name, IFNAMSIZ);

		/* Read interface flags */
		if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) 
		{
			ROFL_ERR(DRIVER_NAME"[pex_driver] Cannot bring down DPDK KNI PEX port\n");
			return HAL_FAILURE;
		}

		#ifdef ifr_flags
			# define IRFFLAGS       ifr_flags
		#else   /* Present on kFreeBSD */
			# define IRFFLAGS       ifr_flagshigh
		#endif

		// If interface is up, bring it down
		if (ifr.IRFFLAGS & IFF_UP)
		{
			ifr.IRFFLAGS &= ~IFF_UP;
			if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) 
			{
				ROFL_ERR(DRIVER_NAME"[pex_driver] Cannot bring down DPDK KNI PEX port\n");
				return HAL_FAILURE;
			}
		}
		return HAL_SUCCESS;
	}
	
	ROFL_ERR(DRIVER_NAME"[pex_driver] The port type must be DPDK_SECONDARY or DPDK_KNI\n");
	return HAL_FAILURE;
}

