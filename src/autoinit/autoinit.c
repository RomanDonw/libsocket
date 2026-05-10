#include <stdio.h>
#include <stdlib.h>

#include "libsocket.h"

char __libsocket_autoinit_dummylnk = (char)0;

#ifdef _MSC_VER
    #define LIBSOCKET_STUPATTR
    #define LIBSOCKET_CLUPATTR
#else
    #define LIBSOCKET_STUPATTR __attribute__((constructor(101)))
    #define LIBSOCKET_CLUPATTR __attribute__((destructor))
#endif

static void LIBSOCKET_STUPATTR libsocket_autostartup(void)
{
    if (socket_isinited()) return;

    if (!socket_startup())
    {
        fprintf(stderr, "[libsocket]: auto startup error: %s. Application aborted.\n", socket_strerror(socket_lasterror));
        abort();
    }
}

static void LIBSOCKET_CLUPATTR libsocket_autocleanup(void)
{
    if (!socket_isinited()) return;
    
    if (!socket_cleanup())
    {
        fprintf(stderr, "[libsocket]: auto cleanup error: %s. Application aborted.\n", socket_strerror(socket_lasterror));
        abort();
    }
}

#ifdef _MSC_VER
    static void libsocket_MSVCautostartup(void)
    {
        libsocket_autostartup();
        if (atexit(libsocket_autocleanup))
        {
            fprintf(stderr, "[libsocket]: error binding library auto cleanup callback in \"atexit\" C function. Application aborted.\n");
            libsocket_autocleanup();
            abort();
        }
    }

    #pragma section(".CRT$XCU", read)
    __declspec(allocate(".CRT$XCU")) void (*libsocket_MSVCautostartup__ptr)(void) = libsocket_MSVCautostartup;
#endif