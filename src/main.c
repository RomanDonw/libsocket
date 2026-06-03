/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "init.h"
#include "err.h"
#include "util.h"
#include "types.h"
#include "sockslist.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/time.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
#endif

SocketError socket_open(Socket **socket_, SocketAddressFamily af, SocketType type, SocketProtocol protocol)
{
    ENSURE_INIT;
    SocketError err;

    SOCKETDESCRIPTOR desc = socket(af, type, protocol);
    if (desc == INVALID_SOCKET) return GETLASTTRANSLATEDSYSERR();

    Socket *ret = allocs.malloc(sizeof(Socket));
    if (!ret) { CLOSESOCKETDESC(desc); return SocketError_MemoryAllocationFailed; }
    ret->af = af;
    ret->type = type;
    ret->protocol = protocol;
    ret->desc = desc;
    if ((err = socket_setnonblocking(ret, false)) != SocketError_Success)
    {
        CLOSESOCKETDESC(desc);
        allocs.free(ret);
        return err;
    }

    SocketsListError listerr = sockslist_add(ret);
    if (listerr != SocketsListError_Success)
    {
        CLOSESOCKETDESC(desc);
        allocs.free(ret);

        switch (listerr)
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

    SocketError err = __closesocket(socket);
    if (err != SocketError_Success) return err;

    sockslist_remove(socket);
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
    SocketError err;

    SOCKETDESCRIPTOR desc = accept(socket->desc, sockaddr, sockaddrlen);
    if (desc == INVALID_SOCKET) return GETLASTTRANSLATEDSYSERR();

    Socket *ret = allocs.malloc(sizeof(Socket));
    if (!ret) { CLOSESOCKETDESC(desc); return SocketError_MemoryAllocationFailed; }
    ret->af = socket->af;
    ret->type = socket->type;
    ret->protocol = socket->protocol;
    ret->desc = desc;
    if ((err = socket_setnonblocking(ret, socket->nonblocking)) != SocketError_Success)
    {
        CLOSESOCKETDESC(desc);
        allocs.free(ret);
        return err;
    }

    SocketsListError listerr = sockslist_add(ret);
    if (listerr != SocketsListError_Success)
    {
        CLOSESOCKETDESC(desc);
        allocs.free(ret);

        switch (listerr)
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
    if (procbytes < 0) return GETLASTTRANSLATEDSYSERR();\
    if (processedbytes) *processedbytes = procbytes;\
    return SocketError_Success;

SocketError socket_recv(const Socket *socket, void *buffer, size_t len, size_t *processedbytes, int flags)
{ SOCKIOFUNCPROTO(recv(socket->desc, buffer, CLAMPSIZET(len), flags)) }

SocketError socket_recvfrom(const Socket *socket, void *buffer, size_t len, size_t *processedbytes, int flags, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{ SOCKIOFUNCPROTO(recvfrom(socket->desc, buffer, CLAMPSIZET(len), flags, (struct sockaddr *)sockaddr, sockaddrlen)) }

SocketError socket_send(const Socket *socket, const void *data, size_t len, size_t *processedbytes, int flags)
{ SOCKIOFUNCPROTO(send(socket->desc, data, CLAMPSIZET(len), flags)) }

SocketError socket_sendto(const Socket *socket, const void *buffer, size_t len, size_t *processedbytes, int flags, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{ SOCKIOFUNCPROTO(sendto(socket->desc, buffer, CLAMPSIZET(len), flags, (const struct sockaddr *)sockaddr, sockaddrlen)) }

#undef SOCKIOFUNCPROTO

#ifdef LIBSOCKET_OS_WINDOWS
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctlsocket(desc, option, value_ptr))
#else
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctl(desc, option, value_ptr))
#endif

bool socket_isnonblocking(const Socket *socket) { return socket->nonblocking; }

SocketError socket_setnonblocking(Socket *socket, bool enable)
{
    ENSURE_INIT;

    #ifdef LIBSOCKET_OS_WINDOWS
        unsigned long val = enable;
        if (IOCTLSOCKET(socket->desc, FIONBIO, &val)) return GETLASTTRANSLATEDSYSERR();
    #else
        int flags = fcntl(socket->desc, F_GETFL, 0);
        if (flags < 0) return GETLASTTRANSLATEDSYSERR();

        if ((enable && fcntl(socket->desc, F_SETFL, flags | O_NONBLOCK) < 0) ||\
            (!enable && fcntl(socket->desc, F_SETFL, flags & (~O_NONBLOCK)) < 0))
                return GETLASTTRANSLATEDSYSERR();
    #endif

    socket->nonblocking = enable;
    return SocketError_Success;
}

SocketError socket_getreadablebytes(const Socket *socket, size_t *availbytes)
{
    ENSURE_INIT;

    #ifdef LIBSOCKET_OS_WINDOWS
        unsigned long val;
    #else
        int val;
    #endif
    if (IOCTLSOCKET(socket->desc, FIONREAD, &val)) return GETLASTTRANSLATEDSYSERR();
    
    *availbytes = val;
    return SocketError_Success;
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
    switch (flags & 3) // 11b
    {
        case 3: // 11b
            mode = SHUT_RDWR;
            break;

        case 2: // 10b
            mode = SHUT_WR;
            break;

        case 1: // 01b
            mode = SHUT_RD;
            break;
    }

    if (shutdown(socket->desc, mode)) return GETLASTTRANSLATEDSYSERR();

    return SocketError_Success;
}

SocketAddressFamily socket_getaf(const Socket *socket) { return socket->af; }
SocketType socket_gettype(const Socket *socket) { return socket->type; }
SocketProtocol socket_getprotocol(const Socket *socket) { return socket->protocol; }

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
