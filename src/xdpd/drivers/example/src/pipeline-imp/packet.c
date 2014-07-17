/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PLATFORM_PACKET_HOOKS_IMPL
#define PLATFORM_PACKET_HOOKS_IMPL

#include <stdint.h>
#include <rofl/datapath/pipeline/platform/packet.h>

/**
* @file packet.c
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief This is an *empty* implementation of the packet mangling API. 
*
* To get more information about the PP API you should have a look on:
*
* src/rofl/datapath/pipeline/platform/packet.h
* 
* from ROFL-pipeline library.
*
* For a full-fledged PP API implementation, using the reference classifier
* take a look at the reference driver (gnu-linux)
*
*/

//
// Configuration to include and empty PP API routines 
//

//This will instruct to conditionally compile an empty set of
//functions when including packet_proto_meta_imp.h
#define EMPTY_PACKET_PROCESSING_ROUTINES 

//Now do the include
#include "packet_proto_meta_imp.h"

/*
* The res of functions here defined are very specific to the platform and are
* not provided by the packet_proto_meta_imp.h file
*/

//Packet dropping
void platform_packet_drop(datapacket_t* pkt){
	//Do nothing...
}

//Packet replication 
datapacket_t* platform_packet_replicate(datapacket_t* pkt){
	return NULL;
}

//Output to a port
void platform_packet_output(datapacket_t* pkt, switch_port_t* output_port){
	//Do nothing...
}


#endif //PLATFORM_PACKET_HOOKS_IMPL
