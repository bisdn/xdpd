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
};

// message command types (MCT)
enum xmpie_command_t {
	XMPIEMCT_NONE				= 0,
	XMPIEMCT_PORT_ATTACH		= 1,
	XMPIEMCT_PORT_DETACH		= 2,
	XMPIEMCT_PORT_ENABLE		= 3,
	XMPIEMCT_PORT_DISABLE		= 4,
	XMPIEMCT_PORT_LIST			= 5,
	XMPIEMCT_PORT_INFO			= 6,
	XMPIEMCT_LSI_LIST			= 7,
	XMPIEMCT_LSI_INFO			= 8,
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
	XMPIET_MULTIPART	= 4,
	XMPIET_PORTINFO		= 5,
	XMPIET_LSINAME		= 6,
	XMPIET_LSIINFO		= 7,
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

#define XMPIE_NAME_SIZE 32

struct xmp_ie_name_t {
	uint16_t	type;
	uint16_t	len;	// including header and payload
	char		name[XMPIE_NAME_SIZE];
} __attribute__((packed));

struct xmp_ie_portinfo_t {
	uint16_t	type;
	uint16_t	len;	// including header and payload
	uint32_t	of_port_num;
	char		portname[XMPIE_NAME_SIZE];

	/* see port_features_t in rofl-core/src/rofl/datapath/pipeline/switch_port.h */
	uint32_t	feat_curr;			/* Current features. */
	uint32_t	feat_supported;		/* Features supported by the port. */
	uint32_t	feat_peer;			/* Features advertised by peer. */

	uint64_t	curr_speed;			/* Current port bitrate in kbps. */
	uint64_t	max_speed;			/* Max port bitrate in kbps */

	uint32_t	state;				/* see port_state_t in rofl-core/src/rofl/datapath/pipeline/switch_port.h */
} __attribute__((packed));

struct xmp_ie_lsiinfo_t {
	uint16_t	type;
	uint16_t	len;	// including header and payload
	char		lsiname[XMPIE_NAME_SIZE];

	/* see enum of1x_capabilities in rofl-core/src/rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h */
	uint32_t	capabilities;		/* Current features. */
	uint32_t	num_of_buffers;
	uint32_t	max_ports;
	uint8_t		max_tables;
} __attribute__((packed));


#endif /* MGMT_PROTOCOL_H_ */


