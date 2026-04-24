#include "libsocket.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

    SOCKETDESCRIPTOR desc = socket(af, type, protocol);
    if (desc == INVALID_SOCKET) return NULL;

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

    SOCKETDESCRIPTOR desc = accept(socket->desc, NULL, NULL);
    if (desc == INVALID_SOCKET) return NULL;

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

ssize_t socket_recvfrom(const Socket *socket, void *buffer, socksize_t len, int flags, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{ ENSURE_INIT; return recvfrom(socket->desc, buffer, len, flags, (struct sockaddr *)sockaddr, sockaddrlen); }

ssize_t socket_sendto(const Socket *socket, const void *buffer, socksize_t len, int flags, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{ ENSURE_INIT; return sendto(socket->desc, buffer, len, flags, (const struct sockaddr *)sockaddr, sockaddrlen); }

ssize_t socket_send(const Socket *socket, const void *data, socksize_t len, int flags) { ENSURE_INIT; return send(socket->desc, data, len, flags); }

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
{
    ENSURE_INIT;

    switch (optname)
    {
        case Socket_Linger:;
            // this can do getsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) { SETLASTERROR(SOCKERR_FAULT); return false; }

            struct linger ling;
            socklen_t lingsz = sizeof(ling);
            if (getsockopt(socket->desc, SocketLevel, Socket_Linger, (void *)&ling, &lingsz)) return false;
            if (lingsz > sizeof(ling)) { SETLASTERROR(SOCKERR_INTERNALERR); return false; }

            SocketLingerOptions lingopts;
            lingopts.enable = ling.l_onoff;
            lingopts.linger = (ling.l_linger > USHRT_MAX) ? USHRT_MAX : ling.l_linger;

            if (*optlen > 0) memcpy(optval, &lingopts, (*optlen > sizeof(lingopts)) ? sizeof(lingopts) : *optlen);
            *optlen = sizeof(lingopts);
            return true;

        case Socket_RecvTimeout:;
        case Socket_SendTimeout:;
            // this can do getsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) { SETLASTERROR(SOCKERR_FAULT); return false; }

            uint32_t millis;
            #ifdef LIBSOCKET_OS_WINDOWS
                socklen_t millissz = sizeof(millis);
                if (getsockopt(socket->desc, level, optname, (void *)&millis, &millissz)) return false;
                if (millissz > sizeof(millis)) { SETLASTERROR(SOCKERR_INTERNALERR); return false; }
            #else
                struct timeval tv;
                socklen_t tvsz = sizeof(tv);
                if (getsockopt(socket->desc, level, optname, (void *)&tv, &tvsz)) return false;
                if (tvsz > sizeof(tv)) { SETLASTERROR(SOCKERR_INTERNALERR); return false; }

                uint64_t usecs;
                
                // check seconds on possible overflow when it will be converted to microseconds & set usecs variable.
                if (tv.tv_sec > UINT64_MAX / 1000000) usecs = UINT64_MAX;
                else usecs = tv.tv_sec * 1000000;

                // check microseconds on possible overflow before adding & clamp usecs value on overflow.
                if (UINT64_MAX - usecs < tv.tv_usec) usecs = UINT64_MAX;
                else usecs += tv.tv_usec;

                usecs /= 1000;
                if (usecs > UINT32_MAX) usecs = UINT32_MAX;
                millis = (uint32_t)usecs;
            #endif

            if (*optlen > 0) memcpy(optval, &millis, (*optlen > sizeof(millis)) ? sizeof(millis) : *optlen);
            *optlen = sizeof(millis);
            return true;

        default:
            return !getsockopt(socket->desc, level, optname, optval, optlen);
    }
}

bool socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, socklen_t optlen)
{
    ENSURE_INIT;

    switch (optname)
    {
        case Socket_Linger:;
            // this can do setsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) { SETLASTERROR(SOCKERR_FAULT); return false; }
            if (optlen < sizeof(SocketLingerOptions)) { SETLASTERROR(SOCKERR_INVAL); return false; }

            const SocketLingerOptions *lingopts = optval;

            struct linger ling;
            ling.l_onoff = lingopts->enable;
            ling.l_linger = lingopts->linger;
            return !setsockopt(socket->desc, level, Socket_Linger, (void *)&ling, sizeof(ling));

        case Socket_RecvTimeout:;
        case Socket_SendTimeout:;
            // this can do setsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) { SETLASTERROR(SOCKERR_FAULT); return false; }
            if (optlen < sizeof(uint32_t)) { SETLASTERROR(SOCKERR_INVAL); return false; }

            #ifdef LIBSOCKET_OS_WINDOWS
                const void *data = optval;
                const socklen_t size = sizeof(uint32_t);
            #else
                uint32_t millis = *(const uint32_t *)optval;

                struct timeval tv;
                tv.tv_sec = millis / 1000;
                tv.tv_usec = (millis % 1000) * 1000;

                const void *data = &tv;
                const socklen_t size = sizeof(tv);
            #endif

            return !setsockopt(socket->desc, level, optname, data, size);

        default:
            return !setsockopt(socket->desc, level, optname, optval, optlen);
    }
}

bool socket_getpeername(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{ ENSURE_INIT; return !getpeername(socket->desc, (struct sockaddr *)sockaddr, size); }

bool socket_getsockname(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{ ENSURE_INIT; return !getsockname(socket->desc, (struct sockaddr *)sockaddr, size); }

SOCKETDESCRIPTOR socket_gethandle(const Socket *socket) { ENSURE_INIT; return socket->desc; }
