#ifndef UTIL_H
#define UTIL_H

#include "libsocket.h"

#include <limits.h>

#ifdef LIBSOCKET_OS_WINDOWS
    #define CLAMPSIZET(x) ((size_t)x > INT_MAX ? (int)INT_MAX : (int)x)
#else
    #define CLAMPSIZET(x) ((size_t)x)
#endif

#endif