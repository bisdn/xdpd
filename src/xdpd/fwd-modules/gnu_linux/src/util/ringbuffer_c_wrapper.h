#ifndef RINGBUFFER_C_WRAPPER_H_
#define RINGBUFFER_C_WRAPPER_H_

#include <rofl.h>

//Opaque pointer
typedef void* ringbuffer_handle_p;

//C++ extern C
ROFL_BEGIN_DECLS

ringbuffer_handle_p
new_ringbuffer(void);

void
delete_ringbuffer(ringbuffer_handle_p);

//C++ extern C
ROFL_END_DECLS

#endif //RINGBUFFER_C_WRAPPER_H_
