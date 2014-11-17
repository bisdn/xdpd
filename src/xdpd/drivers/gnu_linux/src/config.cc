#include "util/compiler_assert.h"
#include "sched.h"
#include "config.h"

//Boundary values retrival
#include "processing/ls_internal_state.h"

/*
* Validation of config values (at compile time)
*/

//I/O subsystem
COMPILER_ASSERT( INVALID_kern_sched_pol , ((IO_KERN_SCHED_POL == SCHED_OTHER) || (IO_KERN_SCHED_POL == SCHED_FIFO ) || (IO_KERN_SCHED_POL == SCHED_RR) ) );
COMPILER_ASSERT(INVALID_io_iface_ring_slots, (IO_IFACE_RING_SLOTS >= 16) );
COMPILER_ASSERT(INVALID_io_bufferpool_reservoir, (IO_BUFFERPOOL_RESERVOIR >= 64) );
COMPILER_ASSERT(INVALID_io_bufferpool_capacity, (IO_BUFFERPOOL_CAPACITY >= 1024) );
//COMPILER_ASSERT(INVALID_io_iface_ring_slots_align_power_2, (IO_IFACE_RING_SLOTS % 2 == 0) );
COMPILER_ASSERT(INVALID_io_iface_frame_size, ( (IO_IFACE_MMAP_FRAME_SIZE >= 2048) && (IO_IFACE_MMAP_FRAME_SIZE <= 8192) ) );
//COMPILER_ASSERT(INVALID_io_iface_frame_size_align_power_2, (IO_IFACE_RING_SLOTS % 2 == 0) );

//Processing subsystem
COMPILER_ASSERT(INVALID_processing_threads_per_lsi, (IO_RX_THREADS > 0) && (IO_TX_THREADS > 0) && ( (IO_RX_THREADS+IO_TX_THREADS) <= IO_MAX_THREADS) );
COMPILER_ASSERT(INVALID_processing_input_queue_slots, (LSI_INPUT_QUEUE_SLOTS >= 1024) );
//COMPILER_ASSERT(INVALID_processing_input_queue_slots_align_power_2, (LSI_INPUT_QUEUE_SLOTS % 2 == 0) );
COMPILER_ASSERT(INVALID_processing_pkt_in_queue_slots, (LSI_PKT_IN_QUEUE_SLOTS >= 4) );
//COMPILER_ASSERT(INVALID_processing_pkt_in_queue_slots_align_power_2, (LSI_PKT_IN_QUEUE_SLOTS % 2 == 0) );
