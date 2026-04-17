#include "libsocket.h"
#include "init.h"

volatile void *___ = NULL;

#ifdef OS_WINDOWS
    #include <stdbool.h>
    #include <stdio.h>

    bool inited = false;

    void __attribute__((constructor(101))) init()
    {
        const WORD version = MAKEWORD(2, 2);

        WSADATA data;
        if (int err = WSAStartup(version, &data))
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

        inited = true;
    }

    void __attribute__((destructor)) cleanup()
    {
        WSACleanup();
        inited = false;
    }
#endif