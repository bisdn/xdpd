#include "LSI.h"

namespace xdpd
{

LSI::LSI(uint64_t dpid, map<string,unsigned int> ports)
		: dpid(dpid), ports(ports) {}


uint64_t LSI::getDpid()
{
	return dpid;
}


map<string,unsigned int> LSI::getPorts()
{
	return ports;
}

}
