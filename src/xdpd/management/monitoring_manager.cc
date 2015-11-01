#include "monitoring_manager.h"

#include "xdpd/common/utils/c_logger.h"

using namespace xdpd;


monitoring_snapshot_state_t* monitoring_manager::get_monitoring_snapshot(uint64_t last_rev){
	return hal_driver_get_monitoring_snapshot(last_rev);
}
