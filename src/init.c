/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"
#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libnthread.h>

#include "err.h"
#include "util.h"

static NThreadAtomicBool inited = NTHREAD_ATOMICBOOLINIT(false);
static NThreadAtomicBool funcslock = NTHREAD_ATOMICBOOLINIT(false);

bool libsocket_initialized(void) { return nthread_atomicbool_load(&inited); }

NError libsocket_startup(const LibSocketStartupOptions *options, LibSocketStartupResults *results)
{
    if (nthread_atomicbool_cmpxchgv(&funcslock, false, true)) return NError_OperationInProgress;
    if (nthread_atomicbool_load(&inited)) { nthread_atomicbool_store(&funcslock, false); return NError_AlreadyInitialized; }
    if (!libnthread_initialized()) { nthread_atomicbool_store(&funcslock, false); return NError_DependencyNotInitialized; }

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

    // =============================================================================

    NError nerr = n_unorderedset_create(&sockslist, allocs, sizeof(Socket *));
    if (nerr != NError_Success) goto errorquit_generic;

    if ((nerr = nthread_mutex_create(&sockslistmutex)) != NError_Success) goto errorquit_aftercreatesockslist;

    LibSocketStartupResults res = {0};

    #ifdef LIBSOCKET_OS_WINDOWS
        unsigned short winsock_version = options->winsock_version ? options->winsock_version : LIBSOCKET_DEFAULT_WINSOCK_VERSION;

        WSADATA wsadata;
        int wsaerr = WSAStartup(winsock_version, &wsadata);
        if (wsaerr) { nerr = translateerror(wsaerr); goto errorquit_aftersockslistmtxcreate; }

        if (wsadata.wVersion != winsock_version)
        {
            if (WSACleanup()) panic_general(GETLASTTRANSLATEDSYSERR(), "WSACleanup error on cleanup while handling not matching WinSock versions.");

            nerr = NError_WSAVersionNotSupported;
            goto errorquit_aftersockslistmtxcreate;
        }
    #endif

    // =============================================================================

    nthread_atomicbool_store(&inited, true);
    nthread_atomicbool_store(&funcslock, false);

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

    return NError_Success;

    errorquit_aftersockslistmtxcreate:
        if ((nerr = nthread_mutex_destroy(sockslistmutex)) != NError_Success) panic_general(nerr, "Unable to destroy mutex of sockets list during handling error.");
        sockslistmutex = NULL;
    errorquit_aftercreatesockslist:
        n_unorderedset_destroy(sockslist);
        sockslist = NULL;
    errorquit_generic:
        __alerthandler = NULL;
        __panichandler = NULL;
        memset(&allocs, 0, sizeof(allocs));
    nthread_atomicbool_store(&funcslock, false);
    return nerr;
}

NError libsocket_cleanup(void)
{
    if (nthread_atomicbool_cmpxchgv(&funcslock, false, true)) return NError_OperationInProgress;
    if (!nthread_atomicbool_load(&inited)) { nthread_atomicbool_store(&funcslock, false); return NError_NotInitialized; }

    // =============================================================================

    NError nerr;

    #ifdef LIBSOCKET_OS_WINDOWS
        if (WSACleanup())
        {
            nthread_atomicbool_store(&funcslock, false);
            return GETLASTTRANSLATEDSYSERR();
        }
    #else
        Socket *socket;
        for (size_t i = 0; i < n_unorderedset_getlength(sockslist); i++)
        {
            if (((nerr = n_unorderedset_getelement(sockslist, i, &socket)) != NError_Success) || ((nerr = __closesocket(socket)) != NError_Success))
            {
                if (!i) { nthread_atomicbool_store(&funcslock, false); return nerr; }
                panic_general(nerr, "Unable to close socket during library cleanup.");
            }
        }
    #endif

    n_unorderedset_destroy(sockslist);
    sockslist = NULL;

    if ((nerr = nthread_mutex_destroy(sockslistmutex)) != NError_Success) panic_general(nerr, n_panicmsg_mutexdestroyduringlibrarycleanup);
    sockslistmutex = NULL;

    // =============================================================================

    __alerthandler = NULL;
    __panichandler = NULL;
    memset(&allocs, 0, sizeof(allocs));

    nthread_atomicbool_store(&inited, false);
    nthread_atomicbool_store(&funcslock, false);
    return NError_Success;
}