/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "util.h"

#include <stdlib.h>

#include "err.h"
#include "types.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
#endif

const IPv4Address IPV4ADDR_ANY = IPV4ADDR_INIT(INADDR_ANY);
const IPv4Address IPV4ADDR_LOOPBACK = IPV4ADDR_INIT(INADDR_LOOPBACK);
const IPv4Address IPV4ADDR_BROADCAST = IPV4ADDR_INIT(INADDR_BROADCAST);

const IPv6Address IPV6ADDR_ANY = IN6ADDR_ANY_INIT;
const IPv6Address IPV6ADDR_LOOPBACK = IN6ADDR_LOOPBACK_INIT;

LibSocketAllocators allocs = {0};

SocketError __closesocket(Socket *socket)
{
    if (CLOSESOCKETDESC(socket->desc)) return GETLASTTRANSLATEDSYSERR();
    allocs.free(socket);
    return SocketError_Success;
}
