#include "port_manager.h"

#include "../openflow/openflow_switch.h"
#include <rofl/datapath/afa/cmm.h>

//FIXME remove this dependency
#include <rofl/datapath/pipeline/physical_switch.h>

using namespace rofl;


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

}

void port_manager::disable_port(std::string port_name) throw (ePmInvalidPort, eOfSmGeneralError){

	//Check
	check_port_existance(port_name);

	if(fwd_module_disable_port(port_name.c_str()) != AFA_SUCCESS)
		throw eOfSmGeneralError();	
}

//Port attachment/detachment
void port_manager::attach_port_to_switch(uint64_t dpid, std::string port_name, unsigned int *of_port_num) throw (eOfSmDoesNotExist, ePmInvalidPort, eOfSmGeneralError){

	unsigned int i=0;

	//Check port existance
	check_port_existance(port_name);

	//Check DP existance
	//TODO

	if (fwd_module_attach_port_to_switch(dpid,port_name.c_str(),&i) != AFA_SUCCESS)
		throw eOfSmGeneralError();

	*of_port_num = i;
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

	//TODO: add virtual and tunnel. Calls already available in the AFA
	
	return port_name_list; 
}

/**
*
* Dispatching of version agnostic messages comming from the driver
*
*/

afa_result_t cmm_notify_port_add(switch_port_t* port){
	
	openflow_switch* sw;
	
	if (!port || !port->attached_sw){
		return AFA_FAILURE;
	}
	if( (sw=switch_manager::find_by_dpid(port->attached_sw->dpid)) == NULL)
		return AFA_FAILURE;	

	//Notify MGMT framework
	//TODO:

	return sw->notify_port_add(port);
}


afa_result_t cmm_notify_port_delete(switch_port_t* port){
	
	openflow_switch* sw;
	
	if (!port || !port->attached_sw){
		return AFA_FAILURE;
	}
	if( (sw=switch_manager::find_by_dpid(port->attached_sw->dpid)) == NULL)
		return AFA_FAILURE;	

	//Notify MGMT framework
	//TODO:

	return sw->notify_port_delete(port);
}


afa_result_t cmm_notify_port_status_changed(switch_port_t* port){
	
	openflow_switch* sw;
	
	if (!port || !port->attached_sw){
		return AFA_FAILURE;
	}
	if( (sw=switch_manager::find_by_dpid(port->attached_sw->dpid)) == NULL)
		return AFA_FAILURE;	

	//Notify MGMT framework
	//TODO:

	return sw->notify_port_status_changed(port);
}
