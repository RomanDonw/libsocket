#include "libsocket.h"
#include "init.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

volatile void *___ = NULL;

#ifdef _MSC_VER
    #define LIBSOCKET_INITATTR
    #define LIBSOCKET_CLUPATTR
#else
    #define LIBSOCKET_INITATTR __attribute__((constructor(101)))
    #define LIBSOCKET_CLUPATTR __attribute__((destructor))
#endif

bool inited = false;

static void LIBSOCKET_INITATTR libsocket_WSAInit(void)
{
    if (inited) return;

    #ifdef LIBSOCKET_OS_WINDOWS
        const WORD version = MAKEWORD(2, 2);

        WSADATA data;
        int err = WSAStartup(version, &data);
        if (err)
        {
            fprintf(stderr, "[libsocket]: WSAStartup error %i. Application aborted.\n", err);
            abort();
        }

        if (data.wVersion != version)
        {
            fprintf(stderr, "[libsocket]: responced WinSock version (high.low: %u.%u) doesn't match requested version (high.low: %u.%u). Application aborted.\n", HIBYTE(data.wVersion), LOBYTE(data.wVersion), HIBYTE(version), LOBYTE(version));

            WSACleanup();
            abort();
        }
    #endif

    inited = true;
}

static void LIBSOCKET_CLUPATTR libsocket_WSACleanup(void)
{
    if (!inited) return;

    #ifdef LIBSOCKET_OS_WINDOWS
        WSACleanup();
    #endif

    inited = false;
}

#ifdef _MSC_VER
    static void libsocket_MSVCinit(void)
    {
        libsocket_WSAInit();
        if (atexit(libsocket_WSACleanup))
        {
            fprintf(stderr, "[libsocket]: error binding library cleanup callback in \"atexit\" C function. Application aborted.\n");
            libsocket_WSACleanup();
            abort();
        }
    }

    #pragma section(".CRT$XCU", read)
    __declspec(allocate(".CRT$XCU")) void (*libsocket_MSVCinit__ptr)(void) = libsocket_MSVCinit;
#endif