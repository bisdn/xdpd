/*
 * dbusmod.cc
 *
 *  Created on: 10.01.2014
 *      Author: andreas
 */

#include "dbusmod.h"

using namespace xdpd::dbus;

dbusmod::dbusmod()
{

}



dbusmod::~dbusmod()
{

}



void
dbusmod::init(
		int argc, char** argv)
{
	conn.request_name("xdpd");
}



void
dbusmod::handle_timeout(
		int opaque)
{

}



void
dbusmod::handle_revent(
		int fd)
{

}


