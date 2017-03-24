/*
 * cparam.cc
 *
 *  Created on: 01.04.2014
 *      Author: andreas
 */

#include "xdpd/common/cparam.h"

using namespace xdpd;


cparam::cparam()
{}



cparam::~cparam()
{}



cparam::cparam(
		std::string const& param) :
				param(param)
{}



cparam::cparam(
		cparam const& param)
{
	*this = param;
}



cparam&
cparam::operator= (
		cparam const& p)
{
	if (this == &p)
		return *this;

	param = p.param;

	return *this;
}



bool
cparam::operator== (
		cparam const& p) const
{
	return (param == p.param);
}



void
cparam::set_int(
		int value)
{
	std::stringstream sstr;
	sstr << value;
	param = sstr.str();
}



int
cparam::get_int() const
{
	return strtol(param.c_str(), NULL, 0);
}



void
cparam::set_uint(
		unsigned int value)
{
	std::stringstream sstr;
	sstr << value;
	param = sstr.str();
}



unsigned int
cparam::get_uint() const
{
	int value;
	std::stringstream(param) >> value;
	return value;
}



void
cparam::set_bool(bool value)
{
	param = (value) ? "true" : "false";
}



bool
cparam::get_bool() const
{
	std::string s_param(param);
	for (unsigned int i = 0; i < param.length(); i++) {
		s_param[i] = tolower(s_param[i]);
	}
	if ((s_param == "true") || (s_param == "yes") || (s_param == "1") || (s_param == "on")) {
		return true;
	}
	return false;
}



