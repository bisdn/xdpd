/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "config.h"
#include "util/compiler_assert.h"


/*
* Validation of config values (at compile time)
*/


//I/O subsystem
COMPILER_ASSERT(INVALID_io_bufferpool_reservoir, (IO_BUFFERPOOL_RESERVOIR >= 64) );
COMPILER_ASSERT(INVALID_io_bufferpool_capacity, (IO_BUFFERPOOL_CAPACITY >= 1024) );
