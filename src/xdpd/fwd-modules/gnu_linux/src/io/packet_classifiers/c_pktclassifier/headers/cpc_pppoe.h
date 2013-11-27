/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_PPPOE_H_
#define _CPC_PPPOE_H_

#include "../cpc_utils.h"

/**
* @file cpc_pppoe.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for PPPoE
*/

#define DEFAULT_PPPOE_FRAME_SIZE 1492

/* PPPoE constants and definitions */
enum pppoe_version_t {
	PPPOE_VERSION = 0x01,	// PPPoE version
};

enum pppoe_type_t {
	PPPOE_TYPE = 0x01,	// PPPoE type
};

// PPPoE ethernet types
enum pppoe_ether_t {
	PPPOE_ETHER_DISCOVERY = 0x8863,
	PPPOE_ETHER_SESSION = 0x8864,
};

// PPPoE codes
enum pppoe_code_t {
	PPPOE_CODE_SESSION_DATA = 0x00,
	PPPOE_CODE_PADO = 0x07,
	PPPOE_CODE_PADI = 0x09,
	PPPOE_CODE_PADR = 0x19,
	PPPOE_CODE_PADS = 0x65,
	PPPOE_CODE_PADT = 0xa7,
};

// PPPoE tags
enum pppoe_tag_t {
	PPPOE_TAG_END_OF_LIST = 0x0000,
	PPPOE_TAG_SERVICE_NAME = 0x0101,
	PPPOE_TAG_AC_NAME = 0x0102,
	PPPOE_TAG_HOST_UNIQ = 0x0103,
	PPPOE_TAG_AC_COOKIE = 0x0104,
	PPPOE_TAG_VENDOR_SPECIFIC = 0x0105,
	PPPOE_TAG_RELAY_SESSION_ID = 0x0110,
	PPPOE_TAG_SERVICE_NAME_ERROR = 0x0201,
	PPPOE_TAG_AC_SYSTEM_ERROR = 0x0202,
	PPPOE_TAG_GENERIC_ERROR = 0x0203,
};

struct cpc_pppoe_hdr {
	uint8_t 	verstype;
	uint8_t 	code;
	uint16_t 	sessid;
	uint16_t 	length;
	uint8_t 	data[0];
} __attribute__((packed));

typedef struct cpc_pppoe_hdr cpc_pppoe_hdr_t;

struct cpc_pppoe_tag_hdr_t {
	uint16_t type;
	uint16_t length;
	uint8_t data[0];
};

inline static
uint8_t get_pppoe_vers(void *hdr){
	return (( ((cpc_pppoe_hdr_t*)hdr)->verstype & 0xf0 ) >>4 );
}

inline static
void set_pppoe_vers(void *hdr, uint8_t version){
	((cpc_pppoe_hdr_t*)hdr)->verstype = (((cpc_pppoe_hdr_t*)hdr)->verstype & 0x0f) + ((version & 0x0f) << 4);
}

inline static
uint8_t get_pppoe_type(void *hdr){
	return (((cpc_pppoe_hdr_t*)hdr)->verstype & 0x0f);
}

inline static
void set_pppoe_type(void *hdr, uint8_t type){
	((cpc_pppoe_hdr_t*)hdr)->verstype = (((cpc_pppoe_hdr_t*)hdr)->verstype & 0xf0) + (type & 0x0f);
}

inline static
uint8_t get_pppoe_code(void *hdr){
	return ((cpc_pppoe_hdr_t*)hdr)->code;
}

inline static
void set_pppoe_code(void *hdr, uint8_t code){
	((cpc_pppoe_hdr_t*)hdr)->code = code;
}


inline static
uint16_t get_pppoe_sessid(void *hdr){
	return CPC_BE16TOH(((cpc_pppoe_hdr_t*)hdr)->sessid);
}

inline static
void set_pppoe_sessid(void *hdr, uint16_t sessid){
	((cpc_pppoe_hdr_t*)hdr)->sessid = CPC_HTOBE16(sessid);
}

inline static
uint16_t get_pppoe_length(void *hdr){
	return CPC_BE16TOH(((cpc_pppoe_hdr_t*)hdr)->length);
}

inline static
void set_pppoe_length(void *hdr, uint16_t length){
	((cpc_pppoe_hdr_t*)hdr)->length = CPC_HTOBE16(length);
}

#endif //_CPC_PPPOE_H_
