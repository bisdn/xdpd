#include "port_manager.h"
#include <assert.h>
#include "switch_manager.h"
#include "plugin_manager.h"

using namespace rofl;
using namespace xdpd;

pthread_mutex_t port_manager::mutex = PTHREAD_MUTEX_INITIALIZER; //Serialize operations 
pthread_rwlock_t port_manager::rwlock = PTHREAD_RWLOCK_INITIALIZER; //Used to protect the vlink cache 
std::map<std::string, std::string> port_manager::vlinks;

bool port_manager::port_exists(std::string& port_name){
	return hal_driver_port_exists(port_name.c_str());
}

//vlinks
bool port_manager::is_vlink(std::string& port_name){
	return port_manager::vlinks.find(port_name) != port_manager::vlinks.end();
}

std::string port_manager::get_vlink_pair(std::string& port_name){

	std::map<std::string, std::string>::iterator it;
	std::string pair;

	//Make sure iterator is readable all along	
	pthread_rwlock_rdlock(&port_manager::rwlock);
	
	it = port_manager::vlinks.find(port_name);
	

	if ( it == port_manager::vlinks.end()){
		pthread_rwlock_unlock(&port_manager::rwlock);
		throw ePmInvalidPort();
	}
	
	//Store the pair and release
	pair = it->second;

	pthread_rwlock_unlock(&port_manager::rwlock);

	return pair; 
}

std::list<std::string> port_manager::list_available_port_names(){

	unsigned int i;
	switch_port_name_list_t* port_names;
	std::list<std::string> port_name_list;
	
	//Call the driver to list the ports
	port_names = hal_driver_get_all_port_names();
	
	if(!port_names)
		throw eOfSmGeneralError();

	//Run over the ports and get the name
	for(i=0;i<port_names->num_of_ports;i++)
		port_name_list.push_back(std::string(port_names->names[i].name));

	//Destroy the list of ports
	switch_port_name_list_destroy(port_names);
	
	return port_name_list; 
}

void port_manager::get_port_info(std::string& name, port_snapshot& snapshot){

	(void)get_port_info; //Prevent unused warning
	
	switch_port_snapshot_t* s =  hal_driver_get_port_snapshot_by_name(name.c_str());
	if(!s)
		throw ePmInvalidPort();
	
	snapshot = port_snapshot(s);

	switch_port_destroy_snapshot(s);	
}

//
//Port operations
//
void port_manager::bring_up(std::string& name){

	hal_result_t result;

	//Serialize . This is not strictly necessary, but prevents
	//inconvenient interlacing of notifications in case of concurrency.
	pthread_mutex_lock(&port_manager::mutex);

	//Check port existance
	if(!port_exists(name)){
		pthread_mutex_unlock(&port_manager::mutex);
		throw ePmInvalidPort();
	}

	result = hal_driver_bring_port_up(name.c_str());
	pthread_mutex_unlock(&port_manager::mutex);

	if(result != HAL_SUCCESS)
	       throw ePmUnknownError();

	//Redundant, the driver should inform us about the change of state in the port
	ROFL_DEBUG("[xdpd][port_manager] Port %s brought administratively up\n", name.c_str());
}

void port_manager::bring_down(std::string& name){

	hal_result_t result;

	//Serialize . This is not strictly necessary, but prevents
	//inconvenient interlacing of notifications in case of concurrency.
	pthread_mutex_lock(&port_manager::mutex);

	//Check port existance
	if(!port_exists(name)){
		pthread_mutex_unlock(&port_manager::mutex);
		throw ePmInvalidPort();
	}

	result = hal_driver_bring_port_down(name.c_str());
	pthread_mutex_unlock(&port_manager::mutex);

	if(result != HAL_SUCCESS)
		throw ePmUnknownError();      
	
	//Redundant, the driver should inform us about the change of state in the port
	ROFL_DEBUG("[xdpd][port_manager] Port %s brought administratively down\n", name.c_str());
}


//
//Port attachment/detachment
//

void port_manager::attach_port_to_switch(uint64_t dpid, std::string& port_name, unsigned int* of_port_num){

	//Serialize
	pthread_mutex_lock(&port_manager::mutex);

	//Check port existance
	if(!port_exists(port_name)){
		pthread_mutex_unlock(&port_manager::mutex);
		ROFL_ERR("[xdpd][port_manager] ERROR: Attempting to attach a non-existent port %s to switch with dpid 0x%llx at port %u\n", port_name.c_str(), (long long unsigned)dpid, *of_port_num);
		throw ePmInvalidPort();
	}

	//Check DP existance
	if(!switch_manager::exists(dpid)){
		pthread_mutex_unlock(&port_manager::mutex);
		ROFL_ERR("[xdpd][port_manager] ERROR: Attempting to attach port %s to a non-existent switch with dpid 0x%llx at port %u\n", port_name.c_str(), (long long unsigned)dpid, *of_port_num);
		throw eOfSmDoesNotExist();	
	}

	if(hal_driver_attach_port_to_switch(dpid, port_name.c_str(), of_port_num) != HAL_SUCCESS){
		pthread_mutex_unlock(&port_manager::mutex);
		assert(0);
		ROFL_ERR("[xdpd][port_manager] ERROR: Driver was unable to attach port %s to switch with dpid 0x%llx at port %u\n", port_name.c_str(), (long long unsigned)dpid, *of_port_num);
		throw ePmUnknownError(); 
	}
	
	ROFL_INFO("[xdpd][port_manager] Port %s attached to switch with dpid 0x%llx at port %u\n", port_name.c_str(), (long long unsigned)dpid, *of_port_num);

	//Recover current snapshot
	switch_port_snapshot_t* port_snapshot = hal_driver_get_port_snapshot_by_name(port_name.c_str());

	//Notify switch
	switch_manager::__notify_port_attached(port_snapshot);
		
	//Notify plugins
	plugin_manager::__notify_port_attached(port_snapshot);
	
	//Release mutex	
	pthread_mutex_unlock(&port_manager::mutex);
	
	//Destroy snapshot
	switch_port_destroy_snapshot(port_snapshot);	
}

void port_manager::connect_switches(uint64_t dpid_lsi1, std::string& port_name1, uint64_t dpid_lsi2, std::string& port_name2){

	switch_port_t *port1 = NULL, *port2 = NULL;

	//Serialize
	pthread_mutex_lock(&port_manager::mutex);

	//Check lsi existance 
	if(!switch_manager::exists(dpid_lsi1) || !switch_manager::exists(dpid_lsi2) ){
		pthread_mutex_unlock(&port_manager::mutex);
		ROFL_ERR("[xdpd][port_manager] ERROR: switch with dpid 0x%llx or 0x%llx (or both) do not exist.\n", (long long unsigned)dpid_lsi1, (long long unsigned)dpid_lsi2);
		throw eOfSmDoesNotExist();
	}

	if(hal_driver_connect_switches(dpid_lsi1, &port1, dpid_lsi2, &port2) != HAL_SUCCESS){
		pthread_mutex_unlock(&port_manager::mutex);
		ROFL_ERR("[xdpd][port_manager] Unknown ERROR: driver was unable to create a link between switch with dpid 0x%llx and 0x%llx\n", (long long unsigned)dpid_lsi1, (long long unsigned)dpid_lsi2);
		assert(0);
		throw ePmUnknownError(); 
	}

	
	//Copy port names
	port_name1 = std::string(port1->name);
	port_name2 = std::string(port2->name);

		//Add them to the cache
	try{	
		add_vlink(port_name1, port_name2);
	}catch(...){
		pthread_mutex_unlock(&port_manager::mutex);
		throw;
	}

	ROFL_INFO("[xdpd][port_manager] Link created between switch with dpid 0x%llx and 0x%llx, with virtual interface names %s and %s respectively \n", (long long unsigned)dpid_lsi1, (long long unsigned)dpid_lsi2, port1->name, port2->name);

	//Release mutex	
	pthread_mutex_unlock(&port_manager::mutex);

	/*
	* Note that there is no need to notify switch_manager or plugin_manager
	* the driver must notify CMM with a port_add message with the appropriate
	* attached dpid after the connection has been done.
	*/
	
	//Destroy copies
	switch_port_destroy_snapshot(port1);	
	switch_port_destroy_snapshot(port2);	
}

void port_manager::detach_port_from_switch(uint64_t dpid, std::string& port_name){

	//Serialize
	pthread_mutex_lock(&port_manager::mutex);

	//Recover current snapshot
	switch_port_snapshot_t* port_snapshot = hal_driver_get_port_snapshot_by_name(port_name.c_str());

	if(!port_snapshot){
		pthread_mutex_unlock(&port_manager::mutex);
		ROFL_ERR("[xdpd][port_manager] ERROR: Attempting to detach non-existent port %s from switch with dpid 0x%llx\n", port_name.c_str(), (long long unsigned)dpid);
		assert(0);
		throw ePmInvalidPort();
	}

	if(port_snapshot->attached_sw_dpid != dpid){
		pthread_mutex_unlock(&port_manager::mutex);
		switch_port_destroy_snapshot(port_snapshot);	
		ROFL_ERR("[xdpd][port_manager] ERROR: Attempting to detach port %s, which is not attached to switch with dpid 0x%llx\n", port_name.c_str(), (long long unsigned)dpid);
		throw ePmPortNotAttachedError();	
	}

	if(hal_driver_detach_port_from_switch(dpid,port_name.c_str()) != HAL_SUCCESS){
		pthread_mutex_unlock(&port_manager::mutex);
		switch_port_destroy_snapshot(port_snapshot);	
		ROFL_ERR("[xdpd][port_manager] Unknown ERROR: Driver was unable to detach non-existent port %s from switch with dpid 0x%llx\n", port_name.c_str(), (long long unsigned)dpid);
		assert(0);	
		throw ePmUnknownError();
	}

	ROFL_INFO("[xdpd][port_manager] Port %s detached from switch with dpid 0x%llx\n", port_name.c_str(), (long long unsigned)dpid);

	/*
	* If the port has been deleted due to the detachment, there is no
	* need to notify switch_manager and plugin_manager; the driver must 
	* notify it via port_delete message
	*/
	if(!port_exists(port_name))
		goto DETACH_RETURN;

	//Notify switch
	switch_manager::__notify_port_detached(port_snapshot);
		
	//Notify plugins
	plugin_manager::__notify_port_detached(port_snapshot);

DETACH_RETURN:
	
	//Release mutex	
	pthread_mutex_unlock(&port_manager::mutex);

	//Destroy snapshot
	switch_port_destroy_snapshot(port_snapshot);	
}

void port_manager::detach_port_from_switch_by_num(uint64_t dpid, unsigned int port_num){

	//Serialize
	pthread_mutex_lock(&port_manager::mutex);

	//Recover current snapshot
	switch_port_snapshot_t* port_snapshot = hal_driver_get_port_snapshot_by_num(dpid, port_num);

	if(!port_snapshot){
		pthread_mutex_unlock(&port_manager::mutex);
		ROFL_ERR("[xdpd][port_manager] ERROR: Attempting to detach port number %u from switch with dpid 0x%llx which is invalid\n", (long long unsigned)dpid, port_num);
		assert(0);
		throw ePmInvalidPort();
	}

	if(port_snapshot->attached_sw_dpid != dpid){
		pthread_mutex_unlock(&port_manager::mutex);
		switch_port_destroy_snapshot(port_snapshot);	
		ROFL_ERR("[xdpd][port_manager] ERROR: Attempting to detach port number %u from switch with dpid 0x%llx which does not exist\n", (long long unsigned)dpid, port_num);
		throw ePmPortNotAttachedError();	
	}


	if(hal_driver_detach_port_from_switch_at_port_num(dpid,port_num) != HAL_SUCCESS){
		pthread_mutex_unlock(&port_manager::mutex);
		ROFL_ERR("[xdpd][port_manager] Unknown ERROR: Driver was unabel to detach port number %u from switch with dpid 0x%llx\n", port_num, (long long unsigned)dpid);
		assert(0);
		switch_port_destroy_snapshot(port_snapshot);	
		throw eOfSmGeneralError();
	}

	ROFL_INFO("[xdpd][port_manager] Port %d detached from switch with dpid 0x%llx\n", port_num, (long long unsigned)dpid);

	/*
	* If the port has been deleted due to the detachment, there is no
	* need to notify switch_manager and plugin_manager; the driver must 
	* notify it via port_delete message
	*/
	std::string port_name(port_snapshot->name);
	if(!port_exists(port_name))
		goto DETACH_BY_NUM_RETURN;
	
	//Notify switch
	switch_manager::__notify_port_detached(port_snapshot);
		
	//Notify plugins
	plugin_manager::__notify_port_detached(port_snapshot);
	
DETACH_BY_NUM_RETURN:
		
	//Release mutex	
	pthread_mutex_unlock(&port_manager::mutex);

	//Destroy snapshot
	switch_port_destroy_snapshot(port_snapshot);	
}
