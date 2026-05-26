/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"

#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "init.h"
#include "socket.h"

#ifdef LIBSOCKET_DEBUG
    #include <stdio.h>
#endif

static SocketError __getsockopt(SOCKETDESCRIPTOR desc, int level, int optname, void *optval, socklen_t optlen);
static void __filloutopt(const void *value, socklen_t size, void *optval, socklen_t *optlen);

SocketError socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, socklen_t *optlen)
{
    ENSURE_INIT;

    SocketError err;

    if (!optval) return SocketError_Fault;

    switch (level)
    {
        case SocketOptionLevel_Socket:
            switch (optname)
            {
                case SocketOptionName_Socket_KeepAliveConnection:
                case SocketOptionName_Socket_AllowReuseAddress:
                case SocketOptionName_Socket_Broadcast:
                case SocketOptionName_Socket_AcceptConnections:
                    goto handlebool;

                case SocketOptionName_Socket_RecvBufferSize:
                case SocketOptionName_Socket_SendBufferSize:
                case SocketOptionName_Socket_InternalError:
                case SocketOptionName_Socket_Type:
                    goto handleint;

                case SocketOptionName_Socket_Linger:
                {
                    struct linger ling;
                    if ((err = __getsockopt(socket->desc, level, optname, &ling, sizeof(ling))) != SocketError_Success) return err;

                    SocketLingerOptions lingopts;
                    lingopts.enable = ling.l_onoff;
                    lingopts.linger = (ling.l_linger > USHRT_MAX) ? USHRT_MAX : ling.l_linger;

                    __filloutopt(&lingopts, sizeof(lingopts), optval, optlen);
                    return SocketError_Success;
                }

                case SocketOptionName_Socket_RecvTimeout:
                case SocketOptionName_Socket_SendTimeout:
                {
                    uint32_t millis;
                    #ifdef LIBSOCKET_OS_WINDOWS
                        if ((err = __getsockopt(socket->desc, level, optname, &millis, sizeof(millis))) != SocketError_Success) return err;
                    #else
                        struct timeval tv;
                        if ((err = __getsockopt(socket->desc, level, optname, &tv, sizeof(tv))) != SocketError_Success) return err;

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

                    __filloutopt(&millis, sizeof(millis), optval, optlen);
                    return SocketError_Success;
                }
            }
            break;

        case SocketOptionLevel_TCP:
            switch (optname)
            {
                case SocketOptionName_TCP_NoDelay:
                case SocketOptionName_TCP_FastOpen:
                    goto handlebool;

                case SocketOptionName_TCP_MaxKeepAliveProbes:
                case SocketOptionName_TCP_KeepAliveProbesInterval:
                case SocketOptionName_TCP_ConnectionKeepIdleTime:
                    goto handleint;
            }
            break;

        case SocketOptionLevel_IP:
            {
                switch (optname)
                {
                    case SocketOptionName_IP_TimeToLive:
                        goto handleuint8;
                }
            }
            break;

        default:
            return SocketError_IncorrectArgumentValue;
    }

    return SocketError_UnsupportedProtocolOption;

    // =============================================================================

    {
        #ifdef LIBSOCKET_OS_WINDOWS
            DWORD val_dwint;
        #else
            int val_dwint;
        #endif

        union
        {
            bool boolean;
            int integer;
            uint8_t uint8;
        } value;
        socklen_t value_realsize;

        // =============================================================================

        handlebool:
            if ((err = __getsockopt(socket->desc, level, optname, &val_dwint, sizeof(val_dwint))) != SocketError_Success) return err;
            value.boolean = val_dwint;
            value_realsize = sizeof(value.boolean);
        goto filloptval;
        
        handleint:
            if ((err = __getsockopt(socket->desc, level, optname, &val_dwint, sizeof(val_dwint))) != SocketError_Success) return err;
            value.integer = val_dwint;
            value_realsize = sizeof(value.integer);
        goto filloptval;
            
        handleuint8:
            if ((err = __getsockopt(socket->desc, level, optname, &val_dwint, sizeof(val_dwint))) != SocketError_Success) return err;
            value.uint8 = val_dwint;
            value_realsize = sizeof(value.uint8);
        goto filloptval;

        // =============================================================================
        
        filloptval:
            __filloutopt(&value, value_realsize, optval, optlen);
        return SocketError_Success;
    }
}

static SocketError __getsockopt(SOCKETDESCRIPTOR desc, int level, int optname, void *optval, socklen_t optlen)
{
    socklen_t realoptlen = optlen;
    if (getsockopt(desc, level, optname, optval, &realoptlen)) return GETLASTTRANSLATEDSYSERR();
    if (realoptlen != optlen)
    {
        #ifdef LIBSOCKET_DEBUG
            fprintf(stderr, "Got internal size mismatch in __getsockopt with params: level=%i, optname=%i.\n", level, optname);
        #endif
        return SocketError_InternalSizeMismatch;
    }

    return SocketError_Success;
}

static void __filloutopt(const void *value, socklen_t size, void *optval, socklen_t *optlen)
{
    if (*optlen > 0) memcpy(optval, value, (*optlen > size) ? size : *optlen);
    *optlen = size;
}