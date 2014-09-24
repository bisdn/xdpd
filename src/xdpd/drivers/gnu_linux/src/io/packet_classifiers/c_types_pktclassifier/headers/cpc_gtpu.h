/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_GTPU_H_
#define _CPC_GTPU_H_

#include "../../../../util/likely.h"
#include "../../../../util/compiler_assert.h"

/**
* @file cpc_gtpu.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for GTP
*/

// full GTP-Uv1 header with E, S, PN flags set to 1
struct cpc_gtpu_e_hdr {
	uint8_t 	flags;		// version, PT, T, E, S, PN
	uint8_t		msgtype;
	uint16_t	len;
	uint32_t	teid;
	uint16_t	seqno;
	uint8_t		n_pdu_no;	// N-PDU number
	uint8_t		exthdr;		// next extension header type
	uint8_t		data[0];	// pointer to start of data
}__attribute__((packed));
typedef struct cpc_gtpu_e_hdr cpc_gtpu_e_hdr_t;

// shortened GTP-Uv1 header with S and PN flags set to 1 only
struct cpc_gtpu_pn_hdr {
	uint8_t 	flags;		// version, PT, T, E, S, PN
	uint8_t		msgtype;
	uint16_t	len;
	uint32_t	teid;
	uint16_t	seqno;
	uint8_t		n_pdu_no;	// N-PDU number
	uint8_t		data[0];	// pointer to start of data
}__attribute__((packed));
typedef struct cpc_gtpu_pn_hdr cpc_gtpu_pn_hdr_t;

// shortened GTP-Uv1 header with S flag set to 1 only
struct cpc_gtpu_s_hdr {
	uint8_t 	flags;		// version, PT, T, E, S, PN
	uint8_t		msgtype;
	uint16_t	len;
	uint32_t	teid;
	uint16_t	seqno;
	uint8_t		data[0];	// pointer to start of data
}__attribute__((packed));
typedef struct cpc_gtpu_s_hdr cpc_gtpu_s_hdr_t;

// shortened GTP-Uv1 header with S, PN, E flags set to 0
struct cpc_gtpu_base_hdr {
	uint8_t 	flags;		// version, PT, T, E, S, PN
	uint8_t		msgtype;
	uint16_t	len;
	uint32_t	teid;
	uint8_t		data[0];	// pointer to start of data
}__attribute__((packed));
typedef struct cpc_gtpu_base_hdr cpc_gtpu_base_hdr_t;

typedef union cpc_gtphu{
	uint8_t				cpc_gtpu_hdr;
	cpc_gtpu_e_hdr_t 	cpc_gtpu_e_hdr;
	cpc_gtpu_pn_hdr_t	cpc_gtpu_pn_hdr;
	cpc_gtpu_s_hdr_t 	cpc_gtpu_s_hdr;
	cpc_gtpu_base_hdr_t cpc_gtpu_base_hdr;
}cpc_gtphu_t;


//NOTE PDULEN?

inline static
uint8_t* get_gtpu_version(void *hdr){
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags;
};

inline static
void set_gtpu_version(void *hdr, uint8_t version){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags = (((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags & ~OF1X_3MSBITS_MASK) | (version & OF1X_3MSBITS_MASK);
};

inline static
uint8_t* get_gtpu_pt_flag(void *hdr){
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags;
};

inline static
void set_gtpu_pt_flag(void *hdr, uint8_t pt_flag){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags = (((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags & ~OF1X_BIT4_MASK) | (pt_flag & OF1X_BIT4_MASK);
};

inline static
uint8_t* get_gtpu_e_flag(void *hdr){
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags;
};

inline static
void set_gtpu_e_flag(void *hdr, uint8_t ext_flag){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags = (((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags & ~OF1X_BIT2_MASK) | (ext_flag & OF1X_BIT2_MASK);
};

inline static
uint8_t* get_gtpu_s_flag(void *hdr){
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags;
};

inline static
void set_gtpu_s_flag(void *hdr, uint8_t seqno_flag){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags = (((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags & ~OF1X_BIT1_MASK) | (seqno_flag & OF1X_BIT1_MASK);
};

inline static
uint8_t* get_gtpu_pn_flag(void *hdr){
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags;
};

inline static
void set_gtpu_pn_flag(void *hdr, uint8_t npdu_flag){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags = (((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.flags & ~OF1X_BIT0_MASK) | (npdu_flag & OF1X_BIT0_MASK);
};

inline static
uint8_t* get_gtpu_msg_type(void *hdr){
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.msgtype;
};

inline static
void set_gtpu_msg_type(void *hdr, uint8_t msgtype){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.msgtype = msgtype;
};

inline static
uint16_t* get_gtpu_length(void *hdr){
	return &((cpc_gtphu_t*)hdr)->cpc_gtpu_base_hdr.len;
};

inline static
void set_gtpu_length(void *hdr, uint16_t length){
	((cpc_gtphu_t*)hdr)->cpc_gtpu_base_hdr.len = length;
};

inline static
uint32_t* get_gtpu_teid(void *hdr){
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.teid;
};

inline static
void set_gtpu_teid(void *hdr, uint32_t teid){
	((cpc_gtphu_t *)hdr)->cpc_gtpu_base_hdr.teid = teid;
};

inline static
uint16_t* get_gtpu_seq_no(void *hdr){
	if(unlikely(get_gtpu_s_flag(hdr)==NULL)){
		assert(0);
		return NULL;
	}
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_s_hdr.seqno;
};

inline static
void set_gtpu_seq_no(void *hdr, uint16_t seqno){
	if(unlikely(get_gtpu_s_flag(hdr)==NULL)){
		assert(0);
		return;
	}
	((cpc_gtphu_t *)hdr)->cpc_gtpu_s_hdr.seqno = seqno;
};

inline static
uint8_t* get_gtpu_npdu_no(void *hdr){
	if(unlikely(get_gtpu_pn_flag(hdr)==NULL)){
		assert(0);
		return NULL;
	}
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_pn_hdr.n_pdu_no;
};

inline static
void set_gtpu_npdu_no(void *hdr, uint8_t npdu){
	if(unlikely(get_gtpu_pn_flag(hdr)==NULL)){
		assert(0);
		return;
	}
	((cpc_gtphu_t *)hdr)->cpc_gtpu_pn_hdr.n_pdu_no = npdu;
};

inline static
uint8_t* get_gtpu_ext_type(void *hdr){
	if(unlikely(get_gtpu_e_flag(hdr)==NULL)){
		assert(0);
		return NULL;
	}
	return &((cpc_gtphu_t *)hdr)->cpc_gtpu_e_hdr.exthdr;
};

inline static
void set_gtpu_ext_type(void *hdr, uint8_t exthdr){
	if(unlikely(get_gtpu_e_flag(hdr)==NULL)){
		assert(0);
		return;
	}
	((cpc_gtphu_t *)hdr)->cpc_gtpu_e_hdr.exthdr = exthdr;
};

#endif //_CPC_GTPU_H_
