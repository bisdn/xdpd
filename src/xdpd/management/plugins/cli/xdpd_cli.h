/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * xdpd_cli.h
 *
 *  Created on: Sep 14, 2012
 *      Author: tobi
 */

#include <rofl/common/config/ccli.h>
#include "../../plugin_manager.h"

#ifndef XDPD_CLI_H_
#define XDPD_CLI_H_

using namespace rofl;

namespace xdpd 
{
class xdpd_cli : public rofl::ccli, public plugin
{
public:
	xdpd_cli(rofl::caddress addr = rofl::caddress(AF_INET, "127.0.0.1", 6620));

	virtual void init(int argc, char** argv);

	virtual std::string get_name(){
		return std::string("xdpd_cli");
	}

	virtual
	~xdpd_cli();

};

}; // namespace rofl

#endif /* XDPD_CLI_H_ */
