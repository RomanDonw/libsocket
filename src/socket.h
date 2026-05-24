#ifndef SOCKET_H
#define SOCKET_H

#include "libsocket.h"

struct Socket
{
    SOCKETDESCRIPTOR desc;

    SocketAddressFamily af;
    SocketType type;
    SocketProtocol protocol;
};

#endif