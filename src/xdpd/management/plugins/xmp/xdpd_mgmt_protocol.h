/*
 * mgmt_protocol.h
 *
 *  Created on: 11.01.2014
 *      Author: andreas
 */

#ifndef MGMT_PROTOCOL_H_
#define MGMT_PROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>
#ifdef __cplusplus
}
#endif

#define XMP_VERSION 1
#define PORTNAMESIZE 32

enum xmp_msg_t {
	XMPT_PORT_ATTACH	= 1,
	XMPT_PORT_DETACH	= 2,
	XMPT_PORT_ENABLE	= 3,
	XMPT_PORT_DISABLE	= 4,
};

struct xmp_header_t {
	uint8_t		version;
	uint8_t		type;
	uint16_t	len;	// header + data
	uint32_t	xid;
} __attribute__((packed));

struct xmp_msg_port_enable_t {
	struct xmp_header_t		xmp_header;
	char					portname[PORTNAMESIZE];
} __attribute__((packed));

struct xmp_msg_port_attachment_t {
	struct xmp_header_t		xmp_header;
	char					portname[PORTNAMESIZE];
	uint64_t				dpid;
} __attribute__((packed));

#endif /* MGMT_PROTOCOL_H_ */


