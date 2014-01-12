/*
 * dbusmod.h
 *
 *  Created on: 10.01.2014
 *      Author: andreas
 */

#ifndef DBUSMOD_H_
#define DBUSMOD_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <dbus/dbus.h>
#ifdef __cplusplus
}
#endif

#include <rofl/common/ciosrv.h>

#include "../../switch_manager.h"
#include "../../port_manager.h"
#include "../../plugin_manager.h"
#include "../../../openflow/openflow_switch.h"

#include "dbusconnection.h"
#include "dbuserror.h"

namespace xdpd {
namespace dbus {

class dbusmod :
		public ciosrv,
		public plugin
{
	dbusconnection 		conn;		// dbus connection

public:

	/**
	 *
	 */
	dbusmod();

	/**
	 *
	 */
	~dbusmod();

	/**
	 *
	 */
	virtual void
	init(int argc, char** argv);

	/**
	* Returns the plugin name
	*/
	virtual std::string
	get_name(void) {
		return std::string("dbus");
	};

public:

	/**
	 *
	 */
	virtual void
	handle_timeout(
			int opaque);


	/**
	 *
	 */
	virtual void
	handle_revent(
			int fd);
};

}; // end of namespace dbus
}; // end of namespace xdpd

#endif /* DBUSMOD_H_ */
