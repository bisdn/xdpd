#include <assert.h>

#include <string>

#include "cxmpclient.h"

int 
main(int argc, char** argv)
{
	xdpd::mgmt::protocol::cxmpclient xmpclient;


	if ((argc >= 2) && (std::string(argv[1]) == std::string("port"))) {

		// xmpclient port attach <dpid> <portname>
		if ((argc >= 3) && (std::string(argv[2]) == std::string("attach"))) {
			assert(argc >= 4);
			xmpclient.port_attach(atoi(argv[3]), std::string(argv[4]));

		// xmpclient port detach <dpid> <portname>
		} else if ((argc >= 3) && (std::string(argv[2]) == std::string("detach"))) {
			assert(argc >= 4);
			xmpclient.port_detach(atoi(argv[3]), std::string(argv[4]));

		// xmpclient port enable <portname>
		} else if ((argc >= 3) && (std::string(argv[2]) == std::string("enable"))) {
			assert(argc >= 3);
			xmpclient.port_enable(std::string(argv[3]));

		// xmpclient port disable <portname>
		} else if ((argc >= 3) && (std::string(argv[2]) == std::string("disable"))) {
			assert(argc >= 3);
			xmpclient.port_disable(std::string(argv[3]));

		} else {

		}
	} else {

	}

	xmpclient.run();

	return 0;
}

