#include "config.h"
#include "util/compiler_assert.h"


/*
* Validation of config values (at compile time)
*/


//I/O subsystem
COMPILER_ASSERT( INVALID_io_bufferpool_size , (IO_BUFFERPOOL_SIZE > 128) );
