#include "nf_port_manager.h"
#include <assert.h>
 
using namespace rofl;
using namespace xdpd;

#define MODULE_NAME "[xdpd][nf_port_manager]" 

pthread_mutex_t nf_port_manager::mutex = PTHREAD_MUTEX_INITIALIZER; 

//
// NF creation/destruction
//
void nf_port_manager::create_nf_port(std::string& nf_name, std::string& nf_port_name, port_type_t nf_type)
{
	if(nf_type == PORT_TYPE_NF_NATIVE)
	{
		ROFL_ERR("%s ERROR: only DPDK_SECONDARY and DPDK_KNI types are currently implemented\n", MODULE_NAME);
		throw eNFPMmInvalidNF();	
	}

	// Serialize
	pthread_mutex_lock(&nf_port_manager::mutex);
	
	// The NF port must not exist
	if(port_manager::exists(nf_port_name))
	{
		pthread_mutex_unlock(&nf_port_manager::mutex);
		ROFL_ERR("%s ERROR: Attempting to create an existent NF port %s\n", MODULE_NAME, nf_port_name.c_str());
		throw eNFPMmInvalidNF();
	}
	
	if(hal_driver_nf_create_nf_port(nf_name.c_str(), nf_port_name.c_str(), nf_type) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&nf_port_manager::mutex);
		assert(0);
		ROFL_ERR("%s ERROR: Driver was unable to create the NF port\n", MODULE_NAME, nf_port_name.c_str());
		throw eNFPMmUnknownError(); 
	}
	
	ROFL_INFO("%s NF port %s created\n", MODULE_NAME, nf_port_name.c_str());

	//TODO: notify the creation to the plugins?

	//Release mutex	
	pthread_mutex_unlock(&nf_port_manager::mutex);
}

void nf_port_manager::destroy_nf_port(std::string& nf_port_name)
{
	// Serialize
	pthread_mutex_lock(&nf_port_manager::mutex);
	
	// The NF must exist
	if(!port_manager::exists(nf_port_name))
	{
		pthread_mutex_unlock(&nf_port_manager::mutex);
		ROFL_ERR("%s ERROR: Attempting to destroy a non-existent NF port %s\n", MODULE_NAME, nf_port_name.c_str());
		throw eNFPMmInvalidNF();
	}
	
	if(hal_driver_nf_destroy_nf_port(nf_port_name.c_str()) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&nf_port_manager::mutex);
		assert(0);
		ROFL_ERR("%s ERROR: Driver was unable to destroy the NF port %s\n", MODULE_NAME, nf_port_name.c_str());
		throw eNFPMmUnknownError(); 
	}
	
	ROFL_INFO("%s NF port %s destroyed\n", MODULE_NAME, nf_port_name.c_str());

	//TODO: notify the destruction to the plugins?

	//Release mutex	
	pthread_mutex_unlock(&nf_port_manager::mutex);
}

