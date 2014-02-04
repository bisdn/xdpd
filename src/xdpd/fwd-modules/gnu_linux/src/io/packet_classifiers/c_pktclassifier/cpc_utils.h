/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_UTILS_H_
#define _CPC_UTILS_H_

#include <rofl/common/endian_conversion.h>
#include <rofl/datapath/pipeline/common/large_types.h>

/**
* @file cpc_utils.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Swap definitions to switch the classifier into Big Endian
*/

#define SWAP_MAC(x) do{ \
	x=__bswap_64(x); \
	x>>=16; \
}while(0)

#define CPC_IN_HOSTBYTEORDER
#ifdef CPC_IN_HOSTBYTEORDER
#  define CPC_HTOBE16(x) htobe16(x)
#  define CPC_HTOLE16(x) htole16(x)
#  define CPC_BE16TOH(x) be16toh(x)
#  define CPC_LE16TOH(x) le16toh(x)

#  define CPC_HTOBE32(x) htobe32(x)
#  define CPC_HTOLE32(x) htole32(x)
#  define CPC_BE32TOH(x) be32toh(x)
#  define CPC_LE32TOH(x) le32toh(x)

#  define CPC_HTOBE64(x) htobe64(x)
#  define CPC_HTOLE64(x) htole64(x)
#  define CPC_BE64TOH(x) be64toh(x)
#  define CPC_LE64TOH(x) le64toh(x)

#	if __BYTE_ORDER == __LITTLE_ENDIAN
#		define CPC_SWAP_U128(x) SWAP_U128(x)
#		define CPC_SWAP_MAC(x) SWAP_MAC(x)
#	else
#		define CPC_SWAP_U128(x)
#		define CPC_SWAP_MAC(x)
#	endif

#else
#  define CPC_HTOBE16(x) (x)
#  define CPC_HTOLE16(x) (x)
#  define CPC_BE16TOH(x) (x)
#  define CPC_LE16TOH(x) (x)

#  define CPC_HTOBE32(x) (x)
#  define CPC_HTOLE32(x) (x)
#  define CPC_BE32TOH(x) (x)
#  define CPC_LE32TOH(x) (x)

#  define CPC_HTOBE64(x) (x)
#  define CPC_HTOLE64(x) (x)
#  define CPC_BE64TOH(x) (x)
#  define CPC_LE64TOH(x) (x)

#  define CPC_SWAP_U128(x)
#  define CPC_SWAP_MAC(x)
#endif

#endif //_CPC_UTILS_