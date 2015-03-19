#include "config.h"
#include "util/compiler_assert.h"

/*
* Validation of config values (at compile time)
*/


//I/O subsystem
COMPILER_ASSERT(INVALID_io_iface_num_queues , (IO_IFACE_NUM_QUEUES > 0) );
COMPILER_ASSERT(INVALID_io_bufferpool_reservoir, (IO_BUFFERPOOL_RESERVOIR >= 64) );


COMPILER_ASSERT(INVALID_max_cpu_sockets, (MAX_CPU_SOCKETS > 0) );

//#if defined(GNU_LINUX_DPDK_ENABLE_NF) && !defined(DPDK_PATCHED_KNI)
	//#warning DPDK is not patched to support rte_kni_init()
//#endif
