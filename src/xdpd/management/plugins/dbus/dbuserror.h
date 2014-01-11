/*
 * dbuserr.h
 *
 *  Created on: 10.01.2014
 *      Author: andreas
 */

#ifndef DBUSERR_H_
#define DBUSERR_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <dbus/dbus.h>
#ifdef __cplusplus
}
#endif

namespace xdpd {
namespace dbus {

class dbuserror {

	DBusError		err;

public:

	/**
	 *
	 */
	dbuserror();

	/**
	 *
	 */
	~dbuserror();

	/**
	 *
	 */
	DBusError&
	get_err() { return err; };

	/**
	 *
	 */
	bool
	is_set() const { return dbus_error_is_set(&err); };
};

}; // end of namespace dbus
}; // end of namespace xdpd

#endif /* DBUSERR_H_ */
