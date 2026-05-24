/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"
#include "init.h"

#include <stdio.h>
#include <stdatomic.h>

#include "err.h"
#include "sockslist.h"

static atomic_bool inited = ATOMIC_VAR_INIT(false);
static atomic_flag initfuncsbusyflag = ATOMIC_FLAG_INIT;

bool socket_initialized(void) { return atomic_load(&inited); }

SocketError socket_startup(const SocketStartupOptions *options)
{
    if (atomic_flag_test_and_set(&initfuncsbusyflag)) return SocketError_OperationInProgress;
    if (atomic_load(&inited)) { atomic_flag_clear(&initfuncsbusyflag); return SocketError_AlreadyInitialized; }

    if (mutex_init(&sockslist_mutex) != MUTEXERROR_SUCCESS)
    { atomic_flag_clear(&initfuncsbusyflag); return SocketError_InitializationError; }

    #ifdef LIBSOCKET_OS_WINDOWS
        static const SocketStartupOptions defaultopts =
        {
            .winsock_version = MAKEWORD(LIBSOCKET_WINSOCK_DEFAULT_VERSION_LOW, LIBSOCKET_WINSOCK_DEFAULT_VERSION_HIGH)
        };

        if (!options) options = &defaultopts;

        WSADATA data;
        int err = WSAStartup(options->winsock_version, &data);
        if (err) { atomic_flag_clear(&initfuncsbusyflag); return translateerror(err); }

        if (data.wVersion != options->winsock_version)
        {
            WSACleanup();
            
            atomic_flag_clear(&initfuncsbusyflag);
            return SocketError_WSAVersionsNotMatch;
        }
    #endif

    atomic_store(&inited, true);
    atomic_flag_clear(&initfuncsbusyflag);
    return SocketError_Success;
}

SocketError socket_cleanup(void)
{
    if (atomic_flag_test_and_set(&initfuncsbusyflag)) return SocketError_OperationInProgress;
    if (!atomic_load(&inited)) { atomic_flag_clear(&initfuncsbusyflag); return SocketError_NotInitialized; }

    #ifdef LIBSOCKET_OS_WINDOWS
        if (WSACleanup())
        {
            atomic_flag_clear(&initfuncsbusyflag);
            return GETLASTTRANSLATEDSYSERR();
        }
    #endif
    
    sockslist_removeall();
    mutex_destroy(sockslist_mutex);

    atomic_store(&inited, false);
    atomic_flag_clear(&initfuncsbusyflag);
    return SocketError_Success;
}