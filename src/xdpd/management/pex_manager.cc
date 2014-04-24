#include "pex_manager.h"
#include <assert.h>
 
using namespace rofl;
using namespace xdpd;

#define MODULE_NAME "[xdpd][pex_manager]" 

pthread_mutex_t pex_manager::mutex = PTHREAD_MUTEX_INITIALIZER; 

bool pex_manager::pex_exists(std::string& pex_name)
{
	return hal_driver_pex_exists(pex_name.c_str());
}

std::list<std::string> pex_manager::list_available_pex_names()
{
	unsigned int i;
	pex_name_list_t* pex_names;
	std::list<std::string> pex_name_list;
	
	//Call the driver to list the ports
	pex_names = hal_driver_get_all_pex_names();
	
	if(!pex_names)
		throw ePexmUnknownError();

	//Run over the PEX and get the name
	for(i=0;i<pex_names->num_of_pex;i++)
		pex_name_list.push_back(std::string(pex_names->names[i].name));

	//Destroy the list of PEX
	//pex_name_list_destroy(pex_names); //TODO: decommentare questa linea
	
	return pex_name_list; 
}

//
// PEX creation/destruction
//

void pex_manager::create_pex(std::string& pex_name, std::string& path, uint32_t core_mask, uint32_t num_memory_channels, uint32_t lcore_id)
{
	// Serialize
	pthread_mutex_lock(&pex_manager::mutex);
	
	// The PEX must not exist
	if(pex_exists(pex_name))
	{
		pthread_mutex_unlock(&pex_manager::mutex);
		ROFL_ERR("%s ERROR: Attempting to create an existent PEX %s\n", MODULE_NAME, pex_name.c_str());
		throw ePexmInvalidPEX();
	}
	
	if(hal_driver_pex_create_pex(pex_name.c_str(), path.c_str(), core_mask, num_memory_channels, lcore_id) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&pex_manager::mutex);
		assert(0);
		ROFL_ERR("%s ERROR: Driver was unable to create the PEX\n", MODULE_NAME, pex_name.c_str());
		throw ePexmUnknownError(); 
	}
	
	ROFL_INFO("%s PEX %s created\n", MODULE_NAME, pex_name.c_str());

	//TODO: notify the creation to the plugins?

	//Release mutex	
	pthread_mutex_unlock(&pex_manager::mutex);
}

void pex_manager::destroy_pex(std::string& pex_name)
{
	// Serialize
	pthread_mutex_lock(&pex_manager::mutex);
	
	// The PEX must exist
	if(!pex_exists(pex_name))
	{
		pthread_mutex_unlock(&pex_manager::mutex);
		ROFL_ERR("%s ERROR: Attempting to destroy a non-existent PEX %s\n", MODULE_NAME, pex_name.c_str());
		throw ePexmInvalidPEX();
	}
	
	if(hal_driver_pex_destroy_pex(pex_name.c_str()) != HAL_SUCCESS)
	{
		pthread_mutex_unlock(&pex_manager::mutex);
		assert(0);
		ROFL_ERR("%s ERROR: Driver was unable to destroy the PEX\n", MODULE_NAME, pex_name.c_str());
		throw ePexmUnknownError(); 
	}
	
	ROFL_INFO("%s PEX %s destroyed\n", MODULE_NAME, pex_name.c_str());

	//TODO: notify the destruction to the plugins?

	//Release mutex	
	pthread_mutex_unlock(&pex_manager::mutex);
}

