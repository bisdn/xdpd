#include "openflow_switch.h"

using namespace rofl;
using namespace xdpd;

openflow_switch::openflow_switch(const uint64_t dpid, const std::string &dpname, const of_version_t version) :
		ofswitch(NULL),
		endpoint(NULL),
		dpid(dpid),
		dpname(dpname),
		version(version)
{

}


