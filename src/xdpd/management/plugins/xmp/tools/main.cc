#include <assert.h>

#include <string>

#include "cxmpclient.h"

int 
main(int argc, char** argv)
{
#ifdef DEBUG
	rofl::logging::init();
	rofl::logging::set_debug_level(rofl::logging::DBG);
#endif

	xdpd::mgmt::protocol::cxmpclient xmpclient;


	if ((argc >= 2)) {

		if (std::string(argv[1]) == std::string("port")) {

			if ((argc >= 3) && (std::string(argv[2]) == std::string("list"))) {
				if (argc == 4) {
					// xmpclient port list <dpid>
					xmpclient.port_list(strtol(argv[3], NULL, 0));
				} else {
					// xmpclient port list
					xmpclient.port_list();
				}

				// xmpclient port attach <dpid> <portname>
			} else if ((argc == 5) && (std::string(argv[2]) == std::string("attach"))) {
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
		} else if (std::string(argv[1]) == std::string("lsi")) {

			// xmpclient lsi list
			if ((argc == 3) && (std::string(argv[2]) == std::string("list"))) {
				xmpclient.lsi_list();
			} else {
				return -1;
			}

		} else {
			// todo usage
			return -1;
		}
	} else {
		// todo usage
		return -1;
	}

	rofl::cioloop::get_loop().run();

	//Logging
	rofl::logging::close();

	//Release ciosrv loop resources
	rofl::cioloop::get_loop().shutdown();

	return 0;
}

