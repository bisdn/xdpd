/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CTL_PACKETS_H
#define CTL_PACKETS_H 

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <rofl/datapath/pipeline/common/datapacket.h>

/**
* @file pktin_dispatcher.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Functions to dispatch pkt_ins from the processing subsystem
* to another thread (bg currently). 
*
*/

#define PKT_IN_PIPE_READ 0
#define PKT_IN_PIPE_WRITE 1

//Notification pipe
extern int pktin_not_pipe[];

//C++ extern C
ROFL_BEGIN_DECLS

/**
* Get notification fd (write edge of the pipe)
*/
inline int get_packet_in_notify_fd(void){ return pktin_not_pipe[PKT_IN_PIPE_WRITE]; }

/**
* Get read fd (read edge of the pipe)
*/
inline int get_packet_in_read_fd(void){ return pktin_not_pipe[PKT_IN_PIPE_READ]; }

/**
* Send a single byte to wake thread attending pkt_ins 
*/
inline void notify_packet_in(void){ 
	int ret;
	char c='a';
	ret = write(pktin_not_pipe[PKT_IN_PIPE_WRITE], &c, 1);
	(void)ret;
}

/**
* Process packet_ins for all LSI 
*/
void process_packet_ins(void);

/**
* Initialize notification pipe
*/
int init_packetin_pipe(void);

/**
* Destroy or release notification pipe resources 
*/
void destroy_packetin_pipe(void);

//C++ extern C
ROFL_END_DECLS

#endif //CTL_PACKETS
