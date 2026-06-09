/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef SOCKSLIST_H
#define SOCKSLIST_H

#include "libsocket.h"

#include <libmutex.h>
#include <stddef.h>

enum SocketsListError
{
    SocketsListError_Success = 0,
    SocketsListError_MemoryAllocationFailed = 1,
    SocketsListError_ItemAlreadyExist = 2,
    SocketsListError_ItemNotExist = 3
} typedef SocketsListError;

extern mutex_t *__libsocket_sockslist_mutex; // recursive mutex.
#define sockslist_mutex __libsocket_sockslist_mutex

bool __libsocket_sockslist_has(Socket *socket);
SocketsListError __libsocket_sockslist_add(Socket *socket);
SocketsListError __libsocket_sockslist_remove(Socket *socket);
void __libsocket_sockslist_removeall(bool closesocks);

#define sockslist_has(...) (__libsocket_sockslist_has(__VA_ARGS__))
#define sockslist_add(...) (__libsocket_sockslist_add(__VA_ARGS__))
#define sockslist_remove(...) (__libsocket_sockslist_remove(__VA_ARGS__))
#define sockslist_removeall(...) (__libsocket_sockslist_removeall(__VA_ARGS__))

#endif