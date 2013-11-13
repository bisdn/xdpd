#include "grouptest.h"


int
main(int argc, char** argv)
{
	cgrouptest grouptest;

	grouptest.rpc_listen_for_dpts(rofl::caddress(AF_INET, "0.0.0.0", 6633));

	grouptest.run();

	return 0;
}

