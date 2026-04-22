#include "libsocket.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "init.h"
#include "err.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

const IPv4Address IPV4ADDR_ANY = IPV4ADDR_INIT(INADDR_ANY);
const IPv4Address IPV4ADDR_LOOPBACK = IPV4ADDR_INIT(INADDR_LOOPBACK);
const IPv4Address IPV4ADDR_BROADCAST = IPV4ADDR_INIT(INADDR_BROADCAST);

const IPv6Address IPV6ADDR_ANY = IN6ADDR_ANY_INIT;
const IPv6Address IPV6ADDR_LOOPBACK = IN6ADDR_LOOPBACK_INIT;

struct Socket
{
    SOCKETDESCRIPTOR desc;

    SocketAddressFamily af;
    SocketType type;
    SocketProtocol protocol;
};

Socket *socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol)
{
    ENSURE_INIT;

    SOCKETDESCRIPTOR desc;
    if ((desc = socket(af, type, protocol)) == INVALID_SOCKET) return NULL;

    Socket *ret = malloc(sizeof(Socket));
    if (!ret)
    {
        SETLASTERROR(SOCKERR_NOMEM);
        return NULL;
    }

    ret->af = af;
    ret->type = type;
    ret->protocol = protocol;
    ret->desc = desc;

    return ret;
}

bool socket_close(Socket *socket)
{
    ENSURE_INIT;

    #ifdef LIBSOCKET_OS_WINDOWS
        if (closesocket(socket->desc)) return false;
    #else
        if (close(socket->desc)) return false;
    #endif

    free(socket);

    return true;
}

bool socket_listen(const Socket *socket, int backlog) { ENSURE_INIT; return !listen(socket->desc, backlog); }

bool socket_connect(const Socket *socket, const SocketAddressInterface *sockaddr)
{ ENSURE_INIT; return !connect(socket->desc, (struct sockaddr *)sockaddr, sizeof(SocketAddressInterface)); }

bool socket_bind(const Socket *socket, const SocketAddressInterface *sockaddr)
{ ENSURE_INIT; return !bind(socket->desc, (struct sockaddr *)sockaddr, sizeof(SocketAddressInterface)); }

Socket *socket_accept(const Socket *socket)
{
    ENSURE_INIT;

    SOCKETDESCRIPTOR desc;
    if ((desc = accept(socket->desc, NULL, NULL)) == INVALID_SOCKET) return NULL;

    Socket *ret = malloc(sizeof(Socket));
    if (!ret)
    {
        SETLASTERROR(SOCKERR_NOMEM);
        return NULL;
    }

    ret->af = socket->af;
    ret->type = socket->type;
    ret->protocol = socket->protocol;
    ret->desc = desc;

    return ret;
}

ssize_t socket_recv(const Socket *socket, void *buffer, socksize_t len, int flags) { ENSURE_INIT; return recv(socket->desc, buffer, len, flags); }
ssize_t socket_send(const Socket *socket, const void *data, socksize_t len) { ENSURE_INIT; return send(socket->desc, data, len, 0); }

bool socket_ioctl(const Socket *socket, SocketIOCTLOption option, void *value)
{
    ENSURE_INIT;

    #ifdef LIBSOCKET_OS_WINDOWS
        return !ioctlsocket(socket->desc, option, value);
    #else
        return !ioctl(socket->desc, option, value);
    #endif
}

bool socket_shutdown(const Socket *socket, SocketShutdownMode mode) { ENSURE_INIT; return !shutdown(socket->desc, mode); }

SocketAddressFamily socket_getaf(const Socket *socket) { ENSURE_INIT; return socket->af; }
SocketType socket_gettype(const Socket *socket) { ENSURE_INIT; return socket->type; }
SocketProtocol socket_getprotocol(const Socket *socket) { ENSURE_INIT; return socket->protocol; }

bool socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, socklen_t *optlen)
{ ENSURE_INIT; return !getsockopt(socket->desc, level, optname, optval, optlen); }

bool socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, socklen_t optlen)
{ ENSURE_INIT; return !setsockopt(socket->desc, level, optname, optval, optlen); }

bool socket_parseaddr(IPAddressInterface *addr, SocketAddressFamily af, const char *straddr)
{
    ENSURE_INIT;

    int ret = inet_pton(af, straddr, addr);
    if (ret == 0) SETLASTERROR(SOCKERR_PARSEADDRFAIL);
    return ret == 1;
}

bool socket_addrtostr(const IPAddressInterface *addr, SocketAddressFamily af, char *straddr, socklen_t size)
{
    ENSURE_INIT;

    bool res = inet_ntop(af, addr, straddr, size);
    #ifdef LIBSOCKET_OS_WINDOWS
        if (!res && GETLASTERROR() == SOCKERR_INVAL) SETLASTERROR(SOCKERR_NOSPC);
    #endif
    return res;
}

LIBSOCKET_API bool LIBSOCKET_ABI socket_packsockaddr(SocketAddressInterface *sockaddr, SocketAddressFamily af, const IPAddressInterface *addr, unsigned short port)
{
    ENSURE_INIT;

    memset(sockaddr, 0, sizeof(SocketAddressInterface));
    sockaddr->ss_family = af;

    switch (af)
    {
        case IPv4:;
            SocketIPv4Address *sa4 = (SocketIPv4Address *)sockaddr;
            sa4->sin_addr = *((IPv4Address *)addr);
            sa4->sin_port = SOCKET_HTONS(port);
            break;

        case IPv6:;
            SocketIPv6Address *sa6 = (SocketIPv6Address *)sockaddr;
            sa6->sin6_addr = *((IPv6Address *)addr);
            sa6->sin6_port = SOCKET_HTONS(port);
            break;

        default:
            SETLASTERROR(SOCKERR_AFNOSUPPORT);
            return false;
    }

    return true;
}

LIBSOCKET_API bool LIBSOCKET_ABI socket_unpacksockaddr(const SocketAddressInterface *sockaddr, SocketAddressFamily af, IPAddressInterface *addr, unsigned short *port)
{
    if (sockaddr->ss_family != af) { SETLASTERROR(SOCKERR_INVAL); return false; }

    switch (af)
    {
        case IPv4:;
            SocketIPv4Address *sa4 = (SocketIPv4Address *)sockaddr;
            *((IPv4Address *)addr) = sa4->sin_addr;
            *port = SOCKET_NTOHS(sa4->sin_port);
            break;

        case IPv6:;
            SocketIPv6Address *sa6 = (SocketIPv6Address *)sockaddr;
            *((IPv6Address *)addr) = sa6->sin6_addr;
            *port = SOCKET_NTOHS(sa6->sin6_port);
            break;

        default:
            SETLASTERROR(SOCKERR_AFNOSUPPORT);
            return false;
    }

    return true;
}

bool socket_getremoteaddr(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{ return !getpeername(socket->desc, (struct sockaddr *)sockaddr, size); }

bool socket_getlocaladdr(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{ return !getsockname(socket->desc, (struct sockaddr *)sockaddr, size); }

SOCKETDESCRIPTOR socket_gethandle(const Socket *socket) { ENSURE_INIT; return socket->desc; }