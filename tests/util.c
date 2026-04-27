#include "util.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef LIBSOCKET_OS_WINDOWS
    #include <windows.h>
#else
    #include <unistd.h>
    #include <time.h>
#endif

void waitms(uint32_t milliseconds)
{
    #ifdef LIBSOCKET_OS_WINDOWS
        Sleep(milliseconds);
    #else
        struct timespec d;
        d.tv_sec = milliseconds / 1000;
        d.tv_nsec = (milliseconds % 1000) * 1000000;
        nanosleep(&d, NULL);
    #endif
}

void handleerror(const char *funcname)
{
    printf("%s error: %s. Application aborted.\n", funcname, socket_strerror(socket_getlasterror()));
    abort();
}