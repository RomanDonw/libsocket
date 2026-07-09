/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef SOCKET_H
#define SOCKET_H

#include "libsocket.h"

#include <libnthread.h>

#ifdef LIBSOCKET_OS_WINDOWS
    typedef SSIZE_T ssize_t;
#endif

struct Socket
{
    SOCKETDESCRIPTOR desc;

    SocketAddressFamily af;
    SocketType type;
    SocketProtocol protocol;
    
    bool nonblocking;
    NThreadMutex *mutex_nonblocking;
};

#endif
