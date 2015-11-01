/*
 * cdaemon.cc
 *
 *  Created on: 28.01.2014
 *      Author: andreas
 */

#include "xdpd/common/cdaemon.h"
#include <stdexcept> 

using namespace xdpd;


/*static*/
void
cdaemon::daemonize(
		std::string const& pidfile, std::string const& logfile)
{
	int pipefd[2];	// pipe for daemonizing

	try {
		/*
		 * close all file descriptors excluding stdin, stdout, stderr
		 */
		struct rlimit rlim;
		if (getrlimit(RLIMIT_NOFILE, &rlim) < 0) {
			throw eSysCall("getrlimit()");
		}
		for (unsigned int i = 3; i < rlim.rlim_cur; i++) {
			close(i);
		}

		/*
		 * create a pipe for signaling successful initialization back from child-2 to parent process
		 */
		if (pipe2(pipefd, 0) < 0) {
			throw eSysCall("pipe2()");
		}

		/*
		 * reset signal handlers to default behaviour
		 */
		for (int i = 1; i < 32; i++) {
			if (i == SIGKILL)
				continue;
			if (i == SIGSTOP)
				continue;
			if (signal(i, SIG_DFL) == SIG_ERR) {
				std::cerr << "i:" << i << std::endl;
				throw eSysCall("signal()");
			}
		}

		/*
		 * reset sigprocmask
		 */
		sigset_t sigset;
		if (sigfillset(&sigset) < 0) {
			throw eSysCall("sigfillset()");
		}
		if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
			throw eSysCall("sigprocmask()");
		}

		/*
		 * environment
		 */
		// we are not setting anything

		/*
		 * fork: create first child
		 */
		pid_t pid1 = fork();
		if (pid1 < 0) {
			throw eSysCall("fork()");
		}
		else if (pid1 > 0) { // parent exit
			uint8_t a;
			ssize_t tmp =  read(pipefd[0], &a, sizeof(a)); // blocks, until child-2 writes a byte to pipefd[1]
			(void) tmp;
			switch (a) {
			case 0: // success
				logging::error << "[rofl][cdaemon] daemonizing successful. PID: "<<pid1<< std::endl;
				exit(EXIT_SUCCESS);
			case 1: // failure
				logging::error << "[rofl][cdaemon] daemonizing failed" << std::endl;
				throw std::runtime_error("Unable to daemonize");
			}
		}

		/*
		 *  call setsid() in the first child
		 */
		pid_t sid;
		if ((sid = setsid()) < 0) { // detach from controlling terminal
			throw eSysCall("setsid()");
		}

		/*
		 * fork: create second child
		 */
		pid_t pid2 = fork();
		if (pid2 < 0) {
			throw eSysCall("fork()");
		}
		else if (pid2 > 0) { // first child exit
			exit(0);
		}

		/*
		 * reset umask
		 */
		umask(0022);

		/*
		 * redirect STDIN, STDOUT, STDERR to logfile
		 */
		int fd = open(logfile.c_str(), O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
		if (fd < 0) {
			throw eSysCall("open()");
		}
		if (dup2(fd, STDIN_FILENO) < 0) {
			throw eSysCall("dup2(): STDIN");
		}
		if (dup2(fd, STDOUT_FILENO) < 0) {
			throw eSysCall("dup2(): STDOUT");
		}
		if (dup2(fd, STDERR_FILENO) < 0) {
			throw eSysCall("dup2(): STDERR");
		}

		/*
		 * change current working directory
		 */
		if (chdir("/") < 0) {
			throw eSysCall("chdir()");
		}

		/*
		 * write pidfile
		 */
		int pidfd = open(pidfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if (pidfd < 0) {
			throw eSysCall("open(): pidfile");
		}
		char s_pidno[32];
		memset(s_pidno, 0, sizeof(s_pidno));
		snprintf(s_pidno, sizeof(s_pidno)-1, "%d", (int)getpid());
		if (write(pidfd, s_pidno, strnlen(s_pidno, sizeof(s_pidno))) < 0) {
			throw eSysCall("write(): pidfile");
		}
		close(pidfd);

		/*
		 * notify parent process about successful initialization
		 */
		uint8_t a = 0;
		if (write(pipefd[1], &a, sizeof(a)) < 0) {
			throw eSysCall("write(): pipe");
		}

	} catch (eSysCall& e) {
		uint8_t a = 1;
		if (write(pipefd[1], &a, sizeof(a)) < 0) {
			std::cerr << e << std::endl;
			throw eSysCall("write(): pipe");
		}
	}
}


