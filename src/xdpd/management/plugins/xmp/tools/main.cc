#include <assert.h>

#include <string>

#include "cxmpclient.h"

int 
main(int argc, char** argv)
{
	rofl::logging::init();
	rofl::logging::set_debug_level(rofl::logging::DBG);

	xdpd::mgmt::protocol::cxmpclient xmpclient;


	if ((argc >= 2) && (std::string(argv[1]) == std::string("port"))) {

		// xmpclient port attach <dpid> <portname>
		if ((argc == 5) && (std::string(argv[2]) == std::string("attach"))) {
			xmpclient.port_attach(strtol(argv[3], NULL, 0), std::string(argv[4]));

		// xmpclient port detach <dpid> <portname>
		} else if ((argc == 5) && (std::string(argv[2]) == std::string("detach"))) {
			xmpclient.port_detach(strtol(argv[3], NULL, 0), std::string(argv[4]));

		// xmpclient port enable <portname>
		} else if ((argc == 4) && (std::string(argv[2]) == std::string("enable"))) {
			xmpclient.port_enable(std::string(argv[3]));

		// xmpclient port disable <portname>
		} else if ((argc == 4) && (std::string(argv[2]) == std::string("disable"))) {
			xmpclient.port_disable(std::string(argv[3]));

		} else {
			// todo usage
			return -1;
		}
	} else {
		// todo usage
		return -1;
	}

	xmpclient.run();

	sleep(2);

	return 0;
}

