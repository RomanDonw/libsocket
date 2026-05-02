#include "libsocket.h"
#include "init.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
    #define LIBSOCKET_INITATTR
    #define LIBSOCKET_CLUPATTR
#else
    #define LIBSOCKET_INITATTR __attribute__((constructor(101)))
    #define LIBSOCKET_CLUPATTR __attribute__((destructor))
#endif

volatile void *___ = NULL;

void *(*libsocket_malloc)(size_t) = malloc;
void *(*libsocket_realloc)(void *, size_t) = realloc;
void (*libsocket_free)(void *) = free;

bool inited = false;

static void LIBSOCKET_INITATTR libsocket_init(void)
{
    if (inited) return;

    #ifdef LIBSOCKET_OS_WINDOWS
        const WORD version = MAKEWORD(LIBSOCKET_WINSOCK_VERSION_LOW, LIBSOCKET_WINSOCK_VERSION_HIGH);

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

static void LIBSOCKET_CLUPATTR libsocket_cleanup(void)
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
        libsocket_init();
        if (atexit(libsocket_cleanup))
        {
            fprintf(stderr, "[libsocket]: error binding library cleanup callback in \"atexit\" C function. Application aborted.\n");
            libsocket_cleanup();
            abort();
        }
    }

    #pragma section(".CRT$XCU", read)
    __declspec(allocate(".CRT$XCU")) void (*libsocket_MSVCinit__ptr)(void) = libsocket_MSVCinit;
#endif