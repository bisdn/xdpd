#include "port_manager.h"
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;


void port_manager::check_port_existance(std::string port_name) throw (ePmInvalidPort){
	switch_port_t* port;
	
	if( (port = fwd_module_get_port_by_name(port_name.c_str())) == NULL)
		throw ePmInvalidPort();
}

//System port admin commands 
void port_manager::enable_port(std::string port_name) throw (ePmInvalidPort, eOfSmGeneralError){

	//Check
	check_port_existance(port_name);	

	if(fwd_module_enable_port(port_name.c_str()) != AFA_SUCCESS)
		throw eOfSmGeneralError();	

	ROFL_INFO("[port_manager] Port %s enabled\n", port_name.c_str());
}

void port_manager::disable_port(std::string port_name) throw (ePmInvalidPort, eOfSmGeneralError){

	//Check
	check_port_existance(port_name);

	if(fwd_module_disable_port(port_name.c_str()) != AFA_SUCCESS)
		throw eOfSmGeneralError();	
	
	ROFL_INFO("[port_manager] Port %s disabled\n", port_name.c_str());
}

//Port attachment/detachment
void port_manager::attach_port_to_switch(uint64_t dpid, std::string port_name, unsigned int *of_port_num) throw (eOfSmDoesNotExist, ePmInvalidPort, eOfSmGeneralError){

	unsigned int i=0;

	//Check port existance
	check_port_existance(port_name);

	//Check DP existance
	if(!switch_manager::find_by_dpid(dpid))
		throw eOfSmDoesNotExist();	

	if (fwd_module_attach_port_to_switch(dpid,port_name.c_str(),&i) != AFA_SUCCESS)
		throw eOfSmGeneralError();

	ROFL_INFO("[port_manager] Port %s attached to switch with dpid 0x%llx\n", port_name.c_str(), (long long unsigned)dpid);
	*of_port_num = i;
}

//Port attachment/detachment
void port_manager::connect_switches(uint64_t dpid_lsi1, std::string& port_name1, uint64_t dpid_lsi2, std::string& port_name2) throw (eOfSmDoesNotExist, ePmInvalidPort, eOfSmGeneralError){

	switch_port_t *port1 = NULL, *port2 = NULL;

	//Check lsi existance 
	if(!switch_manager::find_by_dpid(dpid_lsi1) || !switch_manager::find_by_dpid(dpid_lsi2) )
		throw eOfSmDoesNotExist();	

	if (fwd_module_connect_switches(dpid_lsi1, &port1, dpid_lsi2, &port2) != AFA_SUCCESS)
		throw eOfSmGeneralError();
	
	//Copy port names
	port_name1 = std::string(port1->name);
	port_name2 = std::string(port2->name);

	ROFL_INFO("[port_manager] Link created between switch with dpid 0x%llx and 0x%llx, with virtual interface names %s and %s respectively \n", (long long unsigned)dpid_lsi1, (long long unsigned)dpid_lsi2, port1->name, port2->name);
}

void port_manager::detach_port_from_switch(uint64_t dpid, std::string port_name) throw (eOfSmDoesNotExist, ePmInvalidPort, eOfSmGeneralError){

	if (fwd_module_detach_port_from_switch(dpid,port_name.c_str()) != AFA_SUCCESS)
		throw eOfSmGeneralError();
	
}

void port_manager::detach_port_from_switch_by_num(uint64_t dpid, unsigned int port_num) throw (eOfSmDoesNotExist, ePmInvalidPort, eOfSmGeneralError){

	if (fwd_module_detach_port_from_switch_at_port_num(dpid,port_num) != AFA_SUCCESS)
		throw eOfSmGeneralError();
}

std::list<std::string> port_manager::list_available_port_names() throw (eOfSmGeneralError){

	unsigned int i, max_ports;
	switch_port_t** ports;

	std::list<std::string> port_name_list;
	
	//Call the forwarding module to list the ports
	ports = fwd_module_get_physical_ports(&max_ports);
	
	if(!ports)
		throw eOfSmGeneralError();

	//Run over the ports and get the name
	for(i=0;i<max_ports;i++){
		if(ports[i])
			port_name_list.push_back(std::string(ports[i]->name));
			
	}

	//Call the forwarding module to list the ports
	ports = fwd_module_get_virtual_ports(&max_ports);
	
	//Run over the ports and get the name
	for(i=0;i<max_ports;i++){
		if(ports[i])
			port_name_list.push_back(std::string(ports[i]->name));
	}

	//TODO: add tunnel.
	
	return port_name_list; 
}


