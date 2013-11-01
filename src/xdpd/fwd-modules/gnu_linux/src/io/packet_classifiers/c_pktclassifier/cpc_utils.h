#ifndef _CPC_UTILS_H_
#define _CPC_UTILS_H_

#include <rofl/common/endian_conversion.h>
#include <rofl/datapath/pipeline/common/large_types.h>

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
#		define CPC_SWAP_U128(x) SWAP_U128(x); //htobe128
#	else
#		define CPC_SWAP_U128(x) (x)
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

# define CPC_SWAP_U128(x) (x)
#endif


#endif //_CPC_UTILS_