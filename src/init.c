/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"
#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

#include "err.h"
#include "sockslist.h"
#include "util.h"

static atomic_bool inited = ATOMIC_VAR_INIT(false);
static atomic_flag initfuncsbusyflag = ATOMIC_FLAG_INIT;

bool libsocket_initialized(void) { return atomic_load(&inited); }

SocketError libsocket_startup(const LibSocketStartupOptions *options, LibSocketStartupResults *results)
{
    if (atomic_flag_test_and_set(&initfuncsbusyflag)) return SocketError_OperationInProgress;
    if (atomic_load(&inited)) { atomic_flag_clear(&initfuncsbusyflag); return SocketError_AlreadyInitialized; }

    static const LibSocketStartupOptions defaultopts = LIBSOCKETSTARTUPOPTIONS_DEFAULTINIT;
    if (!options) options = &defaultopts;

    if (options->allocators) allocs = *options->allocators;
    else
    {
        allocs.malloc = malloc;
        allocs.realloc = realloc;
        allocs.free = free;
    }

    if (options->panichandler) __panichandler = options->panichandler;
    else __panichandler = __defaultpanichandler;

    if (options->alerthandler) __alerthandler = options->alerthandler;
    else __alerthandler = __defaultalerthandler;

    SocketError err;

    // =============================================================================

    if (mutex_create(&sockslist_mutex) != MUTEXERROR_SUCCESS)
    { err = SocketError_MutexAPIError; goto errorquit; }

    LibSocketStartupResults res = {0};

    #ifdef LIBSOCKET_OS_WINDOWS
        unsigned short winsock_version = options->winsock_version ? options->winsock_version : LIBSOCKET_DEFAULT_WINSOCK_VERSION;

        WSADATA wsadata;
        int wsaerr = WSAStartup(winsock_version, &wsadata);
        if (wsaerr) { err = translateerror(wsaerr); goto errorquit; }

        if (wsadata.wVersion != winsock_version)
        {
            if (WSACleanup()) panic_general(GETLASTTRANSLATEDSYSERR(), "WSACleanup error on cleanup while handling not matching WinSock versions.");

            err = SocketError_WSAVersionNotSupported;
            goto errorquit;
        }
    #endif

    // =============================================================================

    atomic_store(&inited, true);
    atomic_flag_clear(&initfuncsbusyflag);

    if (results)
    {
        #ifdef LIBSOCKET_OS_WINDOWS
            LibSocketStartupResults res = 
            {
                .used_winsock_version = wsadata.wVersion,
                .max_winsock_version = wsadata.wHighVersion,
                .max_sockets_count = wsadata.iMaxSockets,
                .max_datagram_size = wsadata.iMaxUdpDg
            };

            memcpy(results, &res, sizeof(res));
        #else
            memset(results, 0, sizeof(LibSocketStartupResults));
        #endif
    }

    return SocketError_Success;

    errorquit:
        __alerthandler = NULL;
        __panichandler = NULL;
        memset(&allocs, 0, sizeof(allocs));
        atomic_flag_clear(&initfuncsbusyflag);
    return err;
}

SocketError libsocket_cleanup(void)
{
    if (atomic_flag_test_and_set(&initfuncsbusyflag)) return SocketError_OperationInProgress;
    if (!atomic_load(&inited)) { atomic_flag_clear(&initfuncsbusyflag); return SocketError_NotInitialized; }

    // =============================================================================

    #ifdef LIBSOCKET_OS_WINDOWS
        if (WSACleanup())
        {
            atomic_flag_clear(&initfuncsbusyflag);
            return GETLASTTRANSLATEDSYSERR();
        }

        sockslist_removeall(false);
    #else
        sockslist_removeall(true);
    #endif
    
    if (mutex_destroy(sockslist_mutex) != MUTEXERROR_SUCCESS) panic_general(PANIC_NOERRORCODE, "Can't destroy mutex after library general cleanup.");

    // =============================================================================

    __alerthandler = NULL;
    __panichandler = NULL;
    memset(&allocs, 0, sizeof(allocs));

    atomic_store(&inited, false);
    atomic_flag_clear(&initfuncsbusyflag);
    return SocketError_Success;
}