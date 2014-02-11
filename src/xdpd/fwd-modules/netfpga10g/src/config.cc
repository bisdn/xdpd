#include "config.h"
#include "util/compiler_assert.h"


/*
* Validation of config values (at compile time)
*/


//I/O subsystem
COMPILER_ASSERT(INVALID_io_bufferpool_reservoir, (IO_BUFFERPOOL_RESERVOIR >= 64) );
COMPILER_ASSERT(INVALID_io_bufferpool_capacity, (IO_BUFFERPOOL_CAPACITY >= 1024) );
