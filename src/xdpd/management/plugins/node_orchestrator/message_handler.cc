#include "message_handler.h"

namespace xdpd
{

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
        		bool foundPorts = false;
        		list<string> ports;
				string name;

				for( Object::const_iterator j = nfs.begin(); j != nfs.end(); ++j )
				{
					const string& nf_name  = j->first;
					const Value&  nf_value = j->second;	
									
					if(nf_name == "name")
					{
						name = nf_value.getString();
						foundName = true;
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
				if(!foundName || !foundPorts)
				{
					ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-lsi\" with a network function without the \"name\", the \"ports\", or both");
					return createErrorMessage(string(CREATE_LSI), string(" Received command \"create-lsi\" with a network function without the \"name\", the \"ports\", or both"));
				}
				
				networkFunctions.push_back(make_pair(name,ports));
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
	 		port_id[*p] = NodeOrchestrator::createNfPort(lsi.getDpid(), nfName.str(), portName.str(),DPDK);
	 	}
	 	nfPorts[it->first] = port_id;
 	} 
 	
 	list<pair<unsigned int, unsigned int> > virtual_links;
 	for(int i = 0; i < vlinks_number; i++)
 	{
	 	pair<unsigned int, unsigned int> ids = NodeOrchestrator::createVirtualLink(lsi.getDpid(),vlinks_remote_dpid);
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
 
 	LSI lsi;
 	try
	{   
		NodeOrchestrator::destroyLSI(lsiID);
	} catch (...)
	{
		return createErrorMessage(string(CREATE_LSI),string("error during the destruction of the LSI"));
	}
	 	 
	//Create the answer
	Object json;
	
	json["command"] = DESTROY_LSI;
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
