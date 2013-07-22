#include "config.h"
#include "util/compiler_assert.h"

/*
* Validation of config values (at compile time)
*/


//I/O subsystem
COMPILER_ASSERT( INVALID_io_total_threads , (IO_TOTAL_THREADS > 0) );
COMPILER_ASSERT(INVALID_io_iface_ring_slots, (IO_IFACE_RING_SLOTS >= 16) );
//COMPILER_ASSERT(INVALID_io_iface_ring_slots_align_power_2, (IO_IFACE_RING_SLOTS % 2 == 0) );
COMPILER_ASSERT(INVALID_io_iface_frame_size, ( (IO_IFACE_FRAME_SIZE >= 2048) && (IO_IFACE_FRAME_SIZE <= 8192) ) );
//COMPILER_ASSERT(INVALID_io_iface_frame_size_align_power_2, (IO_IFACE_RING_SLOTS % 2 == 0) );

//Processing subsystem
COMPILER_ASSERT(INVALID_processing_threads_per_lsi, (PROCESSING_THREADS_PER_LSI > 0) );
COMPILER_ASSERT(INVALID_processing_input_queue_slots, (PROCESSING_INPUT_QUEUE_SLOTS >= 1024) );
//COMPILER_ASSERT(INVALID_processing_input_queue_slots_align_power_2, (PROCESSING_INPUT_QUEUE_SLOTS % 2 == 0) );
COMPILER_ASSERT(INVALID_processing_pkt_in_queue_slots, (PROCESSING_PKT_IN_QUEUE_SLOTS >= 16) );
//COMPILER_ASSERT(INVALID_processing_pkt_in_queue_slots_align_power_2, (PROCESSING_PKT_IN_QUEUE_SLOTS % 2 == 0) );
