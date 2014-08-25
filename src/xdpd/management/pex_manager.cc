#include "pex_manager.h"
#include <assert.h>
 
using namespace rofl;
using namespace xdpd;

#define MODULE_NAME "[xdpd][pex_manager]" 

pthread_mutex_t pex_manager::mutex = PTHREAD_MUTEX_INITIALIZER; 

//
// PEX creation/destruction
//

void pex_manager::create_pex_port(std::string& pex_name, std::string& pex_port_name, PexType pexType)
{
	if(pexType == DPDK_PRIMARY)
	{
		ROFL_ERR("%s ERROR: only DPDK_SECONDARY and DPDK_KNI types are currently implemented\n", MODULE_NAME);
		throw ePexmInvalidPEX();	
	}

	// Serialize
	pthread_mutex_lock(&pex_manager::mutex);
	
	// The PEX port must not exist
	if(port_manager::exists(pex_port_name))
	{
		pthread_mutex_unlock(&pex_manager::mutex);
		ROFL_ERR("%s ERROR: Attempting to create an existent PEX port %s\n", MODULE_NAME, pex_port_name.c_str());
		throw ePexmInvalidPEX();
	}
	
	if(hal_driver_pex_create_pex_port(pex_name.c_str(), pex_port_name.c_str(), pexType) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&pex_manager::mutex);
		assert(0);
		ROFL_ERR("%s ERROR: Driver was unable to create the PEX port\n", MODULE_NAME, pex_port_name.c_str());
		throw ePexmUnknownError(); 
	}
	
	ROFL_INFO("%s PEX port %s created\n", MODULE_NAME, pex_port_name.c_str());

	//TODO: notify the creation to the plugins?

	//Release mutex	
	pthread_mutex_unlock(&pex_manager::mutex);
}

void pex_manager::destroy_pex_port(std::string& pex_port_name)
{
	// Serialize
	pthread_mutex_lock(&pex_manager::mutex);
	
	// The PEX must exist
	if(!port_manager::exists(pex_port_name))
	{
		pthread_mutex_unlock(&pex_manager::mutex);
		ROFL_ERR("%s ERROR: Attempting to destroy a non-existent PEX port %s\n", MODULE_NAME, pex_port_name.c_str());
		throw ePexmInvalidPEX();
	}
	
	if(hal_driver_pex_destroy_pex_port(pex_port_name.c_str()) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&pex_manager::mutex);
		assert(0);
		ROFL_ERR("%s ERROR: Driver was unable to destroy the PEX port %s\n", MODULE_NAME, pex_port_name.c_str());
		throw ePexmUnknownError(); 
	}
	
	ROFL_INFO("%s PEX port %s destroyed\n", MODULE_NAME, pex_port_name.c_str());

	//TODO: notify the destruction to the plugins?

	//Release mutex	
	pthread_mutex_unlock(&pex_manager::mutex);
}

