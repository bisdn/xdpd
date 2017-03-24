/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CPIPE_H
#define CPIPE_H 1

#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

#include "xdpd/common/exception.h"

namespace xdpd
{

class ePipeBase : public exception {
public:
	ePipeBase() : exception("ePipeBase") {};
};
class ePipeInit : public ePipeBase {
public:
	ePipeInit() {};
}; // initialization of pipe failed
class ePipeInval : public ePipeBase {
public:
	ePipeInval() {};
}; // invalid parameter

class cpipe
{
	enum cpipe_cmd_t {
		CPIPE_WAKEUP = 1,
	};

public:

	/**
	 *
	 */
	cpipe();

	/**
	 *
	 */
	~cpipe();

	/**
	 *
	 */
	void
	writemsg(unsigned char msg = 0);

	/**
	 *
	 */
	unsigned char
	recvmsg();

	/**
	 *
	 */
	int
	get_readfd() const
	{ return pipefd[0]; };

	/**
	 *
	 */
	int
	get_writefd() const
	{ return pipefd[1]; };

public:

	int 			pipefd[2]; 	// pipefd[0]: read, pipefd[1]: write
	pthread_mutex_t pipelock; 	// mutex for this pipe
	bool 			signal_sent;

};

}; // end of namespace

#endif
