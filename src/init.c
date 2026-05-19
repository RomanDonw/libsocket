#include "libsocket.h"
#include "init.h"

#include <stdio.h>
#include <stdlib.h>

#include "err.h"
#include "sockslist.h"

void *(*libsocket_malloc)(size_t) = malloc;
void *(*libsocket_realloc)(void *, size_t) = realloc;
void (*libsocket_free)(void *) = free;

bool inited = false;

bool socket_initialized(void) { return inited; }

bool socket_startup(const SocketStartupOptions *options)
{
    if (inited) RETURNWITHERROR(SocketError_AlreadyInitialized, false);

    #ifdef LIBSOCKET_OS_WINDOWS
        static const SocketStartupOptions defaultopts =
        {
            .winsock_version = MAKEWORD(LIBSOCKET_WINSOCK_DEFAULT_VERSION_LOW, LIBSOCKET_WINSOCK_DEFAULT_VERSION_HIGH)
        };

        if (!options) options = &defaultopts;

        WSADATA data;
        int err = WSAStartup(options->winsock_version, &data);
        if (err) RETURNWITHERROR(translateerror(err), false);
        //if (data.wVersion != version) { if (WSACleanup()) RETURNWITHSYSERR(false); RETURNWITHERROR(SocketError_WSAVersionNotSupported, false); }
        if (data.wVersion != options->winsock_version) { WSACleanup(); RETURNWITHERROR(SocketError_WSAVersionsNotMatch, false); }
    #endif

    inited = true;
    RETURNWITHSUCCESS(true);
}

bool socket_cleanup(void)
{
    if (!inited) RETURNWITHERROR(SocketError_NotInitialized, false);

    #ifdef LIBSOCKET_OS_WINDOWS
        if (WSACleanup()) RETURNWITHSYSERR(false);
    #endif
    
    sockslist_removeall();

    inited = false;
    RETURNWITHSUCCESS(true);
}