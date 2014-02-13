#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/platform/memory.h>


/*
 * malloc and free should be replaced if necessary by platform specific memory allocators
 */

//Uncomment this line when you are inlining them in the pipeline
//#define STATIC_INLINE__ //static inline
#define STATIC_INLINE__ 


//Per core memory allocators
STATIC_INLINE__ void* platform_malloc( size_t length ){
	return malloc( length );
}
STATIC_INLINE__ void platform_free( void *data ){
	free( data );
}

//Shared memory allocators
STATIC_INLINE__ void* platform_malloc_shared( size_t length ){
	return malloc( length );
}

STATIC_INLINE__ void platform_free_shared( void *data ){
	free( data );
}

STATIC_INLINE__ void* platform_memcpy( void *dst, const void *src, size_t length ){
	return memcpy( dst, src, length );
}

STATIC_INLINE__ void* platform_memset( void *src, int c, size_t length ){
	return memset( src, c, length );
}

STATIC_INLINE__ void* platform_memmove( void *dst, const void *src, size_t length ){
	return memmove( dst, src, length );
}
