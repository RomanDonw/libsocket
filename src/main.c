/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <libmutex.h>

#include "init.h"
#include "err.h"
#include "util.h"
#include "types.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/time.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
#endif

NError socket_open(Socket **socket_, SocketAddressFamily af, SocketType type, SocketProtocol protocol)
{
    ENSURE_INIT;
    NError err;

    // =============================================================================
    
    if (af != SocketAddressFamily_IPv4 && af != SocketAddressFamily_IPv6)
    { err = NError_UnsupportedAddressFamily; goto errorquit_generic; }

    if (type != SocketType_Stream && type != SocketType_Datagram)
    { err = NError_UnsupportedSocketType; goto errorquit_generic; }

    if (protocol != SocketProtocol_Unspecified && protocol != SocketProtocol_TCP && protocol != SocketProtocol_UDP)
    { err = NError_UnsupportedProtocol; goto errorquit_generic; }

    // =============================================================================

    SOCKETDESCRIPTOR desc = socket(af, type, protocol);
    if (desc == INVALID_SOCKET) { err = GETLASTTRANSLATEDSYSERR(); goto errorquit_generic; }

    Socket *ret = allocs.malloc(sizeof(Socket));
    if (!ret) { err = NError_MemoryAllocationFailed; goto errorquit_onalloc; }
    ret->af = af;
    ret->type = type;
    ret->protocol = protocol;
    ret->desc = desc;

    if (mutex_create(&ret->mutex_nonblocking) != MUTEXERROR_SUCCESS)
    { err = NError_MutexAPIError; goto errorquit_onmtxapierr; }

    if ((err = socket_setnonblocking(ret, false)) != NError_Success) goto errorquit_aftermutexcreate;

    SocketsListError listerr = sockslist_add(ret);
    if (listerr != SocketsListError_Success)
    {
        switch (listerr)
        {
            case SocketsListError_MemoryAllocationFailed:
                err = NError_MemoryAllocationFailed;
                break;

            default:
                err = NError_Fault;
        }
        goto errorquit_aftermutexcreate;
    }

    *socket_ = ret;
    return NError_Success;

    // =============================================================================

    errorquit_aftermutexcreate:
        if (mutex_destroy(ret->mutex_nonblocking) != MUTEXERROR_SUCCESS)
        { panic_general(GETLASTTRANSLATEDSYSERR(), "Unable to destroy internal socket mutex on cleanup when handling error."); }
    errorquit_onmtxapierr:
        allocs.free(ret);
    errorquit_onalloc:
        if (CLOSESOCKETDESC(desc))
        { panic_general(GETLASTTRANSLATEDSYSERR(), "Unable to close socket descriptor on cleanup when handling error."); }
    errorquit_generic:
    return err;
}

NError socket_close(Socket *socket)
{
    ENSURE_INIT;
    if ((mutex_lock(sockslist_mutex)) != MUTEXERROR_SUCCESS) return NError_MutexAPIError;

    NError err;

    if (!sockslist_has(socket)) { err = NError_Fault; goto errorquit; }

    if ((err = __closesocket(socket)) != NError_Success) goto errorquit;

    sockslist_remove(socket);
    SAFE_MUTEX_UNLOCK(sockslist_mutex);
    return NError_Success;

    errorquit:
        SAFE_MUTEX_UNLOCK(sockslist_mutex);
    return err;
}

NError socket_listen(const Socket *socket, int backlog)
{
    ENSURE_INIT;
    if (listen(socket->desc, backlog)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NError socket_connect(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{
    ENSURE_INIT;
    if (connect(socket->desc, (struct sockaddr *)sockaddr, sockaddrlen)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NError socket_bind(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{
    ENSURE_INIT;
    if (bind(socket->desc, (struct sockaddr *)sockaddr, sockaddrlen)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NError socket_accept(Socket **acceptedsocket, const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{
    ENSURE_INIT;
    NError err;

    SOCKETDESCRIPTOR desc = accept(socket->desc, sockaddr, sockaddrlen);
    if (desc == INVALID_SOCKET) { err = GETLASTTRANSLATEDSYSERR(); goto errorquit_generic; }

    Socket *ret = allocs.malloc(sizeof(Socket));
    if (!ret) { err = NError_MemoryAllocationFailed; goto errorquit_onalloc; }
    ret->af = socket->af;
    ret->type = socket->type;
    ret->protocol = socket->protocol;
    ret->desc = desc;

    if (mutex_create(&ret->mutex_nonblocking) != MUTEXERROR_SUCCESS)
    { err = NError_MutexAPIError; goto errorquit_onmtxapierr; }

    if ((err = socket_setnonblocking(ret, socket_isnonblocking(socket))) != NError_Success) goto errorquit_aftermutexcreate;

    SocketsListError listerr = sockslist_add(ret);
    if (listerr != SocketsListError_Success)
    {
        switch (listerr)
        {
            case SocketsListError_MemoryAllocationFailed:
                err = NError_MemoryAllocationFailed;
                break;

            default:
                err = NError_Fault;
        }
        goto errorquit_aftermutexcreate;
    }

    *acceptedsocket = ret;
    return NError_Success;

    // =============================================================================

    errorquit_aftermutexcreate:
        if (mutex_destroy(ret->mutex_nonblocking) != MUTEXERROR_SUCCESS)
        { panic_general(GETLASTTRANSLATEDSYSERR(), "Unable to destroy internal socket mutex on cleanup when handling error."); }
    errorquit_onmtxapierr:
        allocs.free(ret);
    errorquit_onalloc:
        CLOSESOCKETDESC(desc);
    errorquit_generic:
    return err;
}

#define SOCKIOFUNCPROTO(func) \
    ENSURE_INIT;\
    ssize_t procbytes = (func);\
    if (procbytes < 0) return GETLASTTRANSLATEDSYSERR();\
    if (processedbytes) *processedbytes = procbytes;\
    return NError_Success;

NError socket_recv(const Socket *socket, void *buffer, size_t len, size_t *processedbytes, int flags)
{ SOCKIOFUNCPROTO(recv(socket->desc, buffer, CLAMPSIZET(len), flags)) }

NError socket_recvfrom(const Socket *socket, void *buffer, size_t len, size_t *processedbytes, int flags, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{ SOCKIOFUNCPROTO(recvfrom(socket->desc, buffer, CLAMPSIZET(len), flags, (struct sockaddr *)sockaddr, sockaddrlen)) }

NError socket_send(const Socket *socket, const void *data, size_t len, size_t *processedbytes, int flags)
{ SOCKIOFUNCPROTO(send(socket->desc, data, CLAMPSIZET(len), flags)) }

NError socket_sendto(const Socket *socket, const void *buffer, size_t len, size_t *processedbytes, int flags, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{ SOCKIOFUNCPROTO(sendto(socket->desc, buffer, CLAMPSIZET(len), flags, (const struct sockaddr *)sockaddr, sockaddrlen)) }

#undef SOCKIOFUNCPROTO

#ifdef LIBSOCKET_OS_WINDOWS
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctlsocket(desc, option, value_ptr))
#else
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctl(desc, option, value_ptr))
#endif

bool socket_isnonblocking(const Socket *socket)
{
    SAFE_MUTEX_LOCK(socket->mutex_nonblocking);
    bool ret = socket->nonblocking;
    SAFE_MUTEX_UNLOCK(socket->mutex_nonblocking);
    return ret;
}

NError socket_setnonblocking(Socket *socket, bool enable)
{
    ENSURE_INIT;

    if (mutex_lock(socket->mutex_nonblocking) != MUTEXERROR_SUCCESS) return NError_MutexAPIError;

    #ifdef LIBSOCKET_OS_WINDOWS
        unsigned long val = enable;
        if (IOCTLSOCKET(socket->desc, FIONBIO, &val)) goto errorquit;
    #else
        int flags = fcntl(socket->desc, F_GETFL, 0);
        if (flags < 0) goto errorquit;

        if ((enable && fcntl(socket->desc, F_SETFL, flags | O_NONBLOCK) < 0) ||\
            (!enable && fcntl(socket->desc, F_SETFL, flags & (~O_NONBLOCK)) < 0))
                goto errorquit;
    #endif

    socket->nonblocking = enable;

    SAFE_MUTEX_UNLOCK(socket->mutex_nonblocking);
    return NError_Success;

    errorquit:
        SAFE_MUTEX_UNLOCK(socket->mutex_nonblocking);
    return GETLASTTRANSLATEDSYSERR();
}

NError socket_getreadablebytes(const Socket *socket, size_t *availbytes)
{
    ENSURE_INIT;

    #ifdef LIBSOCKET_OS_WINDOWS
        unsigned long val;
    #else
        int val;
    #endif
    if (IOCTLSOCKET(socket->desc, FIONREAD, &val)) return GETLASTTRANSLATEDSYSERR();
    
    *availbytes = val;
    return NError_Success;
}

#ifdef LIBSOCKET_OS_WINDOWS
    #define SHUT_RD SD_RECEIVE
    #define SHUT_WR SD_SEND
    #define SHUT_RDWR SD_BOTH
#endif

NError socket_shutdown(const Socket *socket, SocketShutdownFlags flags)
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

    return NError_Success;
}

SocketAddressFamily socket_getaf(const Socket *socket) { return socket->af; }
SocketType socket_gettype(const Socket *socket) { return socket->type; }
SocketProtocol socket_getprotocol(const Socket *socket) { return socket->protocol; }

NError socket_getpeername(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{
    ENSURE_INIT;
    if (getpeername(socket->desc, (struct sockaddr *)sockaddr, size)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NError socket_getsockname(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size)
{
    ENSURE_INIT;
    if (getsockname(socket->desc, (struct sockaddr *)sockaddr, size)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

SOCKETDESCRIPTOR socket_gethandle(const Socket *socket) { return socket->desc; }
