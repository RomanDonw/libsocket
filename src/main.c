#include "libsocket.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "init.h"
#include "err.h"
#include "util.h"
#include "sockslist.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/time.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
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

#ifdef LIBSOCKET_OS_WINDOWS
    #define CLOSESOCKET(descr) closesocket(descr)
#else
    #define CLOSESOCKET(descr) close(descr)
#endif

Socket *socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol)
{
    ENSURE_INIT(NULL);

    SOCKETDESCRIPTOR desc = socket(af, type, protocol);
    if (desc == INVALID_SOCKET) RETURNWITHSYSERR(NULL);

    Socket *ret = libsocket_malloc(sizeof(Socket));
    if (!ret) RETURNWITHERROR(SocketError_MemoryAllocationFailed, NULL);
    ret->af = af;
    ret->type = type;
    ret->protocol = protocol;
    ret->desc = desc;

    sockslisterr_t err = sockslist_add(ret);
    if (err != SOCKSLISTERR_SUCCESS)
    {
        CLOSESOCKET(desc);
        libsocket_free(ret);

        switch (err)
        {
            case SOCKSLISTERR_NOMEM:
                RETURNWITHERROR(SocketError_MemoryAllocationFailed, NULL);

            default:
                RETURNWITHERROR(SocketError_Fault, NULL);
        }
    }

    RETURNWITHSUCCESS(ret);
}

bool socket_close(Socket *socket)
{
    ENSURE_INIT(false);

    if (!sockslist_has(socket)) RETURNWITHERROR(SocketError_Fault, false);

    if (CLOSESOCKET(socket->desc)) RETURNWITHSYSERR(false);

    sockslist_remove(socket);
    libsocket_free(socket);
    RETURNWITHSUCCESS(true);
}

bool socket_listen(const Socket *socket, int backlog)
{
    ENSURE_INIT(false);
    if (listen(socket->desc, backlog)) RETURNWITHSYSERR(false);
    RETURNWITHSUCCESS(true);
}

bool socket_connect(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{
    ENSURE_INIT(false);
    if (connect(socket->desc, (struct sockaddr *)sockaddr, sockaddrlen)) RETURNWITHSYSERR(false);
    RETURNWITHSUCCESS(true);
}

bool socket_bind(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{
    ENSURE_INIT(false);
    if (bind(socket->desc, (struct sockaddr *)sockaddr, sockaddrlen)) RETURNWITHSYSERR(false);
    RETURNWITHSUCCESS(true);
}

Socket *socket_accept(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{
    ENSURE_INIT(NULL);

    SOCKETDESCRIPTOR desc = accept(socket->desc, sockaddr, sockaddrlen);
    if (desc == INVALID_SOCKET) RETURNWITHSYSERR(NULL);

    Socket *ret = libsocket_malloc(sizeof(Socket));
    if (!ret) RETURNWITHERROR(SocketError_MemoryAllocationFailed, NULL);
    ret->af = socket->af;
    ret->type = socket->type;
    ret->protocol = socket->protocol;
    ret->desc = desc;

    sockslisterr_t err = sockslist_add(ret);
    if (err != SOCKSLISTERR_SUCCESS)
    {
        CLOSESOCKET(desc);
        libsocket_free(ret);

        switch (err)
        {
            case SOCKSLISTERR_NOMEM:
                RETURNWITHERROR(SocketError_MemoryAllocationFailed, NULL);

            default:
                RETURNWITHERROR(SocketError_Fault, NULL);
        }
    }

    RETURNWITHSUCCESS(ret);
}

#define SOCKIOFUNCPROTO(func) \
    ENSURE_INIT(-1);\
    ssize_t ret = func;\
    if (ret < 0) socket_lasterror = translateerror(GETLASTERROR());\
    else socket_lasterror = SocketError_Success;\
    return ret;

ssize_t socket_recv(const Socket *socket, void *buffer, size_t len, int flags)
{ SOCKIOFUNCPROTO(recv(socket->desc, buffer, CLAMPSIZET(len), flags)) }

ssize_t socket_recvfrom(const Socket *socket, void *buffer, size_t len, int flags, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{ SOCKIOFUNCPROTO(recvfrom(socket->desc, buffer, CLAMPSIZET(len), flags, (struct sockaddr *)sockaddr, sockaddrlen)) }

ssize_t socket_send(const Socket *socket, const void *data, size_t len, int flags)
{ SOCKIOFUNCPROTO(send(socket->desc, data, CLAMPSIZET(len), flags)) }

ssize_t socket_sendto(const Socket *socket, const void *buffer, size_t len, int flags, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{ SOCKIOFUNCPROTO(sendto(socket->desc, buffer, CLAMPSIZET(len), flags, (const struct sockaddr *)sockaddr, sockaddrlen)) }

#ifdef LIBSOCKET_OS_WINDOWS
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctlsocket(desc, option, value_ptr))
#else
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctl(desc, option, value_ptr))
#endif

bool socket_ioctl(const Socket *socket, SocketIOCTLOption option, void *value)
{
    ENSURE_INIT(false);

    switch (option)
    {
        case NonBlockingIO:
        {
            #ifdef LIBSOCKET_OS_WINDOWS
                unsigned long val = *(bool *)value;
                if (IOCTLSOCKET(socket->desc, FIONBIO, &val)) RETURNWITHSYSERR(false);
            #else
                int flags = fcntl(socket->desc, F_GETFL, 0);
                if (flags < 0) RETURNWITHSYSERR(false);

                if (*(bool *)value && fcntl(socket->desc, F_SETFL, flags | O_NONBLOCK) < 0) RETURNWITHSYSERR(false)
                else if (!(*(bool *)value) && fcntl(socket->desc, F_SETFL, flags & (~O_NONBLOCK)) < 0) RETURNWITHSYSERR(false);
            #endif

            RETURNWITHSUCCESS(true);
        }

        case AvailableDataToRead:
        {
            #ifdef LIBSOCKET_OS_WINDOWS
                unsigned long val;
            #else
                int val;
            #endif
            if (IOCTLSOCKET(socket->desc, FIONREAD, &val)) RETURNWITHSYSERR(false);
            *(uint32_t *)value = val;
            RETURNWITHSUCCESS(true);
        }

        default:
            RETURNWITHERROR(SocketError_IncorrectArgumentValue, false);
    }
}

bool socket_shutdown(const Socket *socket, SocketShutdownMode mode)
{
    ENSURE_INIT(false);
    if (shutdown(socket->desc, mode)) RETURNWITHSYSERR(false);
    RETURNWITHSUCCESS(true);
}

SocketAddressFamily socket_getaf(const Socket *socket) { return socket->af; }
SocketType socket_gettype(const Socket *socket) { return socket->type; }
SocketProtocol socket_getprotocol(const Socket *socket) { return socket->protocol; }

bool socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, socklen_t *optlen)
{
    ENSURE_INIT(false);

    switch (optname)
    {
        case Socket_Linger:;
            // this can do getsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) RETURNWITHERROR(SocketError_Fault, false);

            struct linger ling;
            socklen_t lingsz = sizeof(ling);
            if (getsockopt(socket->desc, SocketLevel, Socket_Linger, (void *)&ling, &lingsz)) RETURNWITHSYSERR(false);
            if (lingsz > sizeof(ling)) RETURNWITHERROR(SocketError_InternalUnknownError, false);

            SocketLingerOptions lingopts;
            lingopts.enable = ling.l_onoff;
            lingopts.linger = (ling.l_linger > USHRT_MAX) ? USHRT_MAX : ling.l_linger;

            if (*optlen > 0) memcpy(optval, &lingopts, (*optlen > sizeof(lingopts)) ? sizeof(lingopts) : *optlen);
            *optlen = sizeof(lingopts);
            RETURNWITHSUCCESS(true);

        case Socket_RecvTimeout:;
        case Socket_SendTimeout:;
            // this can do getsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) RETURNWITHERROR(SocketError_Fault, false);

            uint32_t millis;
            #ifdef LIBSOCKET_OS_WINDOWS
                socklen_t millissz = sizeof(millis);
                if (getsockopt(socket->desc, level, optname, (void *)&millis, &millissz)) RETURNWITHSYSERR(false);
                if (millissz > sizeof(millis)) RETURNWITHERROR(SocketError_InternalUnknownError, false);
            #else
                struct timeval tv;
                socklen_t tvsz = sizeof(tv);
                if (getsockopt(socket->desc, level, optname, (void *)&tv, &tvsz)) RETURNWITHSYSERR(false);
                if (tvsz > sizeof(tv)) RETURNWITHERROR(SocketError_InternalUnknownError, false);

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
            RETURNWITHSUCCESS(true);

        default:
            if (getsockopt(socket->desc, level, optname, optval, optlen)) RETURNWITHSYSERR(false);
            RETURNWITHSUCCESS(true);
    }
}

bool socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, socklen_t optlen)
{
    ENSURE_INIT(false);

    switch (optname)
    {
        case Socket_Linger:;
            // this can do setsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) RETURNWITHERROR(SocketError_Fault, false);
            if (optlen < sizeof(SocketLingerOptions)) RETURNWITHERROR(SocketError_IncorrectArgumentValue, false);

            const SocketLingerOptions *lingopts = optval;

            struct linger ling;
            ling.l_onoff = lingopts->enable;
            ling.l_linger = lingopts->linger;
            if (setsockopt(socket->desc, level, Socket_Linger, (void *)&ling, sizeof(ling))) RETURNWITHSYSERR(false);
            RETURNWITHSUCCESS(true);

        case Socket_RecvTimeout:;
        case Socket_SendTimeout:;
            // this can do setsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) RETURNWITHERROR(SocketError_Fault, false);
            if (optlen < sizeof(uint32_t)) RETURNWITHERROR(SocketError_IncorrectArgumentValue, false);

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

            if (setsockopt(socket->desc, level, optname, data, size)) RETURNWITHSYSERR(false);
            RETURNWITHSUCCESS(true);

        default:
            if (setsockopt(socket->desc, level, optname, optval, optlen)) RETURNWITHSYSERR(false);
            RETURNWITHSUCCESS(true);
    }
}

bool socket_getpeername(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{
    ENSURE_INIT(false);
    if (getpeername(socket->desc, (struct sockaddr *)sockaddr, size)) RETURNWITHSYSERR(false);
    RETURNWITHSUCCESS(true);
}

bool socket_getsockname(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{
    ENSURE_INIT(false);
    if (getsockname(socket->desc, (struct sockaddr *)sockaddr, size)) RETURNWITHSYSERR(false);
    RETURNWITHSUCCESS(true);
}

SOCKETDESCRIPTOR socket_gethandle(const Socket *socket) { return socket->desc; }