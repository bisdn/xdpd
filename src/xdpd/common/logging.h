/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDPD_COMMON_LOGGING_H_
#define XDPD_COMMON_LOGGING_H_ 1

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

#include <utils/c_logger.h>

namespace xdpd
{

class logging
{
public:

	static std::filebuf devnull;
	static std::filebuf logfile;
	static std::ostream emerg;
	static std::ostream alert;
	static std::ostream crit;
	static std::ostream error;
	static std::ostream warn;
	static std::ostream notice;
	static std::ostream info;
	static std::ostream debug;
	static std::ostream debug2;
	static std::ostream debug3;
	static std::ostream trace;
	static std::streamsize width;

public:


	/*
	* Logging levels. It is recommended to use these identifiers
	* for code portability while setting debug level
	*/
	static const unsigned int EMERG 	= 0;
	static const unsigned int ALERT 	= 1;
	static const unsigned int CRIT 		= 2;
	static const unsigned int ERROR 	= 3;
	static const unsigned int WARN		= 4;
	static const unsigned int NOTICE	= 5;
	static const unsigned int INFO		= 6;
	static const unsigned int DBG		= 7; //Prevent DEBUG macro expansion
	static const unsigned int DBG2		= 8; //Prevent DEBUG macro expansion
	static const unsigned int DBG3		= 9; //Prevent DEBUG macro expansion
	static const unsigned int TRACE		= 10;

	/**
	 * Initialize C++ logging facility
	 */
	static void
	init();

	/**
	 * Destroy C++ logging facility resources
	 */
	static void
	close();

	/**
	 * Set the debug level. It is recommended, for portability reasons to use
	 * logging::EMERG, logging::ALERT ... constants
	 */
	static void
	set_debug_level(
			unsigned int debug_level);
};


class indent
{
	static unsigned int width;
	unsigned int my_width;
public:
	indent(unsigned int my_width = 0) :
		my_width(my_width) {
		indent::width += my_width;
	};
	virtual ~indent() {
		indent::width = (indent::width >= my_width) ? (indent::width - my_width) : 0;
	};
	static void inc(unsigned int width) {
		indent::width += width;
	};
	static void dec(unsigned int width) {
		indent::width = (indent::width >= width) ? (indent::width - width) : 0;
	};
	static void null() {
		indent::width = 0;
	};
	friend std::ostream&
	operator<< (std::ostream& os, indent const& i) {
		if (indent::width) {
			os << std::setw(indent::width) << " " << std::setw(0);
		}
		return os;
	};
};


}; // end of namespace

#endif /* XDPD_COMMON_LOGGING_H_ */
