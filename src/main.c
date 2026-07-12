/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libnsocket.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "init.h"
#include "err.h"
#include "util.h"
#include "types.h"

#ifndef LIBNSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/time.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
#endif

#ifdef LIBNSOCKET_OS_WINDOWS
    static const DWORD sockoptval_bool_true = TRUE;
#else
    static const int sockoptval_bool_true = true;
#endif

NError nsocket_open(NSocket **socket_, NSocketAddressFamily af, NSocketType type, NSocketProtocol protocol)
{
    ENSURE_INIT;
    NError nerr;

    // =============================================================================
    
    if (af != NSocketAddressFamily_IPv4 && af != NSocketAddressFamily_IPv6)
    { nerr = NError_UnsupportedAddressFamily; goto errorquit_generic; }

    if (type != NSocketType_Stream && type != NSocketType_Datagram)
    { nerr = NError_UnsupportedSocketType; goto errorquit_generic; }

    if (protocol != NSocketProtocol_Unspecified && protocol != NSocketProtocol_TCP && protocol != NSocketProtocol_UDP)
    { nerr = NError_UnsupportedProtocol; goto errorquit_generic; }

    // =============================================================================

    NSOCKET_NATIVEDESCRIPTOR desc = socket(af, type, protocol);
    if (desc == NSOCKET_INVALIDDESCRIPTOR) { nerr = GETLASTTRANSLATEDSYSERR(); goto errorquit_generic; }

    #ifdef LIBNSOCKET_OS_WINDOWS
        if (setsockopt(desc, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (void *)&sockoptval_bool_true, sizeof(sockoptval_bool_true)))
        { nerr = GETLASTTRANSLATEDSYSERR(); goto errorquit_afteropendesc; }
    #else
        if (setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, (void *)&sockoptval_bool_true, sizeof(sockoptval_bool_true)))
        { nerr = GETLASTTRANSLATEDSYSERR(); goto errorquit_afteropendesc; }
    #endif

    NSocket *ret = allocs.malloc(sizeof(NSocket));
    if (!ret) { nerr = NError_MemoryAllocationFailed; goto errorquit_afteropendesc; }
    ret->af = af;
    ret->type = type;
    ret->protocol = protocol;
    ret->desc = desc;

    if ((nerr = nthread_mutex_create(&ret->mutex_nonblocking)) != NError_Success) goto errorquit_afteralloc;

    if (((nerr = nsocket_setnonblocking(ret, false)) != NError_Success) || ((nerr = nthread_mutex_lock(sockslistmutex)) != NError_Success))
    { goto errorquit_aftermutexcreate; }

    if ((nerr = n_unorderedset_addelement(sockslist, &ret)) != NError_Success) goto errorquit_afterlocksockslistmtx;
    SAFE_MUTEX_UNLOCK(sockslistmutex);

    *socket_ = ret;
    return NError_Success;

    // =============================================================================

    errorquit_afterlocksockslistmtx:
        SAFE_MUTEX_UNLOCK(sockslistmutex);
    errorquit_aftermutexcreate:
        {
            NError tmpnerr = nthread_mutex_destroy(ret->mutex_nonblocking);
            if (tmpnerr != NError_Success) panic_general(tmpnerr, "Unable to destroy internal socket mutex on cleanup when handling error.");
        }
    errorquit_afteralloc:
        allocs.free(ret);
    errorquit_afteropendesc:
        if (CLOSESOCKETDESC(desc))
        { panic_general(GETLASTTRANSLATEDSYSERR(), "Unable to close socket descriptor on cleanup when handling error."); }
    errorquit_generic:
    return nerr;
}

NError nsocket_close(NSocket *socket)
{
    ENSURE_INIT;

    NError nerr = nthread_mutex_lock(sockslistmutex);
    if (nerr != NError_Success) return nerr;

    if (!n_unorderedset_haselement(sockslist, &socket)) { nerr = NError_Fault; goto errorquit; }

    if ((nerr = __closesocket(socket)) != NError_Success) goto errorquit;

    if ((nerr = n_unorderedset_removeelement(sockslist, &socket)) != NError_Success)
    { panic_general(nerr, "Unable to remove socket from internal sockets list after closing socket."); }

    SAFE_MUTEX_UNLOCK(sockslistmutex);
    return NError_Success;

    errorquit:
        SAFE_MUTEX_UNLOCK(sockslistmutex);
    return nerr;
}

NError nsocket_listen(const NSocket *socket, int backlog)
{
    ENSURE_INIT;
    if (listen(socket->desc, backlog)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NError nsocket_connect(const NSocket *socket, const NSocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{
    ENSURE_INIT;
    if (connect(socket->desc, (struct sockaddr *)sockaddr, sockaddrlen)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NError nsocket_bind(const NSocket *socket, const NSocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{
    ENSURE_INIT;
    if (bind(socket->desc, (struct sockaddr *)sockaddr, sockaddrlen)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NError nsocket_accept(NSocket **acceptedsocket, const NSocket *socket, NSocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{
    ENSURE_INIT;
    NError nerr;

    NSOCKET_NATIVEDESCRIPTOR desc = accept(socket->desc, sockaddr, sockaddrlen);
    if (desc == NSOCKET_INVALIDDESCRIPTOR) { nerr = GETLASTTRANSLATEDSYSERR(); goto errorquit_generic; }

    #ifdef LIBNSOCKET_OS_WINDOWS
        if (setsockopt(desc, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (void *)&sockoptval_bool_true, sizeof(sockoptval_bool_true)))
        { nerr = GETLASTTRANSLATEDSYSERR(); goto errorquit_afteropendesc; }
    #else
        if (setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, (void *)&sockoptval_bool_true, sizeof(sockoptval_bool_true)))
        { nerr = GETLASTTRANSLATEDSYSERR(); goto errorquit_afteropendesc; }
    #endif

    NSocket *ret = allocs.malloc(sizeof(NSocket));
    if (!ret) { nerr = NError_MemoryAllocationFailed; goto errorquit_afteropendesc; }
    ret->af = socket->af;
    ret->type = socket->type;
    ret->protocol = socket->protocol;
    ret->desc = desc;

    if ((nerr = nthread_mutex_create(&ret->mutex_nonblocking)) != NError_Success) goto errorquit_afteralloc;

    if (((nerr = nsocket_setnonblocking(ret, false)) != NError_Success) || ((nerr = nthread_mutex_lock(sockslistmutex)) != NError_Success))
    { goto errorquit_aftermutexcreate; }

    if ((nerr = n_unorderedset_addelement(sockslist, &ret)) != NError_Success) goto errorquit_afterlocksockslistmtx;
    SAFE_MUTEX_UNLOCK(sockslistmutex);

    *acceptedsocket = ret;
    return NError_Success;

    // =============================================================================

    errorquit_afterlocksockslistmtx:
        SAFE_MUTEX_UNLOCK(sockslistmutex);
    errorquit_aftermutexcreate:
        {
            NError tmpnerr = nthread_mutex_destroy(ret->mutex_nonblocking);
            if (tmpnerr != NError_Success) panic_general(tmpnerr, "Unable to destroy internal socket mutex on cleanup when handling error.");
        }
    errorquit_afteralloc:
        allocs.free(ret);
    errorquit_afteropendesc:
        if (CLOSESOCKETDESC(desc))
        { panic_general(GETLASTTRANSLATEDSYSERR(), "Unable to close socket descriptor on cleanup when handling error."); }
    errorquit_generic:
    return nerr;
}

#define SOCKIOFUNCPROTO(func) \
    ENSURE_INIT;\
    ssize_t procbytes = (func);\
    if (procbytes < 0) return GETLASTTRANSLATEDSYSERR();\
    if (processedbytes) *processedbytes = procbytes;\
    return NError_Success;

NError nsocket_recv(const NSocket *socket, void *buffer, size_t len, size_t *processedbytes, int flags)
{ SOCKIOFUNCPROTO(recv(socket->desc, buffer, CLAMPSIZET(len), flags)) }

NError nsocket_recvfrom(const NSocket *socket, void *buffer, size_t len, size_t *processedbytes, int flags, NSocketAddressInterface *sockaddr, socklen_t *sockaddrlen)
{ SOCKIOFUNCPROTO(recvfrom(socket->desc, buffer, CLAMPSIZET(len), flags, (struct sockaddr *)sockaddr, sockaddrlen)) }

NError nsocket_send(const NSocket *socket, const void *data, size_t len, size_t *processedbytes, int flags)
{ SOCKIOFUNCPROTO(send(socket->desc, data, CLAMPSIZET(len), flags)) }

NError nsocket_sendto(const NSocket *socket, const void *buffer, size_t len, size_t *processedbytes, int flags, const NSocketAddressInterface *sockaddr, socklen_t sockaddrlen)
{ SOCKIOFUNCPROTO(sendto(socket->desc, buffer, CLAMPSIZET(len), flags, (const struct sockaddr *)sockaddr, sockaddrlen)) }

#undef SOCKIOFUNCPROTO

#ifdef LIBNSOCKET_OS_WINDOWS
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctlsocket(desc, option, value_ptr))
#else
    #define IOCTLSOCKET(desc, option, value_ptr) (ioctl(desc, option, value_ptr))
#endif

bool nsocket_isnonblocking(const NSocket *socket)
{
    SAFE_MUTEX_LOCK(socket->mutex_nonblocking);
    bool ret = socket->nonblocking;
    SAFE_MUTEX_UNLOCK(socket->mutex_nonblocking);
    return ret;
}

NError nsocket_setnonblocking(NSocket *socket, bool enable)
{
    ENSURE_INIT;

    NError nerr = nthread_mutex_lock(socket->mutex_nonblocking);
    if (nerr != NError_Success) return nerr;

    #ifdef LIBNSOCKET_OS_WINDOWS
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

NError nsocket_getreadablebytes(const NSocket *socket, size_t *availbytes)
{
    ENSURE_INIT;

    #ifdef LIBNSOCKET_OS_WINDOWS
        unsigned long val;
    #else
        int val;
    #endif
    if (IOCTLSOCKET(socket->desc, FIONREAD, &val)) return GETLASTTRANSLATEDSYSERR();
    
    *availbytes = val;
    return NError_Success;
}

#ifdef LIBNSOCKET_OS_WINDOWS
    #define SHUT_RD SD_RECEIVE
    #define SHUT_WR SD_SEND
    #define SHUT_RDWR SD_BOTH
#endif

NError nsocket_shutdown(const NSocket *socket, NSocketShutdownFlags flags)
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

NSocketAddressFamily nsocket_getaf(const NSocket *socket) { return socket->af; }
NSocketType nsocket_gettype(const NSocket *socket) { return socket->type; }
NSocketProtocol nsocket_getprotocol(const NSocket *socket) { return socket->protocol; }

NError nsocket_getpeername(const NSocket *socket, NSocketAddressInterface *sockaddr, socklen_t *size)
{
    ENSURE_INIT;
    if (getpeername(socket->desc, (struct sockaddr *)sockaddr, size)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NError nsocket_getsockname(const NSocket *socket, NSocketAddressInterface *sockaddr, socklen_t *size)
{
    ENSURE_INIT;
    if (getsockname(socket->desc, (struct sockaddr *)sockaddr, size)) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NSOCKET_NATIVEDESCRIPTOR nsocket_gethandle(const NSocket *socket) { return socket->desc; }
