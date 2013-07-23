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

#define PKT_IN_PIPE_READ 0
#define PKT_IN_PIPE_WRITE 1

extern int pktin_not_pipe[];

//C++ extern C
ROFL_BEGIN_DECLS

inline int get_packet_in_notify_fd(void){ return pktin_not_pipe[PKT_IN_PIPE_WRITE]; }
inline int get_packet_in_read_fd(void){ return pktin_not_pipe[PKT_IN_PIPE_READ]; }

inline void notify_packet_in(void){ 
	char c='a';
	write(pktin_not_pipe[PKT_IN_PIPE_WRITE], &c, 1);
}

void process_packet_ins(void);
int init_packetin_pipe(void);
void destroy_packetin_pipe(void);

//C++ extern C
ROFL_END_DECLS

#endif //CTL_PACKETS
