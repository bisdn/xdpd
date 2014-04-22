#include "run_pex.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/croflexception.h>
#include <string>
#include <inttypes.h>
#include "../../plugin_manager.h"
#include "../../port_manager.h"
#include "../../switch_manager.h"
#include "../../pex_manager.h"

using namespace xdpd;
using namespace rofl;

#define PLUGIN_NAME "RunPEX_plugin" 

void runPEX::init()
{
	ROFL_INFO("\n\n[xdpd]["PLUGIN_NAME"] **************************\n");	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] This plugin instantiates some PEX, whose description is embedded into the code.\n");
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] **************************\n\n");	
	
	std::string pexName1 = "pex25";
	std::string pexName2 = "pex11";
	
	//Create two PEX
	try
	{
		pex_manager::create_pex(pexName1,0x1,2);
	}catch(...)
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to create %s\n",pexName1.c_str());
		return;
	}

	try{
		pex_manager::create_pex(pexName2,0x2,2);
	}catch(...)
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME" Unable to create %s",pexName2.c_str());
		return;
	}

/*	try{	
		ROFL_INFO("[xdpd]["PLUGIN_NAME"]Existing PEX:\n");
		std::list<std::string> available_pex = pex_manager::list_available_pex_names();	
		for(std::list<std::string>::iterator it = available_pex.begin(); it != available_pex.end(); it++)
		{
			ROFL_INFO("[xdpd]["PLUGIN_NAME"]\t%s\n",(*it).c_str());
		}
	}catch(...)
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME" No PEX exists\n");
		return;
	}
*/	


	if(plugin_manager::get_plugins().size() == 1)
	{

		ROFL_INFO("[xdpd]["PLUGIN_NAME"] You have compiled xdpd with this plugin only. This xdpd execution is pretty useless, since no Logical Switch Instances will be created and there will be no way (RPC) to create them...\n\n");	
		ROFL_INFO("[xdpd]["PLUGIN_NAME"]You may now press Ctrl+C to finish xdpd execution.\n");
	}
	else
	{
		/*
		*	If at least an LSI exists, the PEX ports are connected to the first LSI of the list.
		*/
		std::list<std::string> LSIs =  switch_manager::list_sw_names();
		if(LSIs.size() != 0)
		{
			std::list<std::string>::iterator LSI = LSIs.begin();
			uint64_t dpid = switch_manager::get_switch_dpid(*LSI);
			
			try
			{
				//Attach
				unsigned int port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName1.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName1, &port_number);
			}catch(...)
			{	
				ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to attach port '%s' to LSI '%s'. Unknown error.\n", pexName1.c_str(),dpid);
				return;
			}
			//Bring up
			port_manager::bring_up(pexName1);
	
			try
			{
				//Attach
				unsigned int port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName2.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName2, &port_number);
			}catch(...)
			{	
				ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to attach port '%s' to LSI '%s'. Unknown error.\n", pexName2.c_str(),dpid);
				return;
			}
			//Bring up
			port_manager::bring_up(pexName2);
	
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] All the PEX have been created, and connected to an LSI.\n\n");
		}
	}
		
	//Destroy the PEX previously created
	pex_manager::destroy_pex(pexName1);
	pex_manager::destroy_pex(pexName2);
}

