#include "libsocket.h"
#include "init.h"

#include <stdio.h>
#include <stdlib.h>

#include "err.h"

void *(*libsocket_malloc)(size_t) = malloc;
void *(*libsocket_realloc)(void *, size_t) = realloc;
void (*libsocket_free)(void *) = free;

bool inited = false;

bool socket_isinited(void) { return inited; }

bool socket_startup(void)
{
    if (inited) RETURNWITHERROR(AlreadyInitialized, false);

    #ifdef LIBSOCKET_OS_WINDOWS
        const WORD version = MAKEWORD(LIBSOCKET_WINSOCK_VERSION_LOW, LIBSOCKET_WINSOCK_VERSION_HIGH);

        WSADATA data;
        int err = WSAStartup(version, &data);
        if (err) RETURNWITHERROR(translateerror(err), false);
        //if (data.wVersion != version) { if (WSACleanup()) RETURNWITHSYSERR(false); RETURNWITHERROR(WSAVersionNotSupported, false); }
        if (data.wVersion != version) { WSACleanup(); RETURNWITHERROR(WSAVersionsNotMatch, false); }
    #endif

    inited = true;
    RETURNWITHSUCCESS(true);
}

bool socket_cleanup(void)
{
    if (!inited) RETURNWITHERROR(NotInitialized, false);

    #ifdef LIBSOCKET_OS_WINDOWS
        if (WSACleanup()) RETURNWITHSYSERR(false);
    #endif

    inited = false;
    RETURNWITHSUCCESS(true);
}