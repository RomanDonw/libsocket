/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef UTIL_H
#define UTIL_H

#include "libsocket.h"

#include <limits.h>

#ifdef LIBSOCKET_OS_WINDOWS
    #define CLAMPSIZET(x) ((size_t)x > INT_MAX ? (int)INT_MAX : (int)x)
    #define CLOSESOCKETDESC(descr) (closesocket(descr))
#else
    #define CLAMPSIZET(x) ((size_t)x)
    #define CLOSESOCKETDESC(descr) (close(descr))
#endif

extern LibSocketAllocators allocs;

#ifdef LIBSOCKET_DEBUG
    void __libsocket_logdbgerr(const char *msgformat, ...);
    #define LOGDBGERR(msgformat, ...) (__libsocket_logdbgerr(msgformat, __VA_ARGS__))
#else
    #define LOGDBGERR(msgformat, ...)
#endif

SocketError __libsocket_closesocket(Socket *socket);
#define __closesocket(...) (__libsocket_closesocket(__VA_ARGS__))

//SocketError __setsockdefaultopts(const Socket *socket);

#endif
