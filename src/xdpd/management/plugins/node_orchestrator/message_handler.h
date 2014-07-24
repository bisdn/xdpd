#ifndef SERVER_H_
#define SERVER_H_ 1

#pragma once

#include <json_spirit/json_spirit.h>
#include <json_spirit/writer.h>
#include <json_spirit/reader.h>
#include <json_spirit/reader_template.h>
#include <json_spirit/writer_template.h>
#include <json_spirit/value.h>

#include <string>
#include <sstream>
#include <list>

#include <libconfig.h++> 

#include "node_orchestrator.h"
#include "LSI.h"
#include "orchestrator_constants.h"

using namespace std;
using namespace json_spirit;

namespace xdpd
{

class MessageHandler
{

friend class NodeOrchestrator;

private:
	/**
	*	This map contains, for each LSI, a list of the NFs port names
	*	attached with the LSI itself
	*/
	static map<uint64_t,list<string> > nfPortNames;

	/**
	*	For each physical port, indicates if it is an edge port or a 
	*	core port
	*/
	static map<string,string> portSide;
	
protected:

	/**
	*	Parse the configuration file of the plugin
	*/
	static bool parseConfigFile(string conf_file);

	static string processCommand(string message);

/**
*	Example of command to create a new LSI
*
	{
		"command" : "create-lsi",
		"controller" :
			{
				"address" : "127.0.0.1",
				"port" : "6653"
			},
		"ports" : ["ge0","ge1"],
		"network-functions" : [
			 {
		        "name" : "VPN",
		        "type" : "dpdk",
		        "ports" : [
		            "VPN_1"
		        ]
		    },
		    {
		        "name" : "firewall",
		        "type" : "docker",
		        "ports" : [
		            "firewall_1",
		            "firewall_2",
		            "firewall_3"
		        ]
		    }	
		],
		"virtual-links" : 
			{
				"number" : "2",
				"remote-lsi" : "1"
			}
	}
*
*	Example of answer
*
	{
		"command" : "create-lsi",
		"status" : "ok",
		"lsi-id" : "0x200",
		"ports" : [
			 { 
			 	"name" : "ge0",
			 	"id" : "0x1"
			 },
			{
				"name" : "ge1",
				"id" : "0x2"
			}	
		],
		"network-functions" : [
			{
		        "name" : "VPN",
		        "ports" : [
		            {
		                "id" : 1,
		                "name" : "VPN_1"
		            }
			    ]
		    },
		    {
		        "name" : "firewall",
		        "ports" : [
		            {
		                "id" : 2,
		                "name" : "firewall_1"
		            },
		            {
		                "id" : 3,
		                "name" : "firewall_2"
		            },
		            {
		                "id" : 4,
		                "name" : "firewall_3"
		            }
		        ]
		    }
		],
		"virtual-links" : [
			{
				"local-id" : "0x4",
				"remote-id" : "0xFA"	
			},
			{
				"local-id" : "0x5",
				"remote-id" : "0xFB"	
			}
		]
	}
*/
	static string createLSI(string message);
	static string createLSIAnswer(LSI lsi, map<string,map<string,uint32_t> > nfPorts,list<pair<unsigned int, unsigned int> > virtual_links);

/**
*	Example of command to destroy an LSI
*
	{
		"command" : "create-lsi",
		"lsi-id" : "0x200"
	}
*
*	Example of answer
*
	{
		"command" : "destroy-lsi",
		"status" : "ok"
	}
*/
	static string destroyLSI(string message);

/**
*	Example of command to attach physical ports
*
	{
		"command" : "attach-phy-ports",
		"lsi-id" : "0x100",
		"ports" : ["ge0","ge1"]
	}
*
*	Example of answer
*
	{
		"command" : "attach-phy-ports",
		"status" : "ok",
		"ports" : [
			{ 
			 	"name" : "ge0",
			 	"id" : "0x1"
			},
			{
				"name" : "ge1",
				"id" : "0x2"
			}	
		]
	}
*/
	static string attachPhyPorts(string message);

/**
*	Example of command to detach physical ports
*
	{
		"command" : "detach-physical-ports",
		"lsi-id" : "0x200",
		"ports" : [ge0,ge1]
	}
*
*	Example of answer
*	
*	{
		"command" : "detach-physical-ports",
		"status" : "ok"
	}
*/
	static string detachPhyPorts(string message);


/**
*	Example of command to create NF ports
*
	{
		"command" : "create-nf-ports",
		"lsi-id" : "0x200",
		"network-functions" : [
			 {
		        "name" : "VPN",
		        "type" : "dpdk",
		        "ports" : [
		            "VPN_1"
		        ]
		    },
		    {
		        "name" : "firewall",
		        "type" : "docker",
		        "ports" : [
		            "firewall_1",
		            "firewall_2"
		        ]
		    }
*
*	Example of answer
*
	{
		"command" : "create-nf-ports",
		"status" : "ok",
		"network-functions" : [
			{
		        "name" : "VPN",
		        "ports" : [
		            {
		                "id" : 1,
		                "name" : "VPN_1"
		            }
			    ]
		    },
		    {
		        "name" : "firewall",
		        "ports" : [
		            {
		                "id" : 2,
		                "name" : "firewall_1"
		            },
		            {
		                "id" : 3,
		                "name" : "firewall_2"
		            }
		        ]
		    }
		]
	}
*/
	static string createNFPorts(string message);

/**
*	Example of command to destroy NF ports
*
	{
		"command" : "destroy-nf-ports",
		"lsi-id" : "0x200",
		"ports" : [
			"VPN_1",
			"firewall_2"  
		]
	}
*
*	Example of answer
*	
*	{
		"command" : "destroy-nf-ports",
		"status" : "ok"
	}
*/
	static string destroyNFPorts(string message);

/**
*	Example of command to discover the physical ports of xDPD
*
	{
		"command" : "discover-physical-ports"
	}
*
*	Example of answer
*	
	{
		"answer" : "discover-physical-ports",
		"status" : "ok",
		"ports" : [
			{
				"name" : "ge0", 
				"type " : "edge"
			},
			{
				"name" : "ge1",
				"type" : "core"
			}
		]
	}
*/
	static string discoverPhyPorts(string message);

/**
*	Example of command to create new virtual links
*
	{
		"command" : "create-virtual-links",
		"number" : "2",
		"lsi-a" : "0x100",
		"lsi-b" : "0x200"
	}
*
*	Example of answer
*
	{
		"command" : "create-virtual-links",
		"status" : "ok",
		"virtual-links" : [
			{
				"id-a" : "0x4",
				"id-b" : "0xFA",	
			},
			{
				"id-a" : "0x5",
				"id-b" : "0xFB",	
			}
		]
	}
*/	
	static string createVirtualLinks(string message);

/**
*	Example of command to destroy virtual links
*
	{
		"command" : "destroy-virtual-links"
		"virtual-links" : [
			{
				"lsi-id" : "0x100",
			 	"vlink-id" : "0xF"
			},
			{
				"lsi-id" : "0x100",
			 	"vlink-id" : "0xAF"
			}
		]
	}
*
*	Example of answer
*	
*	{
		"command" : "destroy-virtual-links",
		"status" : "ok"
	}
*/	
	static string destroyVirtualLinks(string message);

/**	
*	Example of answer
*	
	{
		"answer" : "create-lsi",
		"status" : "error",
		"message" : "bla bla bla"
	}
*/
	static string createErrorMessage(string command, string message);
};

}

#endif //SERVER_H_
