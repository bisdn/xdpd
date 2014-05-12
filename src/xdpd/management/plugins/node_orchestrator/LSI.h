#ifndef LSI_H_
#define LSI_H_ 1

#pragma once

#include <map>
#include <inttypes.h>
#include <map>
#include <string>

using namespace std;

namespace xdpd
{

class LSI
{
private:
	uint64_t dpid;
	map<string,unsigned int> ports;
	
public:
	LSI(uint64_t dpid, map<string,unsigned int> ports);

	uint64_t getDpid();
	map<string,unsigned int> getPorts();
};

}

#endif //LSI_H_
