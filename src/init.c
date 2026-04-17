#include "libsocket.h"
#include "init.h"

volatile void *___ = NULL;

#ifdef OS_WINDOWS
    #include <stdbool.h>

    bool inited = false;

    void __attribute__((constructor(101))) init()
    {
        const WORD version = MAKEWORD(2, 2);

        WSADATA data;
        if (WSAStartup(version, &data)) abort();

        if (data.wVersion != version) { WSACleanup(); abort(); }

        inited = true;
    }

    void __attribute__((destructor)) cleanup()
    {
        WSACleanup();
        inited = false;
    }
#endif