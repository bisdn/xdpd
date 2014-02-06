#include "port_manager.h"
#include <rofl/common/utils/c_logger.h>
#include "switch_manager.h"

using namespace rofl;
using namespace xdpd;

pthread_mutex_t port_manager::mutex = PTHREAD_MUTEX_INITIALIZER; 

//
// Basic
//
bool port_manager::port_exists(std::string& port_name){
	return fwd_module_port_exists(port_name.c_str());
}
std::list<std::string> port_manager::list_available_port_names(){

	unsigned int i;
	switch_port_name_list_t* port_names;
	std::list<std::string> port_name_list;
	
	//Call the forwarding module to list the ports
	port_names = fwd_module_get_all_port_names();
	
	if(!port_names)
		throw eOfSmGeneralError();

	//Run over the ports and get the name
	for(i=0;i<port_names->num_of_ports;i++)
		port_name_list.push_back(std::string(port_names->names[i].name));

	//Destroy the list of ports
	switch_port_name_list_destroy(port_names);
	
	return port_name_list; 
}

//
//Port operations
//
bool port_manager::get_admin_state(std::string& name){

	bool state;

	//Get snapshot
	switch_port_snapshot_t* port_snapshot = fwd_module_get_port_snapshot_by_name(name.c_str());
	state = port_snapshot->up;	
	//Destroy
	switch_port_destroy_snapshot(port_snapshot);	

	return state;
}

void port_manager::bring_up(std::string& name){

	afa_result_t result;

	//Serialize . This is not strictly necessary, but prevents
	//a lot of inconvenient notifications in case of concurrency.
	pthread_mutex_lock(&port_manager::mutex);
	result = fwd_module_bring_port_up(name.c_str());
	pthread_mutex_unlock(&port_manager::mutex);

	if(result != AFA_SUCCESS)
	       throw ePmUnknownError();
	ROFL_INFO("[port_manager] Port %s brought administratively up\n", name.c_str());
}

void port_manager::bring_down(std::string& name){

	afa_result_t result;

	//Serialize . This is not strictly necessary, but prevents
	//a lot of inconvenient notifications in case of concurrency.
	pthread_mutex_lock(&port_manager::mutex);
	result = fwd_module_bring_port_down(name.c_str());
	pthread_mutex_unlock(&port_manager::mutex);

	if(result != AFA_SUCCESS)
		throw ePmUnknownError();      
	ROFL_INFO("[port_manager] Port %s brought administratively down\n", name.c_str());
}


//
//Port attachment/detachment
//

//Port attachment/detachment
void port_manager::attach_port_to_switch(uint64_t dpid, std::string& port_name, unsigned int *of_port_num){

	unsigned int i=0;

	//Check port existance
	if(!port_exists(port_name))
		throw ePmInvalidPort();

	//Check DP existance
	if(!switch_manager::exists(dpid))
		throw eOfSmDoesNotExist();	

	if (fwd_module_attach_port_to_switch(dpid,port_name.c_str(),&i) != AFA_SUCCESS)
		throw eOfSmGeneralError();

	ROFL_INFO("[port_manager] Port %s attached to switch with dpid 0x%llx\n", port_name.c_str(), (long long unsigned)dpid);
	*of_port_num = i;
}

//Port attachment/detachment
void port_manager::connect_switches(uint64_t dpid_lsi1, std::string& port_name1, uint64_t dpid_lsi2, std::string& port_name2){

	switch_port_t *port1 = NULL, *port2 = NULL;

	//Check lsi existance 
	if(!switch_manager::exists(dpid_lsi1) || !switch_manager::exists(dpid_lsi2) )
		throw eOfSmDoesNotExist();	

	if (fwd_module_connect_switches(dpid_lsi1, &port1, dpid_lsi2, &port2) != AFA_SUCCESS)
		throw eOfSmGeneralError();
	
	//Copy port names
	port_name1 = std::string(port1->name);
	port_name2 = std::string(port2->name);

	ROFL_INFO("[port_manager] Link created between switch with dpid 0x%llx and 0x%llx, with virtual interface names %s and %s respectively \n", (long long unsigned)dpid_lsi1, (long long unsigned)dpid_lsi2, port1->name, port2->name);
	
	//Destroy copies
	switch_port_destroy_snapshot(port1);	
	switch_port_destroy_snapshot(port2);	
}

void port_manager::detach_port_from_switch(uint64_t dpid, std::string& port_name){
	if (fwd_module_detach_port_from_switch(dpid,port_name.c_str()) != AFA_SUCCESS)
		throw eOfSmGeneralError();
}

void port_manager::detach_port_from_switch_by_num(uint64_t dpid, unsigned int port_num){
	if (fwd_module_detach_port_from_switch_at_port_num(dpid,port_num) != AFA_SUCCESS)
		throw eOfSmGeneralError();
}
