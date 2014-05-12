#include "server.h"

namespace xdpd
{

void *Server::listen(void *param)
{

	int AddressFamily = AF_INET; //use IPv4
	int TransportProtocol = SOCK_STREAM; //use TCP

	char ErrBuf[1024];
	char DataBuffer[1024];
	int ChildSocket;				// keeps the socket ID for connections from clients
	struct addrinfo Hints;			// temporary struct to keep settings needed to open the new socket
	struct addrinfo *AddrInfo;		// keeps the addrinfo chain; required to open a new socket
	struct sockaddr_storage From;	// temp variable that keeps the parameters of the incoming connection
	int ReadBytes, WrittenBytes;
	int ServerSocket;

	// Prepare to open a new server socket
	memset(&Hints, 0, sizeof(struct addrinfo));

	Hints.ai_family= AddressFamily;
	Hints.ai_socktype= TransportProtocol;	// Open a TCP/UDP connection
	Hints.ai_flags = AI_PASSIVE;			// This is a server: ready to bind() a socket
	
	if (sock_initaddress (NULL, LISTEN_ON_PORT, &Hints, &AddrInfo, ErrBuf, sizeof(ErrBuf)) == sockFAILURE)
	{
		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Error resolving given address/port (%s/%s): %s", LISTEN_ON_PORT, ErrBuf);
		return NULL;
	}

	if ( (ServerSocket= sock_open(AddrInfo, 1, 10,  ErrBuf, sizeof(ErrBuf))) == sockFAILURE)
	{
		// AddrInfo is no longer required
		sock_freeaddrinfo(AddrInfo);
		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Cannot opening the socket: %s", ErrBuf);
		return NULL;
	}

	// AddrInfo is no longer required
	sock_freeaddrinfo(AddrInfo);

	while(1)
	{
		if ( (ChildSocket= sock_accept(ServerSocket, &From, ErrBuf, sizeof(ErrBuf))) == sockFAILURE)
		{
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] error when accepting a new connection: %s", ErrBuf);
			continue;
		}

		ReadBytes= sock_recv(ChildSocket, DataBuffer, sizeof(DataBuffer), SOCK_RECEIVEALL_NO, 0/*no timeout*/, ErrBuf, sizeof(ErrBuf));
		if (ReadBytes == sockFAILURE)
		{
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] Error reading data: %s", ErrBuf);
			continue;
		}

		// Terminate buffer, just for printing purposes
		// Warning: this can originate a buffer overflow
		DataBuffer[ReadBytes]= 0;

		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Data received (%d bytes):\n",ReadBytes);
		ROFL_INFO("[xdpd]["PLUGIN_NAME"] %s\n",DataBuffer);

		//FIXME: the message could contain a different command
		string message = processCommand(string(DataBuffer));
		const char *answer = message.c_str();
		
		ROFL_INFO("[xdpd]["PLUGIN_NAME"] Answer to be sent: %s\n",answer);

		WrittenBytes= sock_send(ChildSocket, answer, strlen(answer), ErrBuf, sizeof(ErrBuf));
		if (WrittenBytes == sockFAILURE)
		{
			ROFL_INFO("[xdpd]["PLUGIN_NAME"] Error sending data: %s", ErrBuf);

		}

		sock_close(ChildSocket,ErrBuf,sizeof(ErrBuf));
	}

	return NULL;
}

string Server::processCommand(string message)
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
			if(command == "create-lsi")
				return createLSI(message);
			if(command == "discover-physical-ports")
				return discoverPhyPorts(message);
			
			//ERROR
			break;
        }
    }

	//ERROR

	Object json;
	json["command"] = "ERROR";
	json["status"] = "error";
	json["message"] = "Unknown command";
	
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

string Server::createLSI(string message)
{
	list<string> physicalPorts;
	list<string> networkFunctions;	
	map<string,uint32_t> nfPorts;
	
	string controllerAddress;
	string controllerPort;
	bool foundControllerAddress = false;
	bool foundControllerPort = false;
	
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
				networkFunctions.push_back( nfs_array[i].getString() );
			}
        }
    }
    
    if(!foundControllerAddress || !foundControllerPort)
    {
    	ROFL_INFO("[xdpd]["PLUGIN_NAME"] Received command \"create-lsi\" without field \"port\" or \"address\" or \"both\"");
    	
		Object json;
		json["command"] = "create-lsi";
		json["status"] = "error";
		json["message"] = "Command without port, address, or both";
	
		stringstream ss;
 		write_formatted(json, ss );
    	
    	return ss.str();
    }
    
	LSI lsi = NodeOrchestrator::createLSI(physicalPorts,controllerAddress,controllerPort);
	for(list<string>::iterator it = networkFunctions.begin(); it != networkFunctions.end(); it++)
 	{	
 		stringstream portName;
 		portName << lsi.getDpid() << "_" << *it;
 		nfPorts[*it] = NodeOrchestrator::createNfPort(lsi.getDpid(), portName.str(),DPDK);
 	}  
	return string("dummy");
}

string Server::discoverPhyPorts(string message)
{
	list<string> ports = NodeOrchestrator::discoverPhyPorts();
	
	Object json;
	
	json["command"] = "discover-physical-interfaces";
	json["status"] = "ok";	
	
	Array ports_array;
	for(list<string>::iterator p = ports.begin(); p != ports.end(); p++)
		ports_array.push_back(*p);
	
	json["ports"] = ports_array;
	
	stringstream ss;
 	write_formatted(json, ss );
 	
 	return ss.str();
}

}
