/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "sockslist.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "util.h"

mutex_t *__libsocket_sockslist_mutex;

static Socket **sockets = NULL;
static size_t sockets_count = 0;

bool __libsocket_sockslist_has(Socket *socket)
{
    mutex_lock_ne(sockslist_mutex);

    bool found = false;
    for (size_t i = 0; i < sockets_count; i++) if (sockets[i] == socket) { found = true; break; }

    mutex_unlock_ne(sockslist_mutex);
    return found;
}

SocketsListError __libsocket_sockslist_add(Socket *socket)
{
    mutex_lock_ne(sockslist_mutex);

    if (sockslist_has(socket))
    {
        mutex_unlock_ne(sockslist_mutex);
        return SocketsListError_ItemAlreadyExist;
    }

    {
        Socket **new_sockets = (Socket **)allocs.realloc(sockets, sizeof(Socket *) * (sockets_count + 1));
        if (!new_sockets) { mutex_unlock_ne(sockslist_mutex); return SocketsListError_MemoryAllocationFailed; }
        sockets = new_sockets;
    }

    sockets[sockets_count++] = socket;
    mutex_unlock_ne(sockslist_mutex);
    return SocketsListError_Success;
}

SocketsListError __libsocket_sockslist_remove(Socket *socket)
{
    mutex_lock_ne(sockslist_mutex);

    bool found = false;
    size_t pos;
    for (pos = 0; pos < sockets_count; pos++) if (sockets[pos] == socket) { found = true; break; }
    if (!found) { mutex_unlock_ne(sockslist_mutex); return SocketsListError_ItemNotExist; }

    /*if (pos != sockets_count - 1) */sockets[pos] = sockets[sockets_count - 1];
    sockets_count--;

    if (sockets_count > 0)
    {
        Socket **new_sockets = (Socket **)allocs.realloc(sockets, sizeof(Socket *) * sockets_count);
        if (new_sockets) sockets = new_sockets;
    }
    else
    {
        allocs.free(sockets);
        sockets = NULL;
    }

    mutex_unlock_ne(sockslist_mutex);
    return SocketsListError_Success;
}

void __libsocket_sockslist_removeall(bool closesocks)
{
    mutex_lock_ne(sockslist_mutex);

    if (closesocks) for (size_t i = 0; i < sockets_count; i++) __closesocket(sockets[i]);

    allocs.free(sockets);
    sockets = NULL;
    sockets_count = 0;

    mutex_unlock_ne(sockslist_mutex);
}