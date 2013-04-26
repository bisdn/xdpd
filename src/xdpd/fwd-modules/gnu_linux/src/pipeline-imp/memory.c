#include <stdlib.h>
#include <string.h>
#include <rofl/datapath/pipeline/platform/memory.h>


/*
 * malloc and free should be replaced if necessary by platform specific memory allocators
 */

//Per core memory allocators
void* platform_malloc( size_t length ){
	return malloc( length );
}
void platform_free( void *data ){
	free( data );
}

//Shared memory allocators
void* platform_malloc_shared( size_t length ){
	return malloc( length );
}

void platform_free_shared( void *data ){
	free( data );
}

void* platform_memcpy( void *dst, const void *src, size_t length ){
	return memcpy( dst, src, length );
}

void* platform_memset( void *src, int c, size_t length ){
	return memset( src, c, length );
}

void* platform_memmove( void *dst, const void *src, size_t length ){
	return memmove( dst, src, length );
}
