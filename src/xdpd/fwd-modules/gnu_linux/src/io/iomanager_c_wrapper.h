#ifndef IOMANAGER_C_WRAPPER_H_
#define IOMANAGER_C_WRAPPER_H_

#include <rofl.h>
#include <rofl/datapath/pipeline/switch_port.h>

//C++ extern C
ROFL_BEGIN_DECLS

unsigned int iomanager_create_group_wrapper(unsigned int num_of_threads);
rofl_result_t iomanager_delete_group_wrapper(unsigned int grp_id);
rofl_result_t iomanager_delete_all_groups_wrapper(void);
rofl_result_t iomanager_bring_port_up_wrapper(platform_port_state_t* port);
rofl_result_t iomanager_bring_port_down_wrapper(platform_port_state_t* port);//original definition has a bool mutex_locked= false variable
rofl_result_t iomanager_add_port_to_group_wrapper(unsigned int grp_id, platform_port_state_t* port);
rofl_result_t iomanager_remove_port_from_group_wrapper(unsigned int grp_id, platform_port_state_t* port); //original definition has a bool mutex_locked= false variable

//C++ extern C
ROFL_END_DECLS

#endif //IOMANAGER_C_WRAPPER_H_
