#ifndef RESOURCES_H
#define RESOURCES_H

#include "libsocket.h"

#include <stddef.h>
#include <threads.h>

enum SocketsListError
{
    SocketsListError_Success = 0,
    SocketsListError_MemoryAllocationFailed = 1,
    SocketsListError_ItemAlreadyExist = 2,
    SocketsListError_ItemNotExist = 3
} typedef SocketsListError;

extern mtx_t sockslist_mutex; // recursive mutex.

bool sockslist_has(Socket *socket);
SocketsListError sockslist_add(Socket *socket);
SocketsListError sockslist_remove(Socket *socket);
void sockslist_removeall(void);

#endif