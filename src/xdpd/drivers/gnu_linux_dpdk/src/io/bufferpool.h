/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H 

#include "../util/likely.h"
#include "../config.h"

//Clear flag
#define BUFFERPOOL_CLEAR_IS_REPLICA

//We don't need more capacity in the bufferpool
#define IO_BUFFERPOOL_CAPACITY 0

//Include the meta bufferpool
#include "bufferpool_meta.h"

#endif /* BUFFERPOOL_H_ */
