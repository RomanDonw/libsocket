#include "libsocket.h"

#include <stdlib.h>
#include <string.h>

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

#include "err.h"

#define GETSOCKADDRAF(sockaddr_ptr) (((const struct sockaddr *)sockaddr_ptr)->sa_family)
SocketAddressFamily socket_getsockaddraf(const SocketAddressInterface *sockaddr) { return GETSOCKADDRAF(sockaddr); }

bool socket_packsockaddr(SocketAddressInterface *sockaddr, SocketAddressFamily af, const IPAddressInterface *addr, unsigned short port)
{
    //ENSURE_INIT;

    switch (af)
    {
        case IPv4:;
            SocketIPv4Address *sa4 = sockaddr;
            memset(sa4, 0, sizeof(SocketIPv4Address));

            sa4->sin_family = IPv4;
            sa4->sin_addr = *((IPv4Address *)addr);
            sa4->sin_port = SOCKET_HTONS(port);

            break;

        case IPv6:;
            SocketIPv6Address *sa6 = sockaddr;
            memset(sa6, 0, sizeof(SocketIPv6Address));

            sa6->sin6_family = IPv6;
            sa6->sin6_addr = *((IPv6Address *)addr);
            sa6->sin6_port = SOCKET_HTONS(port);

            break;

        default:
            //SETLASTERROR(SOCKERR_AFNOSUPPORT);
            RETURNWITHERROR(UnsupportedAddressFamily, false);
    }

    RETURNWITHSUCCESS(true);
}

bool socket_unpacksockaddr(const SocketAddressInterface *sockaddr, SocketAddressFamily af, IPAddressInterface *addr, unsigned short *port)
{
    //ENSURE_INIT;

    if (GETSOCKADDRAF(sockaddr) != af) RETURNWITHERROR(IncorrectArgumentValue, false);

    switch (af)
    {
        case IPv4:;
            const SocketIPv4Address *sa4 = sockaddr;
            *((IPv4Address *)addr) = sa4->sin_addr;
            *port = SOCKET_NTOHS(sa4->sin_port);
            break;

        case IPv6:;
            const SocketIPv6Address *sa6 = sockaddr;
            *((IPv6Address *)addr) = sa6->sin6_addr;
            *port = SOCKET_NTOHS(sa6->sin6_port);
            break;

        default:
            //SETLASTERROR(SOCKERR_AFNOSUPPORT);
            RETURNWITHERROR(UnsupportedAddressFamily, false);
    }

    RETURNWITHSUCCESS(true);
}