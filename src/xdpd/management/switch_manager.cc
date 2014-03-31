#include "switch_manager.h"

#include <rofl/datapath/hal/hal.h>
#include <rofl/datapath/hal/cmm.h>
#include <rofl/common/utils/c_logger.h>
#ifdef HAVE_OPENSSL
#include <rofl/common/ssl_lib.h>
#endif

#include "port_manager.h"

//Add here the headers of the version-dependant Openflow switchs 
#include "../openflow/openflow_switch.h"
#include "../openflow/openflow10/openflow10_switch.h"
#include "../openflow/openflow12/openflow12_switch.h"
#include "../openflow/openflow13/openflow13_switch.h"

using namespace rofl;
using namespace xdpd;

const caddress switch_manager::controller_addr = caddress(AF_INET, "127.0.0.1", 6633);
const caddress switch_manager::binding_addr = caddress(AF_INET, "0.0.0.0", 6632);

//Static initialization
std::map<uint64_t, openflow_switch*> switch_manager::switchs;
uint64_t switch_manager::dpid_under_destruction = 0x0;
pthread_rwlock_t switch_manager::rwlock = PTHREAD_RWLOCK_INITIALIZER; 

/**
* Static methods of the manager
*/
openflow_switch* switch_manager::create_switch(
		of_version_t version,
		uint64_t dpid,
		std::string const& dpname,
		unsigned int num_of_tables,
		int* ma_list,
		int reconnect_start_timeout,
		enum rofl::csocket::socket_type_t socket_type,
		caddress const& controller_addr,
		caddress const& binding_addr,
		bool enable_ssl,
		const std::string &cert_and_key_file) throw (eOfSmExists, eOfSmErrorOnCreation, eOfSmVersionNotSupported){

	openflow_switch* dp;
	
	pthread_rwlock_wrlock(&switch_manager::rwlock);

	if(switch_manager::switchs.find(dpid) != switch_manager::switchs.end()){
		pthread_rwlock_unlock(&switch_manager::rwlock);
		throw eOfSmExists();
	}

#ifdef HAVE_OPENSSL
	// setup ssl context
	ssl_context *ctx = NULL;
	if (enable_ssl)	{
		ctx = ssl_lib::get_instance().create_ssl_context(ssl_context::SSL_client, cert_and_key_file);
	}
#else
	assert(false == enable_ssl);
#endif

	switch(version){

		case OF_VERSION_10:
			dp = new openflow10_switch(dpid, dpname, num_of_tables, ma_list, reconnect_start_timeout, socket_type, controller_addr, binding_addr);
			// TODO: hand over SSL cert and key files

			break;

		case OF_VERSION_12:
			dp = new openflow12_switch(dpid, dpname, num_of_tables, ma_list, reconnect_start_timeout, socket_type, controller_addr, binding_addr);
			// TODO: hand over SSL cert and key files

			break;
	
		case OF_VERSION_13:
			dp = new openflow13_switch(dpid, dpname, num_of_tables, ma_list, reconnect_start_timeout, socket_type, controller_addr, binding_addr);
			// TODO: hand over SSL cert and key files

			break;

		//Add more here...
		
		default:
			pthread_rwlock_unlock(&switch_manager::rwlock);
			throw eOfSmVersionNotSupported();

	}	
	
	//Store in the switch list
	switch_manager::switchs[dpid] = dp;
	
	pthread_rwlock_unlock(&switch_manager::rwlock);
	
	ROFL_INFO("[switch_manager] Created switch %s with dpid 0x%llx\n", dpname.c_str(), (long long unsigned)dpid);

	return dp; 
}

//static
void switch_manager::destroy_switch(uint64_t dpid) throw (eOfSmDoesNotExist){

	unsigned int i;
	of_switch_snapshot_t* sw_snapshot;
	switch_port_snapshot_t* port;
	std::string port_name;

	pthread_rwlock_wrlock(&switch_manager::rwlock);
	
	if (switch_manager::switchs.find(dpid) == switch_manager::switchs.end()){
		pthread_rwlock_unlock(&switch_manager::rwlock);
		throw eOfSmDoesNotExist();
	}

	//First detach all ports, so that port_detach messages are properly sent
	sw_snapshot = hal_driver_get_switch_snapshot_by_dpid(dpid);
	
	if(!sw_snapshot){
		pthread_rwlock_unlock(&switch_manager::rwlock);
		assert(0);
		ROFL_ERR("[switch_manager] Unknown ERROR: unable to create snapshot for dpid 0x%llx. Switch deletion aborted...\n", (long long unsigned)dpid);
		throw eOfSmGeneralError(); 
	}

	for(i=0;i<sw_snapshot->max_ports;++i){
		port = sw_snapshot->logical_ports[i].port;
		if(!port || sw_snapshot->logical_ports[i].attachment_state != LOGICAL_PORT_STATE_ATTACHED)
			continue;

		try{
			port_name = std::string(port->name);
			//Detach port
			port_manager::detach_port_from_switch(dpid, port_name);
		}catch(...){
			pthread_rwlock_unlock(&switch_manager::rwlock);
			ROFL_ERR("[switch_manager] ERROR: unable to detach port %s from dpid 0x%llx. Switch deletion aborted...\n", port->name, (long long unsigned)dpid);
			assert(0);

			of_switch_destroy_snapshot(sw_snapshot);		
			throw;
		}
	}
	
	//Get switch instance 
	openflow_switch* dp = switch_manager::switchs[dpid];
	switch_manager::switchs.erase(dpid);

	ROFL_INFO("[switch_manager] Destroyed switch with dpid 0x%llx\n", (long long unsigned)dpid);

	//Set the dpid under destruction
	dpid_under_destruction = dp->dpid;

	//Destroy element
	delete dp;	
	
	dpid_under_destruction = 0x0;
	
	pthread_rwlock_unlock(&switch_manager::rwlock);

	//Destroy snapshot
	of_switch_destroy_snapshot(sw_snapshot);		
}

//static
void switch_manager::destroy_all_switches(){

	pthread_rwlock_wrlock(&switch_manager::rwlock);
	
	for(std::map<uint64_t, openflow_switch*>::iterator it = switchs.begin(); it != switchs.end(); ++it) {
		//Set the dpid under destruction
		dpid_under_destruction = it->second->dpid;

		//Delete	
		delete it->second; 
		
		dpid_under_destruction = 0x0;
	}
	
	switchs.clear();
	
	pthread_rwlock_unlock(&switch_manager::rwlock);
	
}

/**
 * List datapath names
 */
//static std::vector<std::string> path_split(std::string const& words);
std::list<std::string> switch_manager::list_sw_names(void){
	
	std::list<std::string> name_list;

	for(std::map<uint64_t, openflow_switch*>::iterator it = switchs.begin(); it != switchs.end(); ++it) {
		name_list.push_back(it->second->dpname);
	}

	return name_list;
}

/* static */std::list<std::string>
switch_manager::list_matching_algorithms(of_version_t of_version)
{
	std::list<std::string> matching_algorithms;
	int i, count;

	const char * const * names;
	if(hal_driver_list_matching_algorithms(of_version, &names, &count) != HAL_SUCCESS){
		return matching_algorithms;
	}

	for (i = 0; i < count; i++) {
		matching_algorithms.push_back(std::string(names[i]));
	}

	return matching_algorithms;
}


bool switch_manager::exists(uint64_t dpid){
	
	bool found=true;

	pthread_rwlock_rdlock(&switch_manager::rwlock);
	
	if (switch_manager::switchs.find(dpid) == switch_manager::switchs.end())
		found=false;
	
	pthread_rwlock_unlock(&switch_manager::rwlock);
	
	return found;
}

bool switch_manager::exists_by_name(std::string& name){
	
	bool found=false;

	pthread_rwlock_rdlock(&switch_manager::rwlock);

	for(std::map<uint64_t, openflow_switch*>::iterator it = switchs.begin(); it != switchs.end(); ++it){
		if( it->second->dpname == name)
			found=true;	

	}
	
	pthread_rwlock_unlock(&switch_manager::rwlock);

	return found; 
}

uint64_t switch_manager::get_switch_dpid(std::string& name){

	uint64_t dpid;
	bool found=false;

	pthread_rwlock_rdlock(&switch_manager::rwlock);

	for(std::map<uint64_t, openflow_switch*>::iterator it = switchs.begin(); it != switchs.end(); ++it){
		if( it->second->dpname == name){
			found=true;
			dpid = it->second->dpid;
		}

	}
	
	pthread_rwlock_unlock(&switch_manager::rwlock);

	if(found)
		return dpid;
	else
		throw eOfSmDoesNotExist(); 
}




void
switch_manager::rpc_connect_to_ctl(uint64_t dpid, enum rofl::csocket::socket_type_t socket_type, caddress const& ra){

	pthread_rwlock_wrlock(&switch_manager::rwlock);
	
	if (switch_manager::switchs.find(dpid) == switch_manager::switchs.end()){
		pthread_rwlock_unlock(&switch_manager::rwlock);
		throw eOfSmDoesNotExist();
	}

	//Get switch instance
	openflow_switch* dp = switch_manager::switchs[dpid];
	dp->rpc_connect_to_ctl(socket_type, ra);
	pthread_rwlock_unlock(&switch_manager::rwlock);
}



void
switch_manager::rpc_disconnect_from_ctl(uint64_t dpid, caddress const& ra){

	pthread_rwlock_wrlock(&switch_manager::rwlock);
	
	if (switch_manager::switchs.find(dpid) == switch_manager::switchs.end()){
		pthread_rwlock_unlock(&switch_manager::rwlock);
		throw eOfSmDoesNotExist();
	}

	//Get switch instance
	openflow_switch* dp = switch_manager::switchs[dpid];
	dp->rpc_disconnect_from_ctl(ra);
	pthread_rwlock_unlock(&switch_manager::rwlock);
}


openflow_switch* switch_manager::__get_switch_by_dpid(uint64_t dpid){

	if (switch_manager::switchs.find(dpid) == switch_manager::switchs.end()){
		return NULL;
	}

	return switch_manager::switchs[dpid];
}


//
//CMM demux
//

rofl_result_t switch_manager::__notify_port_attached(const switch_port_snapshot_t* port_snapshot){
	
	rofl_result_t result;
	openflow_switch* sw;	

	if(!port_snapshot || port_snapshot->attached_sw_dpid == dpid_under_destruction)
		return ROFL_FAILURE;

	while( pthread_rwlock_tryrdlock(&switch_manager::rwlock) != 0 ){
		//This can happen while destroying the LSI
		if(port_snapshot->attached_sw_dpid == dpid_under_destruction)
			return ROFL_FAILURE;
		usleep(50); //Calm down
	}

	sw = switch_manager::__get_switch_by_dpid(port_snapshot->attached_sw_dpid); 

	if(sw)
		result = sw->notify_port_attached(port_snapshot);
	else	
		result = ROFL_FAILURE;
	
	pthread_rwlock_unlock(&switch_manager::rwlock);

	return result;	
}	

rofl_result_t switch_manager::__notify_port_status_changed(const switch_port_snapshot_t* port_snapshot){
	
	rofl_result_t result;
	openflow_switch* sw;	

	if(!port_snapshot || port_snapshot->attached_sw_dpid == dpid_under_destruction)
		return ROFL_FAILURE;

	while( pthread_rwlock_tryrdlock(&switch_manager::rwlock) != 0 ){
		//This can happen while destroying the LSI
		if(port_snapshot->attached_sw_dpid == dpid_under_destruction)
			return ROFL_FAILURE;
		usleep(50); //Calm down
	}

	sw = switch_manager::__get_switch_by_dpid(port_snapshot->attached_sw_dpid); 

	if(sw)
		result = sw->notify_port_status_changed(port_snapshot);
	else	
		result = ROFL_FAILURE;
	
	pthread_rwlock_unlock(&switch_manager::rwlock);

	return result;	

}

rofl_result_t switch_manager::__notify_port_detached(const switch_port_snapshot_t* port_snapshot){
	
	rofl_result_t result;
	openflow_switch* sw;	

	if(!port_snapshot || port_snapshot->attached_sw_dpid == dpid_under_destruction)
		return ROFL_FAILURE;

	while( pthread_rwlock_tryrdlock(&switch_manager::rwlock) != 0 ){
		//This can happen while destroying the LSI
		if(port_snapshot->attached_sw_dpid == dpid_under_destruction)
			return ROFL_FAILURE;
		usleep(50); //Calm down
	}

	sw = switch_manager::__get_switch_by_dpid(port_snapshot->attached_sw_dpid); 

	if(sw)
		result = sw->notify_port_detached(port_snapshot);
	else	
		result = ROFL_FAILURE;
	
	pthread_rwlock_unlock(&switch_manager::rwlock);

	return result;	
}

rofl_result_t switch_manager::__process_of1x_packet_in(uint64_t dpid,
				uint8_t table_id,
				uint8_t reason,
				uint32_t in_port,
				uint32_t buffer_id,
				uint8_t* pkt_buffer,
				uint32_t buf_len,
				uint16_t total_len,
				packet_matches_t* matches){
	rofl_result_t result;
	openflow_switch* sw;	

	if(dpid == dpid_under_destruction)
		return ROFL_SUCCESS;

	while( pthread_rwlock_tryrdlock(&switch_manager::rwlock) != 0 ){
		//This can happen while destroying the LSI
		if(dpid == dpid_under_destruction)
			return ROFL_FAILURE;
		usleep(50); //Calm down
	}

	sw = switch_manager::__get_switch_by_dpid(dpid); 

	if(sw)
		result = sw->process_packet_in(table_id, reason, in_port, buffer_id, pkt_buffer, buf_len, total_len, matches);
	else	
		result = ROFL_FAILURE;
	
	pthread_rwlock_unlock(&switch_manager::rwlock);

	return result;	

}

rofl_result_t switch_manager::__process_of1x_flow_removed(uint64_t dpid, 
				uint8_t reason, 	
				of1x_flow_entry_t* removed_flow_entry){

	rofl_result_t result;
	openflow_switch* sw;	

	if(dpid == dpid_under_destruction)
		return ROFL_SUCCESS;

	while( pthread_rwlock_tryrdlock(&switch_manager::rwlock) != 0 ){
		//This can happen while destroying the LSI
		if(dpid == dpid_under_destruction)
			return ROFL_FAILURE;
		usleep(50); //Calm down
	}

	sw = switch_manager::__get_switch_by_dpid(dpid); 

	if(sw)
		result = sw->process_flow_removed(reason, removed_flow_entry);
	else	
		result = ROFL_FAILURE;
	
	pthread_rwlock_unlock(&switch_manager::rwlock);

	return result;	

}
