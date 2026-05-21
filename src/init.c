#include "libsocket.h"
#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#include "err.h"
#include "sockslist.h"

void *(*libsocket_malloc)(size_t) = malloc;
void *(*libsocket_realloc)(void *, size_t) = realloc;
void (*libsocket_free)(void *) = free;

static atomic_bool inited = ATOMIC_VAR_INIT(false);

bool socket_initialized(void) { return atomic_load(&inited); }

SocketError socket_startup(const SocketStartupOptions *options)
{
    if (atomic_load(&inited)) return SocketError_AlreadyInitialized;

    void *(*old_libmutex_malloc)(size_t) = libmutex_malloc;
    void (*old_libmutex_free)(void *) = libmutex_free;
    
    libmutex_malloc = libsocket_malloc;
    libmutex_free = libsocket_free;

    if (mutex_init(&sockslist_mutex) != MUTEXERROR_SUCCESS) return SocketError_InitializationError;

    #ifdef LIBSOCKET_OS_WINDOWS
        static const SocketStartupOptions defaultopts =
        {
            .winsock_version = MAKEWORD(LIBSOCKET_WINSOCK_DEFAULT_VERSION_LOW, LIBSOCKET_WINSOCK_DEFAULT_VERSION_HIGH)
        };

        if (!options) options = &defaultopts;

        WSADATA data;
        int err = WSAStartup(options->winsock_version, &data);
        if (err) return translateerror(err);
        //if (data.wVersion != version) { if (WSACleanup()) RETURNWITHSYSERR(false); RETURNWITHERROR(SocketError_WSAVersionNotSupported, false); }
        if (data.wVersion != options->winsock_version) { WSACleanup(); return SocketError_WSAVersionsNotMatch; }
    #endif

    libmutex_malloc = old_libmutex_malloc;
    libmutex_free = old_libmutex_free;

    atomic_store(&inited, true);
    return SocketError_Success;
}

SocketError socket_cleanup(void)
{
    if (!atomic_load(&inited)) return SocketError_NotInitialized;

    #ifdef LIBSOCKET_OS_WINDOWS
        if (WSACleanup()) return GETLASTTRANSLATEDSYSERR();
    #endif
    
    sockslist_removeall();
    
    mutex_destroy(sockslist_mutex);

    atomic_store(&inited, false);
    return SocketError_Success;
}