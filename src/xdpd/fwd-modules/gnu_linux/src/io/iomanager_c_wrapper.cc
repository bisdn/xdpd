#include "iomanager_c_wrapper.h"
#include "iomanager.h"

unsigned int iomanager_create_group_wrapper(unsigned int num_of_threads){
	return iomanager::create_group(num_of_threads);
}

rofl_result_t iomanager_delete_group_wrapper(unsigned int grp_id){
	return iomanager::delete_group(grp_id);
}

rofl_result_t iomanager_bring_port_up_wrapper(platform_port_state_t* port)
{
	return iomanager::bring_port_up((ioport*)port);
}

rofl_result_t iomanager_bring_port_down_wrapper(platform_port_state_t* port)
{
	return iomanager::bring_port_down((ioport*)port, false);
}

rofl_result_t iomanager_add_port_to_group_wrapper(unsigned int grp_id, platform_port_state_t* port)
{
	return iomanager::add_port_to_group(grp_id, (ioport*)port);
}

rofl_result_t iomanager_remove_port_from_group_wrapper(unsigned int grp_id, platform_port_state_t* port)
{
	return iomanager::remove_port_from_group(grp_id,(ioport*)port, false);
}
