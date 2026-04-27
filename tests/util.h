#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include "libsocket.h"

#ifdef LIBSOCKET_OS_WINDOWS
    #include <windows.h>
    #define MILLIS(x) Sleep(x);
#else
    #include <unistd.h>
    #include <time.h>
    #define MILLIS(x) {\
            struct timespec delay;\
            delay.tv_sec = 0;\
            delay.tv_nsec = (x) * 1000000;\
            nanosleep(&delay, NULL);\
        }
#endif

void handleerror(const char *funcname)
{
    printf("%s error: %s. Application aborted.\n", funcname, socket_strerror(socket_getlasterror()));
    abort();
}

#endif