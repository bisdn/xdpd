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
	XMPT_ERROR				= 0,
	XMPT_REQUEST			= 1,
	XMPT_REPLY				= 2,
	XMPT_NOTIFICATION		= 3,
	XMPT_REPLY_MULTIPART	= 4,
};

// message command types (MCT)
enum xmpie_command_t {
	XMPIEMCT_NONE				= 0,
	XMPIEMCT_PORT_ATTACH		= 1,
	XMPIEMCT_PORT_DETACH		= 2,
	XMPIEMCT_PORT_ENABLE		= 3,
	XMPIEMCT_PORT_DISABLE		= 4,
	XMPIEMCT_PORT_LIST			= 5,
};

struct xmp_header_t {
	uint8_t		version;
	uint8_t		type;
	uint16_t	len;	// including header and payload
	uint32_t	xid;
	uint8_t		data[0];
} __attribute__((packed));

struct xmp_msg_port_configuration_t {
	struct xmp_header_t		xmp_header;
	char					portname[PORTNAMESIZE];
	uint32_t				config;
} __attribute__((packed));

struct xmp_msg_port_attachment_t {
	struct xmp_header_t		xmp_header;
	char					portname[PORTNAMESIZE];
	uint64_t				dpid;
} __attribute__((packed));

// information element types
enum xmpie_type_t {
	XMPIET_NONE			= 0,
	XMPIET_COMMAND		= 1,
	XMPIET_PORTNAME		= 2,
	XMPIET_DPID			= 3,
	XMPIET_MULTIPART	= 4
};

struct xmp_ie_header_t {
	uint16_t	type;
	uint16_t	len;	// including header and payload
	uint8_t 	data[0];
} __attribute__((packed));

struct xmp_ie_command_t {
	uint16_t	type;
	uint16_t	len;	// including header and payload
	uint32_t	cmd;
} __attribute__((packed));

struct xmp_ie_dpid_t {
	uint16_t	type;
	uint16_t	len;	// including header and payload
	uint64_t	dpid;
} __attribute__((packed));

#define XMPIE_PORTNAME_SIZE 32

struct xmp_ie_portname_t {
	uint16_t	type;
	uint16_t	len;	// including header and payload
	char		portname[XMPIE_PORTNAME_SIZE];
} __attribute__((packed));

#endif /* MGMT_PROTOCOL_H_ */


