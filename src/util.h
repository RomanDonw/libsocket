/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef UTIL_H
#define UTIL_H

#include "libsocket.h"

#include <limits.h>
#include <stddef.h>
#include <libnthread.h>

#ifdef LIBSOCKET_OS_WINDOWS
    #define CLAMPSIZET(x) ((size_t)x > INT_MAX ? (int)INT_MAX : (int)x)
    #define CLOSESOCKETDESC(descr) (closesocket(descr))

    #if defined(_MSC_VER) && (_MSC_VER) < 1300
        #define __func__ __FUNCTION__
    #endif
#else
    #define CLAMPSIZET(x) ((size_t)x)
    #define CLOSESOCKETDESC(descr) (close(descr))
#endif

extern NMemoryAllocators __libsocket_allocators;
#define allocs (__libsocket_allocators)

extern NUnorderedSet *__libsocket_sockslist;
#define sockslist (__libsocket_sockslist)

extern NThreadMutex *__libsocket_sockslistmutex;
#define sockslistmutex (__libsocket_sockslistmutex)

// =============================================================================

extern NPanicHandler *__libsocket_panichandler;
#define __panichandler (__libsocket_panichandler)

NPanicHandler __libsocket_defaultpanichandler;
#define __defaultpanichandler (__libsocket_defaultpanichandler)


#define PANIC_NOERRORCODE (NError_Success)

#define panic_general(errorcode, description) \
    {\
        __panichandler(LIBSOCKET_MODULENAME, __FILE__, __LINE__, __func__, (description), (errorcode));\
        abort();\
    }

// =============================================================================

extern NAlertHandler *__libsocket_alerthandler;
#define __alerthandler (__libsocket_alerthandler)

NAlertHandler __libsocket_defaultalerthandler;
#define __defaultalerthandler (__libsocket_defaultalerthandler)

#ifdef LIBSOCKET_DEBUG
    #define alert(format, ...) (__alerthandler(LIBSOCKET_MODULENAME, __FILE__, __LINE__, __func__, format, __VA_ARGS__))
#else
    #define alert(format, ...)
#endif

// =============================================================================

NError __libsocket_closesocket(Socket *socket);
#define __closesocket(...) (__libsocket_closesocket(__VA_ARGS__))

#define SAFE_MUTEX_LOCK(mutex) \
    { NError nerr = nthread_mutex_lock(mutex); if (nerr != NError_Success) panic_general(nerr, n_panicmsg_mutexlock); }

#define SAFE_MUTEX_UNLOCK(mutex) \
    { NError nerr = nthread_mutex_unlock(mutex); if (nerr != NError_Success) panic_general(nerr, n_panicmsg_mutexunlock); }

// =============================================================================

#endif
