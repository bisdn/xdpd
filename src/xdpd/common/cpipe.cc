/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cpipe.h"

using namespace xdpd;

cpipe::cpipe() :
		signal_sent(false)
{
	// create two pipes for connecting stdin and stdout
	if (pipe(pipefd) < 0) {
		throw eSysCall("pipe()");
	}

	// make pipe non-blocking
	int flags = 0;


	if ((flags = fcntl(pipefd[0], F_GETFL)) < 0) {
		throw eSysCall("fcntl(): F_GETFL");
	}

	flags |= O_NONBLOCK;

	if (fcntl(pipefd[0], F_SETFL, flags) < 0) {
		throw eSysCall("fcntl(): F_SETFL");
	}

	pthread_mutex_init(&pipelock, NULL);
}


cpipe::~cpipe()
{
	pthread_mutex_destroy(&pipelock);

	close(pipefd[0]);
	close(pipefd[1]);
}


void
cpipe::writemsg(unsigned char msg)
{
	//if (signal_sent)
	//	return;
	int rc = write(pipefd[1], &msg, sizeof(msg));
	if (rc == 0) {

	} else if (rc < 0) {
		throw eSysCall("writemsg()");
	}
	signal_sent = true;
}


unsigned char
cpipe::recvmsg()
{
	signal_sent = false;
	unsigned char msg = 0;
	int rc;
	if ((rc = read(pipefd[0], &msg, sizeof(msg))) < 0) {

		switch (errno) {
		case EAGAIN:
			return 0;
		default:
			break;
		}
	}
	return msg;
}


