/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"

#include <stdlib.h>
#include <string.h>

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

#include "err.h"

SocketError socket_packsockipaddr(SocketIPAddressInterface *sockaddr, SocketAddressFamily af, const IPAddressInterface *addr, unsigned short port)
{
    switch (af)
    {
        case SocketAddressFamily_IPv4:;
            SocketIPv4Address *sa4 = sockaddr;
            memset(sa4, 0, sizeof(SocketIPv4Address));

            sa4->sin_family = SocketAddressFamily_IPv4;
            sa4->sin_addr = *((IPv4Address *)addr);
            sa4->sin_port = SOCKET_HTONS(port);

            break;

        case SocketAddressFamily_IPv6:;
            SocketIPv6Address *sa6 = sockaddr;
            memset(sa6, 0, sizeof(SocketIPv6Address));

            sa6->sin6_family = SocketAddressFamily_IPv6;
            sa6->sin6_addr = *((IPv6Address *)addr);
            sa6->sin6_port = SOCKET_HTONS(port);

            break;

        default:
            return SocketError_UnsupportedAddressFamily;
    }

    return SocketError_Success;
}

SocketError socket_unpacksockipaddr(const SocketIPAddressInterface *sockaddr, SocketAddressFamily af, IPAddressInterface *addr, unsigned short *port)
{
    if (SOCKET_GETSOCKADDRAF(sockaddr) != af) return SocketError_IncorrectArgumentValue;

    switch (af)
    {
        case SocketAddressFamily_IPv4:;
            const SocketIPv4Address *sa4 = sockaddr;
            *((IPv4Address *)addr) = sa4->sin_addr;
            *port = SOCKET_NTOHS(sa4->sin_port);
            break;

        case SocketAddressFamily_IPv6:;
            const SocketIPv6Address *sa6 = sockaddr;
            *((IPv6Address *)addr) = sa6->sin6_addr;
            *port = SOCKET_NTOHS(sa6->sin6_port);
            break;

        default:
            return SocketError_UnsupportedAddressFamily;
    }

    return SocketError_Success;
}