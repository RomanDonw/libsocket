#include "sockslist.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>

mtx_t sockslist_mutex;

static Socket **sockets = NULL;
static size_t sockets_count = 0;

bool sockslist_has(Socket *socket)
{
    mtx_lock(&sockslist_mutex);

    bool found = false;
    for (size_t i = 0; i < sockets_count; i++) if (sockets[i] == socket) { found = true; break; }

    mtx_unlock(&sockslist_mutex);
    return found;
}

SocketsListError sockslist_add(Socket *socket)
{
    if (sockslist_has(socket)) return SocketsListError_ItemAlreadyExist;
    mtx_lock(&sockslist_mutex);

    {
        Socket **new_sockets = (Socket **)libsocket_realloc(sockets, sizeof(Socket *) * (sockets_count + 1));
        if (!new_sockets) { mtx_unlock(&sockslist_mutex); return SocketsListError_MemoryAllocationFailed; }
        sockets = new_sockets;
    }

    sockets[sockets_count++] = socket;
    mtx_unlock(&sockslist_mutex);
    return SocketsListError_Success;
}

SocketsListError sockslist_remove(Socket *socket)
{
    mtx_lock(&sockslist_mutex);

    bool found = false;
    size_t pos;
    for (pos = 0; pos < sockets_count; pos++) if (sockets[pos] == socket) { found = true; break; }
    if (!found) { mtx_unlock(&sockslist_mutex); return SocketsListError_ItemNotExist; }

    /*if (pos != sockets_count - 1) */sockets[pos] = sockets[sockets_count - 1];
    sockets_count--;

    if (sockets_count > 0)
    {
        Socket **new_sockets = (Socket **)libsocket_realloc(sockets, sizeof(Socket *) * sockets_count);
        if (new_sockets) sockets = new_sockets;
    }
    else
    {
        libsocket_free(sockets);
        sockets = NULL;
    }

    mtx_unlock(&sockslist_mutex);
    return SocketsListError_Success;
}

void sockslist_removeall(void)
{
    mtx_lock(&sockslist_mutex);

    libsocket_free(sockets);
    sockets = NULL;
    sockets_count = 0;

    mtx_unlock(&sockslist_mutex);
}