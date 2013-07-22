#ifndef _COMPILER_ASSERT_H_
#define _COMPILER_ASSERT_H_

#include <assert.h>

#define COMPILER_ASSERT(tag,cond) \
        enum { COMPILER_ASSERT__ ## tag = 1/(cond) }

#endif //COMPILER_ASSERT_H
