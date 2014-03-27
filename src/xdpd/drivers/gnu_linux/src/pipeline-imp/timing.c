#include <stdlib.h>
#include <sys/time.h>

int platform_gettimeofday(struct timeval * tval){
	return gettimeofday(tval, NULL);
}