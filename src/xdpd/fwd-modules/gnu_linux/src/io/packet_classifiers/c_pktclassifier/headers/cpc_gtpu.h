/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_GTPU_H_
#define _CPC_GTPU_H_

#include "../cpc_utils.h"
#include "../../../../util/likely.h"
#include "../../../../util/compiler_assert.h"

enum gtpu_version_t {
	GTPU_VERS_1 = 1,
};

enum gtpu_udp_port_t {
	GTPU_UDP_PORT = 2152,
};

enum gtpu_flag_t {
	GTPU_PT_FLAG = (1 << 4),
	GTPU_E_FLAG  = (1 << 2),
	GTPU_S_FLAG  = (1 << 1),
	GTPU_PN_FLAG = (1 << 0),
};

// full GTP-Uv1 header with E, S, PN flags set to 1
struct cpc_gtpu_e_hdr_t {
	uint8_t 	flags;		// version, PT, T, E, S, PN
	uint8_t		msgtype;
	uint16_t	len;
	uint32_t	teid;
	uint16_t	seqno;
	uint8_t		n_pdu_no;	// N-PDU number
	uint8_t		exthdr;		// next extension header type
	uint8_t		data[0];	// pointer to start of data
};//NOTE __attribute__((packed));?

// shortened GTP-Uv1 header with S and PN flags set to 1 only
struct cpc_gtpu_pn_hdr_t {
	uint8_t 	flags;		// version, PT, T, E, S, PN
	uint8_t		msgtype;
	uint16_t	len;
	uint32_t	teid;
	uint16_t	seqno;
	uint8_t		n_pdu_no;	// N-PDU number
	uint8_t		data[0];	// pointer to start of data
};//NOTE __attribute__((packed));?

// shortened GTP-Uv1 header with S flag set to 1 only
struct cpc_gtpu_s_hdr_t {
	uint8_t 	flags;		// version, PT, T, E, S, PN
	uint8_t		msgtype;
	uint16_t	len;
	uint32_t	teid;
	uint16_t	seqno;
	uint8_t		data[0];	// pointer to start of data
};//NOTE __attribute__((packed));?

// shortened GTP-Uv1 header with S, PN, E flags set to 0
struct cpc_gtpu_base_hdr_t {
	uint8_t 	flags;		// version, PT, T, E, S, PN
	uint8_t		msgtype;
	uint16_t	len;
	uint32_t	teid;
	uint8_t		data[0];	// pointer to start of data
};//NOTE __attribute__((packed));?

union cpc_gtphu{
	uint8_t*					cpc_gtpu_hdr;
	struct cpc_gtpu_e_hdr_t* 		cpc_gtpu_e_hdr;
	struct cpc_gtpu_pn_hdr_t*		cpc_gtpu_pn_hdr;
	struct cpc_gtpu_s_hdr_t* 		cpc_gtpu_s_hdr;
	struct cpc_gtpu_base_hdr_t* 	cpc_gtpu_short_hdr;
};

typedef union cpc_gtphu cpc_gtphu_t;

//NOTE PDULEN?

inline static
uint8_t get_gtp_version(void *hdr){
	return (((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags & 0xe0) >> 5;
};

inline static
void set_gtp_version(void *hdr, uint8_t version){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags = (((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags & 0x1f) | ((version & 0x03) << 5);
};

inline static
bool get_pt_flag(void *hdr){
	return ((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags & GTPU_PT_FLAG; 
};

inline static
void set_pt_flag(void *hdr, bool pt){
	if (pt)
		((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags |=  GTPU_PT_FLAG;
	else
		((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags &= ~GTPU_PT_FLAG;
};

inline static
bool get_e_flag(void *hdr){
	return ((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags & GTPU_E_FLAG; 
};

inline static
void set_e_flag(void *hdr, bool e){
	if (e)
		((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags |=  GTPU_E_FLAG;
	else
		((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags &= ~GTPU_E_FLAG;
};

inline static
bool get_s_flag(void *hdr){
	return ((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags & GTPU_S_FLAG; 
};

inline static
void set_s_flag(void *hdr, bool s){
	if (s)
		((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags |=  GTPU_S_FLAG;
	else
		((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags &= ~GTPU_S_FLAG;
};

inline static
bool get_pn_flag(void *hdr){
	return ((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags & GTPU_PN_FLAG; 
};

inline static
void set_pn_flag(void *hdr, bool pn){
	if (pn)
		((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags |=  GTPU_PN_FLAG;
	else
		((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->flags &= ~GTPU_PN_FLAG;
};

inline static
uint8_t get_msg_type(void *hdr){
	return ((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->msgtype;
};

inline static
void set_msg_type(void *hdr, uint8_t msgtype){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->msgtype = msgtype;
};

inline static
uint16_t get_gtp_length(void *hdr){
	return CPC_BE16TOH(((cpc_gtphu_t*)hdr)->cpc_gtpu_short_hdr->len);
};

inline static
void set_gtp_length(void *hdr, uint16_t length){
	((cpc_gtphu_t*)hdr)->cpc_gtpu_short_hdr->len = CPC_HTOBE16(length);
};

inline static
uint32_t get_teid(void *hdr){
	return CPC_BE32TOH(((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->teid);
};

inline static
void set_teid(void *hdr, uint32_t teid){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_short_hdr->teid = CPC_HTOBE32(teid);
};

inline static
uint16_t get_seq_no(void *hdr){
	if(unlikely(get_s_flag(hdr))){
		assert(0);
		return 0;
	}
	return CPC_BE16TOH(((cpc_gtphu_t *)hdr)->cpc_gtpu_s_hdr->seqno);
};

inline static
void set_seq_no(void *hdr, uint16_t seqno){
	if(unlikely(get_s_flag(hdr)==false)){
		assert(0);
		return;
	}
	((cpc_gtphu_t *)hdr)->cpc_gtpu_s_hdr->seqno = CPC_HTOBE16(seqno);
};

inline static
uint8_t get_npdu_no(void *hdr){
	if(unlikely(get_pn_flag(hdr)==false)){
		assert(0);
		return 0;
	}
	return ((cpc_gtphu_t *)hdr)->cpc_gtpu_pn_hdr->n_pdu_no;
};

inline static
void set_npdu_no(void *hdr, uint8_t npdu){
	if(unlikely(get_pn_flag(hdr)==false)){
		assert(0);
		return;
	}
	((cpc_gtphu_t *)hdr)->cpc_gtpu_pn_hdr->n_pdu_no = npdu;
};

inline static
uint8_t get_ext_type(void *hdr){
	if(unlikely(get_e_flag(hdr)==false)){
		assert(0);
		return 0;
	}
	return ((cpc_gtphu_t *)hdr)->cpc_gtpu_e_hdr->exthdr;
};

inline static
void set_ext_type(void *hdr, uint8_t exthdr){
	if(unlikely(get_e_flag(hdr)==false)){
		assert(0);
		return;
	}
	((cpc_gtphu_t *)hdr)->cpc_gtpu_e_hdr->exthdr = exthdr;
};

#endif //_CPC_GTPU_H_
