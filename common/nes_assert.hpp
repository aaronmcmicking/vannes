#pragma once

#include <assert.h>

#define VANNESS_ASSERT_ERRORS
#if defined(VANNESS_ASSERT_ERRORS)
    #define VNES_ASSERT(e) assert(e)
#else
    #define VNES_ASSERT
#endif
