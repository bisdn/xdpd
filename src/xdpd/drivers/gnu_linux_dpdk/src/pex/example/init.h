/**
* @file init.h
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief Initialize the PEX.
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <signal.h>

#include <rte_memcpy.h>
#include <rte_mbuf.h>

#include "main.h"

void pex_init(void);
