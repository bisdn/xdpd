
#include "ringbuffer_c_wrapper.h"
#include "ringbuffer.h"


ringbuffer_handle_p
new_ringbuffer()
{
	return new ringbuffer();
}

void 
delete_ringbuffer(ringbuffer_handle_p rb)
{
	delete (ringbuffer*)rb;
}
