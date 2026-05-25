/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"

#include <string.h>
#include <limits.h>

#include "init.h"
#include "socket.h"

SocketError socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, socklen_t *optlen)
{
    ENSURE_INIT;

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
                    socklen_t lingsz = sizeof(ling);
                    if (getsockopt(socket->desc, level, optname, (void *)&ling, &lingsz)) return GETLASTTRANSLATEDSYSERR();
                    if (lingsz != sizeof(ling)) return SocketError_InternalUnknownError;

                    SocketLingerOptions lingopts;
                    lingopts.enable = ling.l_onoff;
                    lingopts.linger = (ling.l_linger > USHRT_MAX) ? USHRT_MAX : ling.l_linger;

                    if (*optlen > 0) memcpy(optval, &lingopts, (*optlen > sizeof(lingopts)) ? sizeof(lingopts) : *optlen);
                    *optlen = sizeof(lingopts);
                    return SocketError_Success;
                }

                case SocketOptionName_Socket_RecvTimeout:
                case SocketOptionName_Socket_SendTimeout:
                {
                    uint32_t millis;
                    #ifdef LIBSOCKET_OS_WINDOWS
                        socklen_t millissz = sizeof(millis);
                        if (getsockopt(socket->desc, level, optname, (void *)&millis, &millissz)) return GETLASTTRANSLATEDSYSERR();
                        if (millissz != sizeof(millis)) return SocketError_InternalUnknownError;
                    #else
                        struct timeval tv;
                        socklen_t tvsz = sizeof(tv);
                        if (getsockopt(socket->desc, level, optname, (void *)&tv, &tvsz)) return GETLASTTRANSLATEDSYSERR();
                        if (tvsz != sizeof(tv)) return SocketError_InternalUnknownError;

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

        default:
            return SocketError_IncorrectArgumentValue;
    }

    return SocketError_UnsupportedProtocolOption;

    // =============================================================================

    handlebool:
    {
        #ifdef LIBSOCKET_OS_WINDOWS
            DWORD val;
        #else
            int val;
        #endif
        socklen_t valsz = sizeof(val);
        if (getsockopt(socket->desc, level, optname, (void *)&val, &valsz)) return GETLASTTRANSLATEDSYSERR();
        if (valsz != sizeof(val)) return SocketError_InternalUnknownError;

        bool ret = val;
        if (*optlen > 0) memcpy(optval, &ret, (*optlen > sizeof(ret)) ? sizeof(ret) : *optlen);
        *optlen = sizeof(ret);
        return SocketError_Success;
    }
    return SocketError_Success;

    handleint:
    {
        #ifdef LIBSOCKET_OS_WINDOWS
            int val;
            socklen_t valsz = sizeof(val);
            if (getsockopt(socket->desc, level, optname, (void *)&val, &valsz)) return GETLASTTRANSLATEDSYSERR();
            if (valsz != sizeof(val)) return SocketError_InternalUnknownError;

            if (*optlen > 0) memcpy(optval, &val, (*optlen > sizeof(val)) ? sizeof(val) : *optlen);
            *optlen = sizeof(val);
        #else
            if (getsockopt(socket->desc, level, optname, optval, optlen)) return GETLASTTRANSLATEDSYSERR();
        #endif
    }
    return SocketError_Success;
}

SocketError socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, socklen_t optlen)
{
    ENSURE_INIT;

    if (!optval) return SocketError_Fault;

    switch (level)
    {
        case SocketOptionLevel_Socket:
            switch (optname)
            {
                case SocketOptionName_Socket_AcceptConnections:
                case SocketOptionName_Socket_InternalError:
                case SocketOptionName_Socket_Type:
                    return SocketError_UnsupportedOperation;

                case SocketOptionName_Socket_KeepAliveConnection:
                case SocketOptionName_Socket_AllowReuseAddress:
                case SocketOptionName_Socket_Broadcast:
                    goto handlebool;

                case SocketOptionName_Socket_RecvBufferSize:
                case SocketOptionName_Socket_SendBufferSize:
                    goto handleint;

                case SocketOptionName_Socket_Linger:
                {
                    if (optlen < sizeof(SocketLingerOptions)) return SocketError_IncorrectArgumentValue;

                    const SocketLingerOptions *lingopts = optval;

                    struct linger ling;
                    ling.l_onoff = lingopts->enable;
                    ling.l_linger = lingopts->linger;
                    if (setsockopt(socket->desc, level, optname, (void *)&ling, sizeof(ling))) return GETLASTTRANSLATEDSYSERR();
                    return SocketError_Success;
                }

                case SocketOptionName_Socket_RecvTimeout:;
                case SocketOptionName_Socket_SendTimeout:;
                {
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

        default:
            return SocketError_IncorrectArgumentValue;
    }

    return SocketError_UnsupportedProtocolOption;

    // =============================================================================

    handlebool:
    {
        if (optlen < sizeof(bool)) return SocketError_IncorrectArgumentValue;

        #ifdef LIBSOCKET_OS_WINDOWS
            DWORD val = (*(bool *)optval) ? TRUE : FALSE;
        #else
            int val = *(bool *)optval;
        #endif

        if (setsockopt(socket->desc, level, optname, (void *)&val, sizeof(val))) return GETLASTTRANSLATEDSYSERR();
    }
    return SocketError_Success;

    handleint:
    {
        if (optlen < sizeof(int)) return SocketError_IncorrectArgumentValue;

        #ifdef LIBSOCKET_OS_WINDOWS
            DWORD val = *(int *)optval;
            optval = &val;
            optlen = sizeof(val);
        #endif

        if (setsockopt(socket->desc, level, optname, optval, optlen)) return GETLASTTRANSLATEDSYSERR();
    }
    return SocketError_Success;
}