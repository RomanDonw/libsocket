#include "libsocket.h"

#include <stdlib.h>
#include <string.h>

#include "init.h"
#include "err.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

SocketAddressFamily socket_getsockaddraf(const SocketAddressInterface *sockaddr) { return sockaddr->ss_family; }

bool socket_packsockaddr(SocketAddressInterface *sockaddr, SocketAddressFamily af, const IPAddressInterface *addr, unsigned short port)
{
    ENSURE_INIT;

    memset(sockaddr, 0, sizeof(SocketAddressInterface));
    sockaddr->ss_family = af;

    switch (af)
    {
        case IPv4:;
            SocketIPv4Address *sa4 = (void *)sockaddr;
            sa4->sin_addr = *((IPv4Address *)addr);
            sa4->sin_port = SOCKET_HTONS(port);
            break;

        case IPv6:;
            SocketIPv6Address *sa6 = (void *)sockaddr;
            sa6->sin6_addr = *((IPv6Address *)addr);
            sa6->sin6_port = SOCKET_HTONS(port);
            break;

        default:
            SETLASTERROR(SOCKERR_AFNOSUPPORT);
            return false;
    }

    return true;
}

bool socket_unpacksockaddr(const SocketAddressInterface *sockaddr, SocketAddressFamily af, IPAddressInterface *addr, unsigned short *port)
{
    ENSURE_INIT;

    if (sockaddr->ss_family != af) { SETLASTERROR(SOCKERR_INVAL); return false; }

    switch (af)
    {
        case IPv4:;
            const SocketIPv4Address *sa4 = (const void *)sockaddr;
            *((IPv4Address *)addr) = sa4->sin_addr;
            *port = SOCKET_NTOHS(sa4->sin_port);
            break;

        case IPv6:;
            const SocketIPv6Address *sa6 = (const void *)sockaddr;
            *((IPv6Address *)addr) = sa6->sin6_addr;
            *port = SOCKET_NTOHS(sa6->sin6_port);
            break;

        default:
            SETLASTERROR(SOCKERR_AFNOSUPPORT);
            return false;
    }

    return true;
}