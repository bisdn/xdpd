/*
 * dbuserror.cc
 *
 *  Created on: 10.01.2014
 *      Author: andreas
 */

#include "dbuserror.h"

using namespace xdpd::dbus;

dbuserror::dbuserror()
{
	dbus_error_init(&err);
}



dbuserror::~dbuserror()
{
	dbus_error_free(&err);
}


