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
	uint64_t dpid;

	ROFL_INFO("\n\n[xdpd]["PLUGIN_NAME"] **************************\n");	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] This plugin instantiates some PEX, whose description is embedded into the code.\n");
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] **************************\n\n");	
	
	std::string pexName1 = "vEth1";
	std::string pexName2 = "vEth2";
	std::string pexName3 = "vEth3";
	std::string pexName4 = "vEth4";
	std::string pexName5 = "vEth5";
	std::string pexName6 = "vEth6";
	std::string pexName7 = "vEth7";
	std::string pexName8 = "vEth8";
	std::string pexName9 = "vEth9";
	std::string pexName10 = "vEth10";
	
	std::string scriptPath = "/home/ivano/Desktop/pex";
	
	//Create two PEX
	try
	{
		pex_manager::create_pex_port(pexName1,pexName1,DOCKER);
		pex_manager::create_pex_port(pexName2,pexName2,DOCKER);
		pex_manager::create_pex_port(pexName3,pexName3,DOCKER);
		pex_manager::create_pex_port(pexName4,pexName4,DOCKER);
		pex_manager::create_pex_port(pexName5,pexName5,DOCKER);
		pex_manager::create_pex_port(pexName6,pexName6,DOCKER);
		pex_manager::create_pex_port(pexName7,pexName7,DOCKER);
		pex_manager::create_pex_port(pexName8,pexName8,DOCKER);
		pex_manager::create_pex_port(pexName9,pexName9,DOCKER);
		pex_manager::create_pex_port(pexName10,pexName10,DOCKER);
	}catch(...)
	{
		ROFL_ERR("[xdpd]["PLUGIN_NAME" Unable to create a port");
		return;
	}


/*	try{	
		ROFL_INFO("[xdpd]["PLUGIN_NAME"]Existing PEX:\n");
		std::list<std::string> available_pex = pex_manager::list_available_pex_port_names();	
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

	if(plugin_manager::get_plugins().size() != 1)
	{
		/*
		*	If at least an LSI exists, the PEX ports are connected to the first LSI of the list.
		*/
		std::list<std::string> LSIs =  switch_manager::list_sw_names();
		if(LSIs.size() != 0)
		{
			std::list<std::string>::iterator LSI = LSIs.begin();
			dpid = switch_manager::get_switch_dpid(*LSI);
			
			try
			{
				//Attach
				unsigned int port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName1.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName1, &port_number);
				port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName2.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName2, &port_number);
				port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName3.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName3, &port_number);
				port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName4.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName4, &port_number);
				port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName5.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName5, &port_number);
				port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName6.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName6, &port_number);
				port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName7.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName7, &port_number);
				port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName8.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName8, &port_number);
				port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName9.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName9, &port_number);
				port_number = 0;
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName10.c_str(),dpid);	
				port_manager::attach_port_to_switch(dpid, pexName10, &port_number);
			}catch(...)
			{	
				ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to attach port to LSI '%s'. Unknown error.\n",dpid);
				return;
			}
			//Bring up
			port_manager::bring_up(pexName1);
			port_manager::bring_up(pexName2);
			port_manager::bring_up(pexName3);
			port_manager::bring_up(pexName4);
			port_manager::bring_up(pexName5);
			port_manager::bring_up(pexName6);
			port_manager::bring_up(pexName7);
			port_manager::bring_up(pexName8);
			port_manager::bring_up(pexName9);
			port_manager::bring_up(pexName10);

//			*try
//			{
				//Attach
//				unsigned int port_number = 0;
//				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Attaching PEX port '%s' to LSI '%x'\n",pexName2.c_str(),dpid);	
//				port_manager::attach_port_to_switch(dpid, pexName2, &port_number);
//			}catch(...)
//			{	
//				ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to attach port '%s' to LSI '%s'. Unknown error.\n", pexName2.c_str(),dpid);
//				return;
//			}
			//Bring up
//			port_manager::bring_up(pexName2);
	
//			ROFL_INFO("[xdpd]["PLUGIN_NAME"] All the PEX have been created, and connected to an LSI.\n\n");		

		}
	}

	//Sleep some seconds before destroying the PEX		
/*	char c;
	scanf("%c",&c);
	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] ***********************************\n");
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] ***********************************\n");	
	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Destroying all the PEX just created\n");	
	
	if(plugin_manager::get_plugins().size() != 1)
	{
		//Bring down the port
		port_manager::bring_down(pexName1);
		try
		{
			//Detatch
			port_manager::detach_port_from_switch(dpid, pexName1);
		}catch(...)
		{	
			ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to detatch port '%s' from LSI '%s'. Unknown error.\n", pexName1.c_str(),dpid);
		}
		
		//Bring down the port
		port_manager::bring_down(pexName2);
		try
		{
			//Detatch
			port_manager::detach_port_from_switch(dpid, pexName2);
		}catch(...)
		{	
			ROFL_ERR("[xdpd]["PLUGIN_NAME"] Unable to detatch port '%s' from LSI '%s'. Unknown error.\n", pexName2.c_str(),dpid);
		}
	}

	try
	{
		pex_manager::destroy_pex_port(pexName1);
	}catch(...){}
	try
	{
		pex_manager::destroy_pex_port(pexName2);
	}catch(...){}
	
*/	if(plugin_manager::get_plugins().size() == 1)
	{	
		ROFL_INFO("[xdpd]["PLUGIN_NAME"] You have compiled xdpd with this plugin only. This xdpd execution is pretty useless, since no Logical Switch Instances will be created and there will be no way (RPC) to create them...\n\n");	
		ROFL_INFO("[xdpd]["PLUGIN_NAME"]You may now press Ctrl+C to finish xdpd execution.\n");
	}

}

