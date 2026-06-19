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
#include <libmutex.h>

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

extern LibSocketAllocators __libsocket_allocators;
#define allocs __libsocket_allocators

// =============================================================================

extern LibSocketPanicHandler *__libsocket_panichandler;
#define __panichandler __libsocket_panichandler

LibSocketPanicHandler __libsocket_defaultpanichandler;
#define __defaultpanichandler __libsocket_defaultpanichandler

#define PANIC_NOERRORCODE (SocketError_Success)

#define __PANICINFOBASE(errcode, _description) \
    .file = __FILE__,\
    .line = __LINE__,\
    .function = __func__,\
    .description = (_description),\
    .error = (errcode)

#define __PANICBASE(panicinfo_initializer) \
    {\
        const LibSocketPanicInfo info = { panicinfo_initializer };\
        __panichandler(&info);\
        abort();\
    }

#define panic_general(errcode, description) __PANICBASE(__PANICINFOBASE(errcode, description))

// =============================================================================

SocketError __libsocket_closesocket(Socket *socket);
#define __closesocket(...) (__libsocket_closesocket(__VA_ARGS__))

#define SAFE_MUTEX_LOCK(mutex) \
    { if (mutex_lock(mutex) != MUTEXERROR_SUCCESS) panic_general(SocketError_MutexAPIError, "mutex_lock error in critical library section."); }

#define SAFE_MUTEX_UNLOCK(mutex) \
    { if (mutex_unlock(mutex) != MUTEXERROR_SUCCESS) panic_general(SocketError_MutexAPIError, "mutex_unlock error in critical library section."); }

// =============================================================================

#ifdef LIBSOCKET_DEBUG
    void __libsocket_logdbgerr(const char *msgformat, ...);
    #define LOGDBGERR(msgformat, ...) (__libsocket_logdbgerr(msgformat, __VA_ARGS__))
#else
    #define LOGDBGERR(msgformat, ...)
#endif

#endif
