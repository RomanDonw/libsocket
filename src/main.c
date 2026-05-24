/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "init.h"
#include "err.h"
#include "util.h"
#include "socket.h"
#include "sockslist.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/time.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
#endif

#ifdef LIBSOCKET_OS_WINDOWS
    #define CLOSESOCKET(descr) closesocket(descr)
#else
    #define CLOSESOCKET(descr) close(descr)
#endif

SocketError socket_open(Socket **socket_, SocketAddressFamily af, SocketType type, SocketProtocol protocol)
{
    ENSURE_INIT;

    SOCKETDESCRIPTOR desc = socket(af, type, protocol);
    if (desc == INVALID_SOCKET) return GETLASTTRANSLATEDSYSERR();

    Socket *ret = libsocket_malloc(sizeof(Socket));
    if (!ret) { CLOSESOCKET(desc); return SocketError_MemoryAllocationFailed; }
    ret->af = af;
    ret->type = type;
    ret->protocol = protocol;
    ret->desc = desc;

    SocketsListError err = sockslist_add(ret);
    if (err != SocketsListError_Success)
    {
        CLOSESOCKET(desc);
        libsocket_free(ret);

        switch (err)
        {
            case SocketsListError_MemoryAllocationFailed:
                return SocketError_MemoryAllocationFailed;

            default:
                return SocketError_Fault;
        }
    }

    *socket_ = ret;
    return SocketError_Success;
}

SocketError socket_close(Socket *socket)
{
    ENSURE_INIT;

    if (!sockslist_has(socket)) return SocketError_Fault;

    if (CLOSESOCKET(socket->desc)) return GETLASTTRANSLATEDSYSERR();

    sockslist_remove(socket);
    libsocket_free(socket);
    return SocketError_Success;
}

SocketError socket_listen(const Socket *socket, int backlog)
{
    ENSURE_INIT;
    if (listen(socket->desc, backlog)) return GETLASTTRANSLATEDSYSERR();
    return SocketError_Success;
}

SocketError socket_connect(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{
    ENSURE_INIT;
    if (connect(socket->desc, (struct sockaddr *)sockaddr, sockaddrlen)) return GETLASTTRANSLATEDSYSERR();
    return SocketError_Success;
}

SocketError socket_bind(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{
    ENSURE_INIT;
    if (bind(socket->desc, (struct sockaddr *)sockaddr, sockaddrlen)) return GETLASTTRANSLATEDSYSERR();
    return SocketError_Success;
}

SocketError socket_accept(Socket **acceptedsocket, const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{
    ENSURE_INIT;

    SOCKETDESCRIPTOR desc = accept(socket->desc, sockaddr, sockaddrlen);
    if (desc == INVALID_SOCKET) return GETLASTTRANSLATEDSYSERR();

    Socket *ret = libsocket_malloc(sizeof(Socket));
    if (!ret) { CLOSESOCKET(desc); return SocketError_MemoryAllocationFailed; }
    ret->af = socket->af;
    ret->type = socket->type;
    ret->protocol = socket->protocol;
    ret->desc = desc;

    SocketsListError err = sockslist_add(ret);
    if (err != SocketsListError_Success)
    {
        CLOSESOCKET(desc);
        libsocket_free(ret);

        switch (err)
        {
            case SocketsListError_MemoryAllocationFailed:
                return SocketError_MemoryAllocationFailed;

            default:
                return SocketError_Fault;
        }
    }

    *acceptedsocket = ret;
    return SocketError_Success;
}

#define SOCKIOFUNCPROTO(func) \
    ENSURE_INIT;\
    ssize_t procbytes = func;\
    if (processedbytes) *processedbytes = procbytes;\
    if (procbytes < 0) return GETLASTTRANSLATEDSYSERR();\
    return SocketError_Success;

SocketError socket_recv(const Socket *socket, void *buffer, size_t len, ssize_t *processedbytes, int flags)
{ SOCKIOFUNCPROTO(recv(socket->desc, buffer, CLAMPSIZET(len), flags)) }

SocketError socket_recvfrom(const Socket *socket, void *buffer, size_t len, ssize_t *processedbytes, int flags, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{ SOCKIOFUNCPROTO(recvfrom(socket->desc, buffer, CLAMPSIZET(len), flags, (struct sockaddr *)sockaddr, sockaddrlen)) }

SocketError socket_send(const Socket *socket, const void *data, size_t len, ssize_t *processedbytes, int flags)
{ SOCKIOFUNCPROTO(send(socket->desc, data, CLAMPSIZET(len), flags)) }

SocketError socket_sendto(const Socket *socket, const void *buffer, size_t len, ssize_t *processedbytes, int flags, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{ SOCKIOFUNCPROTO(sendto(socket->desc, buffer, CLAMPSIZET(len), flags, (const struct sockaddr *)sockaddr, sockaddrlen)) }

#ifdef LIBSOCKET_OS_WINDOWS
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctlsocket(desc, option, value_ptr))
#else
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctl(desc, option, value_ptr))
#endif

SocketError socket_ioctl(const Socket *socket, SocketIOCTLOption option, void *value)
{
    ENSURE_INIT;

    switch (option)
    {
        case SocketIOCTLOption_NonBlockingIO:
        {
            #ifdef LIBSOCKET_OS_WINDOWS
                unsigned long val = *(bool *)value;
                if (IOCTLSOCKET(socket->desc, FIONBIO, &val)) return GETLASTTRANSLATEDSYSERR();
            #else
                int flags = fcntl(socket->desc, F_GETFL, 0);
                if (flags < 0) return GETLASTTRANSLATEDSYSERR();

                if (*(bool *)value && fcntl(socket->desc, F_SETFL, flags | O_NONBLOCK) < 0) return GETLASTTRANSLATEDSYSERR();
                else if (!(*(bool *)value) && fcntl(socket->desc, F_SETFL, flags & (~O_NONBLOCK)) < 0) return GETLASTTRANSLATEDSYSERR();
            #endif

            return SocketError_Success;
        }

        case SocketIOCTLOption_AvailableDataToRead:
        {
            #ifdef LIBSOCKET_OS_WINDOWS
                unsigned long val;
            #else
                int val;
            #endif
            if (IOCTLSOCKET(socket->desc, FIONREAD, &val)) return GETLASTTRANSLATEDSYSERR();
            *(uint32_t *)value = val;
            return SocketError_Success;
        }

        default:
            return SocketError_IncorrectArgumentValue;
    }
}

#ifdef LIBSOCKET_OS_WINDOWS
    #define SHUT_RD SD_RECEIVE
    #define SHUT_WR SD_SEND
    #define SHUT_RDWR SD_BOTH
#endif

SocketError socket_shutdown(const Socket *socket, SocketShutdownFlags flags)
{
    ENSURE_INIT;

    int mode = 0;
    switch (flags & SOCKET_SD_ALLFLAGS)
    {
        case 0b11:
            mode = SHUT_RDWR;
            break;

        case 0b10:
            mode = SHUT_WR;
            break;

        case 0b01:
            mode = SHUT_RD;
            break;
    }

    if (shutdown(socket->desc, mode)) return GETLASTTRANSLATEDSYSERR();

    return SocketError_Success;
}

SocketAddressFamily socket_getaf(const Socket *socket) { return socket->af; }
SocketType socket_gettype(const Socket *socket) { return socket->type; }
SocketProtocol socket_getprotocol(const Socket *socket) { return socket->protocol; }

SocketError socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, socklen_t *optlen)
{
    ENSURE_INIT;

    switch (optname)
    {
        case SocketOptionName_Socket_Linger:;
            // this can do getsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) return SocketError_Fault;

            struct linger ling;
            socklen_t lingsz = sizeof(ling);
            if (getsockopt(socket->desc, level, optname, (void *)&ling, &lingsz)) return GETLASTTRANSLATEDSYSERR();
            if (lingsz > sizeof(ling)) return SocketError_InternalUnknownError;

            SocketLingerOptions lingopts;
            lingopts.enable = ling.l_onoff;
            lingopts.linger = (ling.l_linger > USHRT_MAX) ? USHRT_MAX : ling.l_linger;

            if (*optlen > 0) memcpy(optval, &lingopts, (*optlen > sizeof(lingopts)) ? sizeof(lingopts) : *optlen);
            *optlen = sizeof(lingopts);
            return SocketError_Success;

        case SocketOptionName_Socket_RecvTimeout:;
        case SocketOptionName_Socket_SendTimeout:;
            // this can do getsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) return SocketError_Fault;

            uint32_t millis;
            #ifdef LIBSOCKET_OS_WINDOWS
                socklen_t millissz = sizeof(millis);
                if (getsockopt(socket->desc, level, optname, (void *)&millis, &millissz)) return GETLASTTRANSLATEDSYSERR();
                if (millissz > sizeof(millis)) return SocketError_InternalUnknownError;
            #else
                struct timeval tv;
                socklen_t tvsz = sizeof(tv);
                if (getsockopt(socket->desc, level, optname, (void *)&tv, &tvsz)) return GETLASTTRANSLATEDSYSERR();
                if (tvsz > sizeof(tv)) return SocketError_InternalUnknownError;

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
            return SocketError_Success;

        default:
            if (getsockopt(socket->desc, level, optname, optval, optlen)) return GETLASTTRANSLATEDSYSERR();
            return SocketError_Success;
    }
}

SocketError socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, socklen_t optlen)
{
    ENSURE_INIT;

    switch (optname)
    {
        case SocketOptionName_Socket_Linger:;
            // this can do setsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) return SocketError_Fault;
            if (optlen < sizeof(SocketLingerOptions)) return SocketError_IncorrectArgumentValue;

            const SocketLingerOptions *lingopts = optval;

            struct linger ling;
            ling.l_onoff = lingopts->enable;
            ling.l_linger = lingopts->linger;
            if (setsockopt(socket->desc, level, SocketOptionName_Socket_Linger, (void *)&ling, sizeof(ling))) return GETLASTTRANSLATEDSYSERR();
            return SocketError_Success;

        case SocketOptionName_Socket_RecvTimeout:;
        case SocketOptionName_Socket_SendTimeout:;
            // this can do setsockopt -> if (level != SocketLevel) { SETLASTERROR(SOCKERR_NOPROTOOPT); return false; }
            if (!optval) return SocketError_Fault;
            if (optlen < sizeof(uint32_t)) return SocketError_IncorrectArgumentValue;

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

            if (setsockopt(socket->desc, level, optname, data, size)) return GETLASTTRANSLATEDSYSERR();
            return SocketError_Success;

        default:
            if (setsockopt(socket->desc, level, optname, optval, optlen)) return GETLASTTRANSLATEDSYSERR();
            return SocketError_Success;
    }
}

SocketError socket_getpeername(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{
    ENSURE_INIT;
    if (getpeername(socket->desc, (struct sockaddr *)sockaddr, size)) return GETLASTTRANSLATEDSYSERR();
    return SocketError_Success;
}

SocketError socket_getsockname(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{
    ENSURE_INIT;
    if (getsockname(socket->desc, (struct sockaddr *)sockaddr, size)) return GETLASTTRANSLATEDSYSERR();
    return SocketError_Success;
}

SOCKETDESCRIPTOR socket_gethandle(const Socket *socket) { return socket->desc; }