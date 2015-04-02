#include "nf_port_manager.h"
#include <assert.h>
#include "system_manager.h"
 
using namespace rofl;
using namespace xdpd;

#define MODULE_NAME "[xdpd][nf_port_manager]" 

pthread_mutex_t nf_port_manager::mutex = PTHREAD_MUTEX_INITIALIZER; 

//
// NF creation/destruction
//
void nf_port_manager::create_nf_port(std::string& nf_name, std::string& nf_port_name, port_type_t nf_type)
{

	hal_extension_ops_t* ops = system_manager::__get_driver_hal_extension_ops();
	
	if(!ops->nf_ports.create_nf_port)
		throw eNFPMmNotsupportedByDriver(); 

	if(nf_type == PORT_TYPE_NF_NATIVE)
	{
		ROFL_ERR(DEFAULT, "%s ERROR: only SHMEM or EXTERNAL types are currently implemented\n", MODULE_NAME);
		throw eNFPMmInvalidNF();	
	}

	// Serialize
	pthread_mutex_lock(&nf_port_manager::mutex);
	
	// The NF port must not exist
	if(port_manager::exists(nf_port_name))
	{
		pthread_mutex_unlock(&nf_port_manager::mutex);
		ROFL_ERR(DEFAULT, "%s ERROR: Attempting to create an existent NF port %s\n", MODULE_NAME, nf_port_name.c_str());
		throw eNFPMmInvalidNF();
	}
	
	if((*ops->nf_ports.create_nf_port)(nf_name.c_str(), nf_port_name.c_str(), nf_type) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&nf_port_manager::mutex);
		assert(0);
		ROFL_ERR(DEFAULT, "%s ERROR: Driver was unable to create the NF port\n", MODULE_NAME, nf_port_name.c_str());
		throw eNFPMmUnknownError(); 
	}

	ROFL_INFO(DEFAULT, "%s Successfully created NF port %s of type %s, {%s}\n", MODULE_NAME, nf_port_name.c_str(), (nf_type == PORT_TYPE_NF_SHMEM)? "SHMEM": "EXTERNAL", nf_name.c_str() );

	//TODO: notify the creation to the plugins?

	//Release mutex	
	pthread_mutex_unlock(&nf_port_manager::mutex);
}

void nf_port_manager::destroy_nf_port(std::string& nf_port_name)
{

	hal_extension_ops_t* ops = system_manager::__get_driver_hal_extension_ops();
	
	if(!ops->nf_ports.destroy_nf_port)
		throw eNFPMmNotsupportedByDriver();
	// Serialize
	pthread_mutex_lock(&nf_port_manager::mutex);
	
	// The NF must exist
	if(!port_manager::exists(nf_port_name))
	{
		pthread_mutex_unlock(&nf_port_manager::mutex);
		ROFL_ERR(DEFAULT, "%s ERROR: Attempting to destroy a non-existent NF port %s\n", MODULE_NAME, nf_port_name.c_str());
		throw eNFPMmInvalidNF();
	}
	
	if((*ops->nf_ports.destroy_nf_port)(nf_port_name.c_str()) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&nf_port_manager::mutex);
		assert(0);
		ROFL_ERR(DEFAULT, "%s ERROR: Driver was unable to destroy the NF port %s\n", MODULE_NAME, nf_port_name.c_str());
		throw eNFPMmUnknownError(); 
	}
	
	ROFL_INFO(DEFAULT, "%s NF port %s destroyed\n", MODULE_NAME, nf_port_name.c_str());

	//TODO: notify the destruction to the plugins?

	//Release mutex	
	pthread_mutex_unlock(&nf_port_manager::mutex);
}

