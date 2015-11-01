/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CUNIXENV_H
#define CUNIXENV_H 1

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <execinfo.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/stat.h>

#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>

#include "xdpd/common/exception.h"

namespace xdpd
{

/*
* Helper class to manipulate easily arguments in C++ programs. Argmuents are ALWAYS strings
*/

#define NO_ARGUMENT no_argument
#define REQUIRED_ARGUMENT required_argument
#define OPTIONAL_ARGUMENT optional_argument

class coption
{

public:
	bool optional;
	int value_type;
	char shortcut;
	std::string full_name;
	std::string description;
	std::string default_value;
	
	/* Current state */
	bool present;
	std::string current_value;


	/* Constructor */
	//coption(void){};

	coption(bool optional, 
		int value_type, 
		char shortcut, 
		std::string full_name, 
		std::string description, 
		std::string default_value);

	/* Atempt to parse argument from optarg provided by getopt*/
	std::string parse_argument(char* optarg);
	bool is_present(void){return present;}
	std::string value(void){return current_value;}
};

/* Forward declaration */

/* 
* Unix parser class 
*/
class cunixenv
{
public:

	/**
	 * Constructor
	 */
	cunixenv(int argc = 0, char** argv = NULL);
	
	/**
	 * Destructor
	 */
	~cunixenv();

	/*
	* Add argument to current list of arguments to parse 
	*/
	void add_option(const coption &arg);
	
	/**
	 * Parse arguments using getopt
	 */
	void parse_args();

	/*
	 * Get value methods
	 */
	std::string get_arg(const std::string &name);

	std::string get_arg(const char shortcut);

	bool
	is_arg_set(const std::string &name);

	void
	update_default_option(const std::string &option_name, const std::string &value);

	/**
	 * Get usage pre-formatted message
	 */
	std::string get_usage(char *argv0);

private:

	std::vector<coption> arguments;
	bool parsed;

	// copy of values obtained from (int argc, char** argv)
	std::vector<std::string> cargs;
};

}; // end of namespace

#endif
