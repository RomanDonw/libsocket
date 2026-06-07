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
#else
    #define CLAMPSIZET(x) ((size_t)x)
#endif

#ifdef LIBSOCKET_OS_WINDOWS
    #define CLOSESOCKETDESC(descr) (closesocket(descr))
#else
    #define CLOSESOCKETDESC(descr) (close(descr))
#endif

extern LibSocketAllocators allocs;

SocketError __closesocket(Socket *socket);

//SocketError __setsockdefaultopts(const Socket *socket);

#endif
