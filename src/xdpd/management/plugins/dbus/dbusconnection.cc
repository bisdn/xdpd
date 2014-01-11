/*
 * dbusconnection.cc
 *
 *  Created on: 10.01.2014
 *      Author: andreas
 */

#include "dbusconnection.h"

using namespace xdpd::dbus;

dbusconnection::dbusconnection() :
		conn(0)
{
	dbuserror e;

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &(e.get_err()));

	if (e.is_set()) {
		std::cerr << "[xdpd][dbus] connection failed:" << e.get_err().message << std::endl;
		throw eDBusConnFailed();
	}
	if (0 == conn) {
		std::cerr << "[xdpd][dbus] connection failed." << std::endl;
		throw eDBusConnFailed();
	}
}



dbusconnection::~dbusconnection()
{
	dbus_connection_close(conn);
	std::cerr << "[xdpd][dbus] connection closed." << std::endl;
}



void
dbusconnection::request_name(
		std::string const& name)
{
	dbuserror e;

	int rc = 0;

	rc = dbus_bus_request_name(conn, name.c_str(), DBUS_NAME_FLAG_REPLACE_EXISTING, &(e.get_err()));

	if (e.is_set()) {
		std::cerr << "[xdpd][dbus] requesting name failed:" << e.get_err().message << std::endl;
		throw eDBusConnReqName();
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != rc) {
		std::cerr << "[xdpd][dbus] requesting name failed." << std::endl;
		throw eDBusConnReqName();
	}
}



