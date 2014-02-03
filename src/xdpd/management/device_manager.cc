#include "device_manager.h"
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;
using namespace rofl;


monitoring_snapshot_state_t* device_manager::get_monitoring_snapshot(){
	return fwd_module_get_monitoring_snapshot(0);
}
