#include "monitoring_manager.h"
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;
using namespace rofl;


monitoring_snapshot_state_t* monitoring_manager::get_monitoring_snapshot(uint64_t last_rev){
	return fwd_module_get_monitoring_snapshot(last_rev);
}
