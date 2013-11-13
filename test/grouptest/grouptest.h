#ifndef GROUPTEST_H
#define GROUPTEST_H

#include <rofl/common/crofbase.h>

class cgrouptest : public rofl::crofbase {
public:
	/**
	 *
	 */
	cgrouptest();

	/**
	 *
	 */
	virtual
	~cgrouptest();


	virtual void
	handle_dpath_open(rofl::cofdpt *dpt) {};


	virtual void
	handle_dpath_close(rofl::cofdpt *dpt) {};
};

#endif
