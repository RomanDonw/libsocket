#include "libsocket.h"

#include <stdlib.h>

#ifdef OS_WINDOWS
    void __attribute__((constructor(101))) init()
    {
        const WORD version = MAKEWORD(2, 2);

        WSADATA data;
        if (WSAStartup(version, &data)) abort();

        if (data.wVersion != version) { WSACleanup(); abort(); }
    }

    void __attribute__((destructor)) cleanup()
    {
        WSACleanup();
    }
#endif