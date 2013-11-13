#include "grouptest.h"


enum fruits_t {
		APPLE = 1000,
		PEACH = 1001,
		STRAWBERRY = 1002,
};

int
main(int argc, char** argv)
{
	fruits_t fruit = PEACH;

	int state = fruit;

	fprintf(stderr, "state: %d\n", state);

	return 0;

	cgrouptest grouptest;

	grouptest.rpc_listen_for_dpts(rofl::caddress(AF_INET, "0.0.0.0", 6633));

	grouptest.run();

	return 0;
}

