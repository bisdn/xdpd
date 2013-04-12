/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//FIXME: Put this into the config file

#define XDPD_LOG_FILE "xdpd.log"

#define XDPD_CLI_IP "127.0.0.1"
#define XDPD_CLI_TCP_PORT "6620"

#include <rofl/common/ciosrv.h>
#include <rofl/common/caddress.h>
#include <rofl/datapath/pipeline/physical_switch.h>

namespace rofl {

class xdpd :
	public ciosrv
{
private:

		static xdpd *instance;

private:


		/**
		 *
		 */
		xdpd() {};

		/**
		 *
		 */
		virtual
		~xdpd() {};


public:

		/**
		 *
		 */
		static xdpd&
		getInstance()
		{
			if (xdpd::instance == NULL)
			{
				instance = new xdpd();
			}
			return (*instance);
		}

		/**
		 *
		 */
		void
		platform_init();

		/**
		 *
		 */
		void
		platform_destroy();

		/**
		 *
		 */
		void
		logical_switch_init(
				std::string const& s_lib_driver,
				std::string const& s_dpname,
				uint64_t dpid,
				caddress const& rpc_ctl_addr);

		/**
		 *
		 */
		void
		logical_switch_destroy(
				uint64_t dpid);

};

}  // namespace rofl
