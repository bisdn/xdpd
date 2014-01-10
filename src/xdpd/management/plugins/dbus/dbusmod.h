/*
 * dbusmod.h
 *
 *  Created on: 10.01.2014
 *      Author: andreas
 */

#ifndef DBUSMOD_H_
#define DBUSMOD_H_

#include "../../switch_manager.h"
#include "../../port_manager.h"
#include "../../plugin_manager.h"
#include "../../../openflow/openflow_switch.h"

namespace xdpd {
namespace dbus {

class dbusmod :
	public plugin {
public:

	dbusmod();
	~dbusmod();

	virtual void init(int argc, char** argv) {};

	/**
	* Returns the plugin name
	*/
	virtual std::string get_name(void) { return std::string(""); };

};

}; // end of namespace dbus
}; // end of namespace xdpd

#endif /* DBUSMOD_H_ */
