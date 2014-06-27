#include "message_handler.h"

namespace xdpd
{

map<uint64_t,list<string> > MessageHandler::nfPortNames;

string MessageHandler::processCommand(string message)
{
	Value value;
    read( message, value );
	Object obj = value.getObject();
	
	for( Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
    {
        const string& name  = i->first;
        const Value&  value = i->second;

        if( name == "command" )
        {
        	string command = value.getString();
			if(command == CREATE_LSI)
				return createLSI(message);
			if(command == DESTROY_LSI)
				return destroyLSI(message);
			if(command == ATTACH_PHY_PORTS)
				return attachPhyPorts(message);
			if(command == DETACH_PHY_PORTS)
				return detachPhyPorts(message);
			if(command == CREATE_NF_PORTS)
				return createNFPorts(message);
			if(command == DESTROY_NF_PORTS)
				return destroyNFPorts(message);
			if(command == CREATE_VIRTUAL_LINKS)
				return createVirtualLinks(message);
			if(command == DESTROY_VIRTUAL_LINKS)
				return destroyVirtualLinks(message);
			if(command == DISCOVER_PHY_PORTS)
				return discoverPhyPorts(message);
			
			//ERROR
			break;
        }
    }

	//ERROR
	return createErrorMessage(string(ERROR), string("Unknown command"));
}

string MessageHandler::createLSI(string message)
{
	list<string> physicalPorts;
	list< pair<string,list<string> > > networkFunctions; //list <nf name, list <port name> >
	map<string,PexType> nfTypes; //list of <nf_name, nf type>
	map<string,map<string,uint32_t> > nfPorts; //map <nf name, map <port name, port id> >
	
	string controllerAddress;
	string controllerPort;
	bool foundControllerAddress = false;
	bool foundControllerPort = false;
	
	int vlinks_number = 0;
    uint64_t vlinks_remote_dpid = 0;
	
	Value value;
    read( message, value );
	Object obj = value.getObject();
		
	for( Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
    {
        const string& name  = i->first;
        const Value&  value = i->second;

        if( name == "command" )
        {
			continue;
        }
        else if( name == "controller")
        {
      		Object controller = value.getObject();  
      		for( Object::const_iterator j = controller.begin(); j != controller.end(); ++j )
			{
				const string& c_name  = j->first;
				const Value&  c_value = j->second;
				
				if( c_name == "address" )
				{
					controllerAddress += (c_value.getString());
					foundControllerAddress = true;
				}
				else if (c_name == "port")
				{
					controllerPort += (c_value.getString());
					foundControllerPort = true;
				}
			}
        }
        else if( name == "ports" )
        {
			const Array& ports_array = value.getArray();
			for( unsigned int i = 0; i < ports_array.size(); ++i )
			{
				physicalPorts.push_back( ports_array[i].getString() );
			}
        }
        else if( name == "network-functions")
    	{
			const Array& nfs_array = value.getArray();
			
			for( unsigned int i = 0; i < nfs_array.size(); ++i )
			{
				Object nfs = nfs_array[i].getObject();
				
				bool foundName = false;
				bool foundType = false;
        		bool foundPorts = false;
        		list<string> ports;
				string name;
				PexType nfType;

				for( Object::const_iterator j = nfs.begin(); j != nfs.end(); ++j )
				{
					const string& nf_name  = j->first;
					const Value&  nf_value = j->second;	
									
					if(nf_name == "name")
					{
						name = nf_value.getString();
						foundName = true;
					}
					else if(nf_name == "type")
					{
						string tmp = nf_value.getString();
						if(tmp != "dpdk" && tmp != "docker")
						{
							ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-lsi\" with a network function with a wrong \"type\"");
						return createErrorMessage(string(CREATE_LSI), string(" Received command \"create-lsi\" with a network function with a wrong \"type\""));
						}
						
						nfType = (tmp == "dpdk")? DPDK_SECONDARY : DPDK_KNI;
						foundType = true;
					}
					else if(nf_name == "ports")
					{
						const Array& ports_array = nf_value.getArray();
						for( unsigned int i = 0; i < ports_array.size(); ++i )
						{
							ports.push_back( ports_array[i].getString() );
						}
						if(ports.size() > 0)
							foundPorts = true;
					}
				
				}
				if(!foundName || !foundPorts || !foundType)
				{
					ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-lsi\" with a network function without the \"name\", the \"ports\", the \"type\", all of them");
					return createErrorMessage(string(CREATE_LSI), string(" Received command \"create-lsi\" with a network function without the \"name\", the \"ports\", the \"type\", all of them"));
				}
				
				networkFunctions.push_back(make_pair(name,ports));
				nfTypes[name] = nfType;
			}
        }
        else if( name == "virtual-links")
        {
        	Object virtual_links = value.getObject();
        	bool foundNumber = false;
        	bool foundRemoteLSI = false; 
        	 
      		for( Object::const_iterator j = virtual_links.begin(); j != virtual_links.end(); ++j )
			{
				const string& c_name  = j->first;
				const Value&  c_value = j->second;
				
				if( c_name == "number" )
				{
					vlinks_number = c_value.getInt();
					foundNumber = true;
				}
				else if (c_name == "remote-lsi")
				{
					vlinks_remote_dpid = c_value.getInt(); //FIXME: the dpid is actually a uint64_t
					foundRemoteLSI = true;
				}
			}
			if(!foundNumber || !foundRemoteLSI)
			{
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-lsi\" with field \"virtual-links\" without sub-fields \"number\" or \"remote-lsi\" or \"both\"");
				return createErrorMessage(string(CREATE_LSI), string("Received command \"create-lsi\" with field \"virtual-links\" without sub-fields \"number\" or \"remote-lsi\" or \"both\""));
			}
        }
    }
    
    if(!foundControllerAddress || !foundControllerPort)
    {
    	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-lsi\" without field \"port\" or \"address\" or \"both\"");
    	return createErrorMessage(string(CREATE_LSI), string("Command without controller port, controller address, or both"));
    }
 
 	LSI lsi;
 	try
	{   
		lsi = NodeOrchestrator::createLSI(physicalPorts,controllerAddress,controllerPort);
	} catch (...)
	{
		return createErrorMessage(string(CREATE_LSI),string("error during the creation of the LSI"));
	}
	
	list<string> names;
	for(list< pair<string,list<string> > >::iterator it = networkFunctions.begin(); it != networkFunctions.end(); it++)
 	{	
 		stringstream nfName;
 		nfName << lsi.getDpid() << "_" << it->first;
 		
 		list<string> ports = it->second;
 		map<string,unsigned int> port_id;
 		
 		for(list<string>::iterator p = ports.begin(); p != ports.end(); p++)
 		{
	 		stringstream portName;
	 		portName << lsi.getDpid() << "_" << *p;
	 		try
	 		{
	 			names.push_back(portName.str());
		 		port_id[*p] = NodeOrchestrator::createNfPort(lsi.getDpid(), nfName.str(), portName.str(),nfTypes[it->first]);
		 	}catch(...)
		 	{
		 		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Command \"create-lsi\" failed");
				stringstream ss;
				ss << "An error occurred while creating/attaching the NF port " << portName.str();
				return createErrorMessage(string(CREATE_LSI), ss.str());	
		 	}
	 	}
	 	
	 	nfPorts[it->first] = port_id;
 	}
 	nfPortNames[lsi.getDpid()] = names;
 	
 	list<pair<unsigned int, unsigned int> > virtual_links;
 	for(int i = 0; i < vlinks_number; i++)
 	{
	 	pair<unsigned int, unsigned int> ids;
	 	try
	 	{
	 		ids = NodeOrchestrator::createVirtualLink(lsi.getDpid(),vlinks_remote_dpid);
	 	}catch(...)
	 	{
	 		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Command \"create-lsi\" failed");
			return createErrorMessage(string(CREATE_LSI), "An error occurred while creating a virtual link");
	 	}
	 	virtual_links.push_back(ids);
    }
 	 
	return createLSIAnswer(lsi,nfPorts,virtual_links);
}

string MessageHandler::createLSIAnswer(LSI lsi, map<string,map<string,uint32_t> > nfPorts,list<pair<unsigned int, unsigned int> > virtual_links)
{
	Object json;
	
	json["command"] = CREATE_LSI;
	json["status"] = "ok";	
	
	json["lsi-id"] = lsi.getDpid();
	
	Array ports_array;
	map<string,unsigned int> ports = lsi.getPorts();
	map<string,unsigned int>::iterator p = ports.begin();
	for(; p != ports.end(); p++)
	{
		Object port;
		port["name"] = p->first;
		port["id"] = p->second;
		ports_array.push_back(port);
	}	
	if(ports.size() > 0)
		json["ports"] = ports_array;
	
	Array nfs_array;
	map<string,map<string,uint32_t> >::iterator nf = nfPorts.begin();
	for(; nf != nfPorts.end(); nf++)
	{
		Object network_function;
		network_function["name"] = nf->first;
		Array ports_array;
		map<string,uint32_t> ports = nf->second;
		for(map<string,uint32_t>::iterator p = ports.begin(); p != ports.end(); p++)
		{
			Object port;
			port["name"] = p->first;
			port["id"] = p->second;
			ports_array.push_back(port);		
		}
		network_function["ports"] = ports_array;
		
		nfs_array.push_back(network_function);
	}
	if(nfPorts.size() > 0)
		json["network-functions"] = nfs_array;
	
	Array vlinks_array;
	list<pair<unsigned int, unsigned int> >::iterator vl = virtual_links.begin();
	for(; vl != virtual_links.end(); vl++)
	{
		Object virtual_link;
		virtual_link["local-id"] = vl->first;
		virtual_link["remote-id"] = vl->second;
		vlinks_array.push_back(virtual_link);
	}
	if(virtual_links.size() > 0)
		json["virtual-links"] = vlinks_array;
	
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

string MessageHandler::destroyLSI(string message)
{	
	bool foundLsiID = false;
	uint64_t lsiID = 0;
		
	Value value;
    read( message, value );
	Object obj = value.getObject();
		
	for( Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
    {
        const string& name  = i->first;
        const Value&  value = i->second;

        if( name == "command" )
        {
			continue;
        }
        else if( name == "lsi-id")
        {
        	foundLsiID = true;
			lsiID = value.getInt(); //FIXME: it isn't an int!
        }
    }
    
    if(!foundLsiID)
    {
    	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"destroy-lsi\" without field \"lsi-id\"");
    	return createErrorMessage(string(DESTROY_LSI), string("Command without lsi-id"));
    }
 
 	try
	{   
		NodeOrchestrator::destroyLSI(lsiID);
	
		//The NF ports must be destroyed manually
		list<string> names = nfPortNames[lsiID];		
		for(list<string>::iterator n = names.begin(); n != names.end(); n++)
			NodeOrchestrator::destroyNfPort(lsiID,*n,false);
		nfPortNames.erase(lsiID);
	} catch (...)
	{
		return createErrorMessage(string(DESTROY_LSI),string("error during the destruction of the LSI"));
	}
	 	 
	//Create the answer
	Object json;
	
	json["command"] = DESTROY_LSI;
	json["status"] = "ok";	

	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

string MessageHandler::attachPhyPorts(string message)
{
	bool foundLsiID = false;
	bool foundPorts = false;

	list<string> physicalPorts;
	uint64_t lsiID = 0;
	
	Value value;
    read( message, value );
	Object obj = value.getObject();
		
	for( Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
    {
        const string& name  = i->first;
        const Value&  value = i->second;

        if( name == "command" )
        {
			continue;
        }
        else if( name == "lsi-id")
        {
        	foundLsiID = true;
			lsiID = value.getInt(); //FIXME: it isn't an int!
        }
        else if( name == "ports" )
        {
        	foundPorts = true;
			const Array& ports_array = value.getArray();
			for( unsigned int i = 0; i < ports_array.size(); ++i )
			{
				physicalPorts.push_back( ports_array[i].getString() );
			}
        }
    }
    
    if(!foundLsiID || !foundPorts)
    {
    	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"attach-phy-ports\" without field \"ports\" or \"lsi-id\" or \"both\"");
    	return createErrorMessage(string(ATTACH_PHY_PORTS), string("Command without ports, lsi-id, or both"));
    }
	
	map<string,unsigned int> ports;    
    for(list<string>::iterator port =  physicalPorts.begin(); port != physicalPorts.end(); port++)
    {
    	unsigned int portID = 0;
    	try
    	{
			portID = NodeOrchestrator::attachPhyPort(lsiID,*port);
		}catch(...)
		{
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] Command \"attach-phy-ports\" failed");
			stringstream ss;
			ss << "An error occurred while attaching the physical port " << *port;
			return createErrorMessage(string(ATTACH_PHY_PORTS), ss.str());
		}
		ports[*port] = portID;
    }
 
 	//Prepare the answer
	Object json;
	
	json["command"] = ATTACH_PHY_PORTS;
	json["status"] = "ok";	
	
	Array ports_array;
	map<string,unsigned int>::iterator p = ports.begin();
	for(; p != ports.end(); p++)
	{
		Object port;
		port["name"] = p->first;
		port["id"] = p->second;
		ports_array.push_back(port);
	}	
	if(ports.size() > 0)
		json["ports"] = ports_array;
	
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

string MessageHandler::detachPhyPorts(string message)
{
	bool foundLsiID = false;
	bool foundPorts = false;

	list<string> physicalPorts;
	uint64_t lsiID = 0;
	
	Value value;
    read( message, value );
	Object obj = value.getObject();
		
	for( Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
    {
        const string& name  = i->first;
        const Value&  value = i->second;

        if( name == "command" )
        {
			continue;
        }
        else if( name == "lsi-id")
        {
        	foundLsiID = true;
			lsiID = value.getInt(); //FIXME: it isn't an int!
        }
        else if( name == "ports" )
        {
        	foundPorts = true;
			const Array& ports_array = value.getArray();
			for( unsigned int i = 0; i < ports_array.size(); ++i )
			{
				physicalPorts.push_back( ports_array[i].getString() );
			}
        }
    }
    
    if(!foundLsiID || !foundPorts)
    {
    	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"detach-phy-ports\" without field \"ports\" or \"lsi-id\" or \"both\"");
    	return createErrorMessage(string(DETACH_PHY_PORTS), string("Command without ports, lsi-id, or both"));
    }

    for(list<string>::iterator port =  physicalPorts.begin(); port != physicalPorts.end(); port++)
    {
    	if(!NodeOrchestrator::detachPort(lsiID,*port,false))
 		{
 			stringstream ss;
 			ss << "An error occurred while detaching the port " << *port << " from LSI " << lsiID;
 			return createErrorMessage(string(DETACH_PHY_PORTS), ss.str());
 		}
    }
 
 	//Prepare the answer
	Object json;
	json["command"] = DETACH_PHY_PORTS;
	json["status"] = "ok";		
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

string MessageHandler::createNFPorts(string message)
{
	list< pair<string,list<string> > > networkFunctions; //list <nf name, list <port name> >
	map<string,PexType> nfTypes; //list of <nf_name, nf type>
	map<string,map<string,uint32_t> > nfPorts; //map <nf name, map <port name, port id> >

	bool foundLsiID = false;
	uint64_t lsiID;
	
	Value value;
    read( message, value );
	Object obj = value.getObject();
		
	for( Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
    {
        const string& name  = i->first;
        const Value&  value = i->second;

        if( name == "command" )
        {
			continue;
        }
        else if( name == "lsi-id")
        {
        	foundLsiID = true;
			lsiID = value.getInt(); //FIXME: it isn't an int!
        }
		else if( name == "network-functions")
    	{
			const Array& nfs_array = value.getArray();
			
			for( unsigned int i = 0; i < nfs_array.size(); ++i )
			{
				Object nfs = nfs_array[i].getObject();
				
				bool foundName = false;
				bool foundType = false;
        		bool foundPorts = false;
        		list<string> ports;
				string name;
				PexType nfType;

				for( Object::const_iterator j = nfs.begin(); j != nfs.end(); ++j )
				{
					const string& nf_name  = j->first;
					const Value&  nf_value = j->second;	
									
					if(nf_name == "name")
					{
						name = nf_value.getString();
						foundName = true;
					}
					else if(nf_name == "type")
					{
						string tmp = nf_value.getString();
						if(tmp != "dpdk" && tmp != "docker")
						{
							ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-lsi\" with a network function with a wrong \"type\"");
						return createErrorMessage(string(CREATE_NF_PORTS), string(" Received command \"create-lsi\" with a network function with a wrong \"type\""));
						}
						
						nfType = (tmp == "dpdk")? DPDK_SECONDARY : DPDK_SECONDARY;
						foundType = true;
					}
					else if(nf_name == "ports")
					{
						const Array& ports_array = nf_value.getArray();
						for( unsigned int i = 0; i < ports_array.size(); ++i )
						{
							ports.push_back( ports_array[i].getString() );
						}
						if(ports.size() > 0)
							foundPorts = true;
					}
				
				}
				if(!foundName || !foundPorts || !foundType)
				{
					ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-nf-ports\" with a network function without the \"name\", the \"ports\", the \"type\", all of them");
					return createErrorMessage(string(CREATE_NF_PORTS), string(" Received command \"create-nf-ports\" with a network function without the \"name\", the \"ports\", the \"type\", all of them"));
				}
				
				networkFunctions.push_back(make_pair(name,ports));
				nfTypes[name] = nfType;
			}
        }
    }
    
    if(!foundLsiID)
    {
    	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-nf-ports\" without field \"lsi-id\"");
    	return createErrorMessage(string(CREATE_NF_PORTS), string("Command without lsi-id"));
    }
	
	list<string> names = nfPortNames[lsiID];
	for(list< pair<string,list<string> > >::iterator it = networkFunctions.begin(); it != networkFunctions.end(); it++)
 	{	
 		stringstream nfName;
 		nfName << lsiID << "_" << it->first;
 		
 		list<string> ports = it->second;
 		map<string,unsigned int> port_id;
 		for(list<string>::iterator p = ports.begin(); p != ports.end(); p++)
 		{
	 		stringstream portName;
	 		portName << lsiID << "_" << *p;
	 		names.push_back(portName.str());
	 		try
	 		{
	 			port_id[*p] = NodeOrchestrator::createNfPort(lsiID, nfName.str(), portName.str(),nfTypes[it->first]);
		 	}catch(...)
		 	{
		 		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Command \"create-nf-ports\" failed");
				stringstream ss;
				ss << "An error occurred while creating/attaching the NF port " << portName.str();
				return createErrorMessage(string(CREATE_LSI), ss.str());	
		 	}
	 	}
	 	nfPorts[it->first] = port_id;
 	} 
 	nfPortNames.erase(lsiID);
 	nfPortNames[lsiID] = names;

	//Prepare the answer
	Object json;
	
	json["command"] = CREATE_NF_PORTS;
	json["status"] = "ok";
	
	Array nfs_array;
	map<string,map<string,uint32_t> >::iterator nf = nfPorts.begin();
	for(; nf != nfPorts.end(); nf++)
	{
		Object network_function;
		network_function["name"] = nf->first;
		Array ports_array;
		map<string,uint32_t> ports = nf->second;
		for(map<string,uint32_t>::iterator p = ports.begin(); p != ports.end(); p++)
		{
			Object port;
			port["name"] = p->first;
			port["id"] = p->second;
			ports_array.push_back(port);		
		}
		network_function["ports"] = ports_array;
		
		nfs_array.push_back(network_function);
	}
	if(nfPorts.size() > 0)
		json["network-functions"] = nfs_array;
		
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

string MessageHandler::destroyNFPorts(string message)
{
	bool foundLsiID = false;
	bool foundPorts = false;
	uint64_t lsiID = 0;
	list<string> ports;
		
	Value value;
    read( message, value );
	Object obj = value.getObject();
		
	for( Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
    {
        const string& name  = i->first;
        const Value&  value = i->second;

        if( name == "command" )
        {
			continue;
        }
        else if( name == "lsi-id")
        {
        	foundLsiID = true;
			lsiID = value.getInt(); //FIXME: it isn't an int!
        }
        else if( name == "ports")
        {
        	foundPorts = true;
      		const Array& ports_array = value.getArray();
			
			if(ports_array.size() == 0)
			{
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"destroy-nf-ports\" with an empty \"ports\" list");
		    	return createErrorMessage(string(DESTROY_NF_PORTS), string("Command with an empty ports list"));	
			}
			
			for( unsigned int i = 0; i < ports_array.size(); ++i )
			{
				string port = ports_array[i].getString();
				ports.push_back(port);
			}
        }
    }
    
    if(!foundLsiID || !foundPorts)
    {
    	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"destroy-nf-ports\" without field \"lsi-id\", or the field \"ports\", or both");
    	return createErrorMessage(string(DESTROY_NF_PORTS), string("Command without lsi-id, ports or both"));
    }
 
 	for(list<string>::iterator port = ports.begin(); port != ports.end(); port++)
 	{
 	
		list<string> names = nfPortNames[lsiID];		
		for(list<string>::iterator n = names.begin(); n != names.end(); n++)
		{
			if(*n == *port)
			{
				names.erase(n);
				break;
			}
 		}
 		nfPortNames.erase(lsiID);
 		nfPortNames[lsiID] = names;
 	
 		if(!NodeOrchestrator::destroyNfPort(lsiID,*port))
 		{
 			stringstream ss;
 			ss << "An error occurred while destroying port " << *port;
 			return createErrorMessage(string(DESTROY_NF_PORTS), ss.str());
 		}
 	}
 	
 	//Create the answer
 	Object json;
	json["command"] = DESTROY_NF_PORTS;
	json["status"] = "ok";		
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

string MessageHandler::createVirtualLinks(string message)
{	
	int vlinks_number = 0;
    uint64_t dpid_a = 0, dpid_b;
	
	bool foundNumber = false;
	bool foundDpIDa = false;
	bool foundDpIDb = false;
	
	Value value;
    read( message, value );
	Object obj = value.getObject();
		
	for( Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
    {
        const string& name  = i->first;
        const Value&  value = i->second;

        if( name == "command" )
        {
			continue;
        }
		if( name == "number" )
		{
			vlinks_number = value.getInt();
			foundNumber = true;
		}
		else if (name == "lsi-a")
		{
			dpid_a = value.getInt(); //FIXME: the dpid is actually a uint64_t
			foundDpIDa = true;
		}
		else if (name == "lsi-b")
		{
			dpid_b = value.getInt(); //FIXME: the dpid is actually a uint64_t
			foundDpIDb = true;
		}
	}
	if(!foundNumber || !foundDpIDa || !foundDpIDb)
	{
		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-virtual-links\" without sub-fields \"number\", \"lsi-a\", \"lsi-b\" or may of them");
		return createErrorMessage(string(CREATE_VIRTUAL_LINKS), string("Received command \"create-virtual-links\" without sub-fields \"number\", \"lsi-a\", \"lsi-b\" or may of them"));
	}
    
 	list<pair<unsigned int, unsigned int> > virtual_links;
 	for(int i = 0; i < vlinks_number; i++)
 	{
	 	pair<unsigned int, unsigned int> ids;
	 	try
	 	{
	 		ids = NodeOrchestrator::createVirtualLink(dpid_a,dpid_b);
	 	}catch(...)
	 	{
	 		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Command \"create-virtual-links\" failed");
			return createErrorMessage(string(CREATE_VIRTUAL_LINKS), "An error occurred while creating a virtual link");
	 	}
	 	virtual_links.push_back(ids);
    }

	//Prepare the answer
	Object json;
	
	json["command"] = CREATE_VIRTUAL_LINKS;
	json["status"] = "ok";	
	
	Array vlinks_array;
	list<pair<unsigned int, unsigned int> >::iterator vl = virtual_links.begin();
	for(; vl != virtual_links.end(); vl++)
	{
		Object virtual_link;
		virtual_link["id-a"] = vl->first;
		virtual_link["id-b"] = vl->second;
		vlinks_array.push_back(virtual_link);
	}
	if(virtual_links.size() > 0)
		json["virtual-links"] = vlinks_array;
	
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

string MessageHandler::destroyVirtualLinks(string message)
{
	bool foundVlinks = false;
	list<pair<uint64_t,uint64_t> > virtual_links;
		
	Value value;
    read( message, value );
	Object obj = value.getObject();
		
	for( Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
    {
        const string& name  = i->first;
        const Value&  value = i->second;

        if( name == "command" )
        {
			continue;
        }
        else if( name == "virtual-links")
        {
        	foundVlinks = true;
      		const Array& vlinks_array = value.getArray();
			
			if(vlinks_array.size() == 0)
			{
				ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"destroy-virtual-links\" with an empty \"virtual-links\" list");
		    	return createErrorMessage(string(DESTROY_VIRTUAL_LINKS), string("Command with an empty virtual links list"));	
			}
			
			for( unsigned int i = 0; i < vlinks_array.size(); ++i )
			{
				Object vlink = vlinks_array[i].getObject();
				
				bool foundLsiID = false;
				bool foundLinkID = false;
				
				uint64_t lsiID = 0;
				uint64_t vlinkID = 0;
				
				for( Object::const_iterator v = vlink.begin(); v != vlink.end(); ++v )
    			{			
					const string& v_name  = v->first;
		        	const Value&  v_value = v->second;
		        	
		        	if( v_name == "lsi-id" )
		        	{
		        		foundLsiID = true;
		        		lsiID = v_value.getInt(); //FIXME: it's not an int!
		        	}
		        	else if( v_name == "vlink-id" )
		        	{
		        		foundLinkID = true;
		        		vlinkID = v_value.getInt(); //FIXME: it's not an int!
		        	}
		        }
		        
		        if(!foundLsiID || !foundLinkID)
				{
					ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"destroy-virtual-links\" with a virtual link without field \"lsi-id\", \"vlink-id\", or both");
					return createErrorMessage(string(DESTROY_VIRTUAL_LINKS), string("Command with a virtual-links without the lsi-id, the vlink-id, or both"));
				}
				virtual_links.push_back(make_pair(lsiID,vlinkID));
			}
        }
    }
    
    if(!foundVlinks)
    {
    	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"destroy-virtual-links\" without field \"virtual-links\"");
    	return createErrorMessage(string(DESTROY_VIRTUAL_LINKS), string("Command without virtual-links"));
    }
 
 	for(list<pair<uint64_t, uint64_t> >::iterator vlink = virtual_links.begin(); vlink != virtual_links.end(); vlink++)
 	{
 		if(!NodeOrchestrator::detachPort(vlink->first,vlink->second,true))
 		{
 			stringstream ss;
 			ss << "An error occurred while destroying the virtual link - lsi-id: " << vlink->first << " - vlink-id: " << vlink->second;
 			return createErrorMessage(string(DESTROY_VIRTUAL_LINKS), ss.str());
 		}
 	}
 	
 	//Create the answer
 	Object json;
	json["command"] = DESTROY_VIRTUAL_LINKS;
	json["status"] = "ok";		
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}


string MessageHandler::discoverPhyPorts(string message)
{
	list<string> ports = NodeOrchestrator::discoverPhyPorts();
	
	Object json;
	
	json["command"] = DISCOVER_PHY_PORTS;
	json["status"] = "ok";	
	
	Array ports_array;
	for(list<string>::iterator p = ports.begin(); p != ports.end(); p++)
		ports_array.push_back(*p);
	
	json["ports"] = ports_array;
	
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

string MessageHandler::createErrorMessage(string command, string message)
{
	//ERROR
	Object json;
	json["command"] = command.c_str();
	json["status"] = "error";
	json["message"] = message.c_str();
	
	stringstream ss;
	write_formatted(json, ss );
	return ss.str();
}

}
