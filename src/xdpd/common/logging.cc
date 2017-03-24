/*
 * logging.cc
 *
 *  Created on: 23.11.2013
 *      Author: andreas
 *
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logging.h"

using namespace xdpd;

std::filebuf logging::devnull;
std::ostream logging::emerg	 (&logging::devnull);
std::ostream logging::alert  (&logging::devnull);
std::ostream logging::crit   (&logging::devnull);
std::ostream logging::error  (&logging::devnull);
std::ostream logging::warn   (&logging::devnull);
std::ostream logging::notice (&logging::devnull);
std::ostream logging::info   (&logging::devnull);
std::ostream logging::debug  (&logging::devnull);
std::ostream logging::debug2 (&logging::devnull);
std::ostream logging::debug3 (&logging::devnull);
std::ostream logging::trace  (&logging::devnull);

std::streamsize logging::width(70);
unsigned int indent::width(0);


void
logging::init()
{
	if (not logging::devnull.is_open()) {
		logging::devnull.open("/dev/null", std::ios::out);
	}
}


void
logging::close()
{
	if (logging::devnull.is_open()) {
		logging::devnull.close();
	}
}



void
logging::set_debug_level(
			unsigned int debug_level)
{
	logging::init();

	// EMERG
	logging::emerg .rdbuf(std::cerr.rdbuf());

	// ALERT
	if (debug_level >= ALERT) {
		logging::alert .rdbuf(std::cerr.rdbuf());
	} else {
		logging::alert .rdbuf(&logging::devnull);
	}

	// CRIT
	if (debug_level >= CRIT) {
		logging::crit  .rdbuf(std::cerr.rdbuf());
	} else {
		logging::crit  .rdbuf(&logging::devnull);
	}

	// ERROR
	if (debug_level >= ERROR) {
		logging::error .rdbuf(std::cerr.rdbuf());
	} else {
		logging::error .rdbuf(&logging::devnull);
	}

	// WARN
	if (debug_level >= WARN) {
		logging::warn  .rdbuf(std::cerr.rdbuf());
	} else {
		logging::warn  .rdbuf(&logging::devnull);
	}

	// NOTICE
	if (debug_level >= NOTICE) {
		logging::notice.rdbuf(std::cerr.rdbuf());
	} else {
		logging::notice.rdbuf(&logging::devnull);
	}

	// INFO
	if (debug_level >= INFO) {
		logging::info  .rdbuf(std::cerr.rdbuf());
	} else {
		logging::info  .rdbuf(&logging::devnull);
	}

	// DEBUG
	if (debug_level >= DBG) {
		logging::debug .rdbuf(std::cerr.rdbuf());
	} else {
		logging::debug .rdbuf(&logging::devnull);
	}

	// DEBUG2
	if (debug_level >= DBG2) {
		logging::debug2.rdbuf(std::cerr.rdbuf());
	} else {
		logging::debug2.rdbuf(&logging::devnull);
	}

	// DEBUG3
	if (debug_level >= DBG3) {
		logging::debug3.rdbuf(std::cerr.rdbuf());
	} else {
		logging::debug3.rdbuf(&logging::devnull);
	}

	// TRACE 
	if (debug_level >= TRACE) {
		logging::trace .rdbuf(std::cerr.rdbuf());
	} else {
		logging::trace .rdbuf(&logging::devnull);
	}
}

