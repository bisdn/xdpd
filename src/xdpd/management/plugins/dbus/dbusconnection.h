/*
 * dbusconnection.h
 *
 *  Created on: 10.01.2014
 *      Author: andreas
 */

#ifndef DBUSCONNECTION_H_
#define DBUSCONNECTION_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <dbus/dbus.h>
#ifdef __cplusplus
}
#endif

#include "dbuserror.h"
#include "rofl/common/cerror.h"

namespace xdpd {
namespace dbus {

class eDBusConnBase 		: public rofl::cerror {};
class eDBusConnFailed		: public eDBusConnBase {};
class eDBusConnReqName		: public eDBusConnBase {};

class dbusconnection {

	DBusConnection		*conn;

public:

	/**
	 *
	 */
	dbusconnection();

	/**
	 *
	 */
	~dbusconnection();

public:

	/**
	 *
	 */
	void
	request_name(
			std::string const& name);

};

}; // end of namespace dbus
}; // end of namespace xdpd

#endif /* DBUSCONNECTION_H_ */
