/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "optfunc.h"

#include "util.h"

static inline NError __getsockopt(SOCKETDESCRIPTOR desc, int level, int optname, void *optval, socklen_t optlen);
static void __filloutopt(const void *value, size_t size, void *optval, size_t *optlen);

NError socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, size_t *optsize)
{
    ENSURE_INIT;

    NError err;

    if (!optval || !optsize) return NError_Fault;

    switch (level)
    {
        case SocketOptionLevel_Socket:
            switch (optname)
            {
                case SocketOptionName_Socket_KeepAliveConnection:
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
                    if ((err = __getsockopt(socket->desc, level, optname, &ling, sizeof(ling))) != NError_Success) return err;

                    /*
                        on Linux (by POSIX standard) 'l_linger' field has 'int' type, but in WinSock it has 'unsigned short' type,
                            so we need to cast types with limits checking when we`re not on Windows.
                    */
                    #ifndef LIBSOCKET_OS_WINDOWS
                        if (ling.l_linger > USHRT_MAX || ling.l_linger < 0) goto varoverflowerr;
                    #endif

                    SocketLingerOptions lingopts;
                    lingopts.enable = ling.l_onoff;
                    lingopts.linger = ling.l_linger;

                    __filloutopt(&lingopts, sizeof(lingopts), optval, optsize);
                    return NError_Success;
                }

                case SocketOptionName_Socket_RecvTimeout:
                case SocketOptionName_Socket_SendTimeout:
                {
                    uint32_t millis;
                    #ifdef LIBSOCKET_OS_WINDOWS
                        if ((err = __getsockopt(socket->desc, level, optname, &millis, sizeof(millis))) != NError_Success) return err;
                    #else
                        struct timeval tv;
                        if ((err = __getsockopt(socket->desc, level, optname, &tv, sizeof(tv))) != NError_Success) return err;

                        uint64_t usecs;

                        // algorithm here isn`t perfect, but it works!
                        
                        // check seconds on possible overflow when it will be converted to microseconds & set usecs variable.
                        if (tv.tv_sec > UINT64_MAX / 1000000) goto varoverflowerr;
                        usecs = tv.tv_sec * 1000000;

                        // check microseconds on possible overflow before adding & return error when overflow got.
                        if (UINT64_MAX - usecs < tv.tv_usec) goto varoverflowerr;
                        else usecs += tv.tv_usec;

                        usecs /= 1000;
                        if (usecs > UINT32_MAX) goto varoverflowerr;
                        millis = (uint32_t)usecs;
                    #endif

                    __filloutopt(&millis, sizeof(millis), optval, optsize);
                    return NError_Success;
                }

                default:
                    break;
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

                default:
                    break;
            }
            break;

        case SocketOptionLevel_IP:
            {
                switch (optname)
                {
                    case SocketOptionName_IP_TimeToLive:
                        goto handleuint8;

                    default:
                        break;
                }
            }
            break;

        default:
            return NError_IncorrectArgumentValue;
    }

    return NError_UnsupportedProtocolOption;

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
        size_t value_realsize;

        // =============================================================================

        handlebool:
            if ((err = __getsockopt(socket->desc, level, optname, &val_dwint, sizeof(val_dwint))) != NError_Success) return err;
            value.boolean = val_dwint;
            value_realsize = sizeof(value.boolean);
        goto filloptval;
        
        handleint:
            if ((err = __getsockopt(socket->desc, level, optname, &val_dwint, sizeof(val_dwint))) != NError_Success) return err;
            
            #ifdef LIBSOCKET_OS_WINDOWS
                if (val_dwint > INT_MAX) goto varoverflowerr;
            #endif

            value.integer = val_dwint;
            value_realsize = sizeof(value.integer);
        goto filloptval;
            
        handleuint8:
            if ((err = __getsockopt(socket->desc, level, optname, &val_dwint, sizeof(val_dwint))) != NError_Success) return err;

            if (val_dwint > UINT8_MAX) goto varoverflowerr;
            #ifndef LIBSOCKET_OS_WINDOWS
                if (val_dwint < 0) goto varoverflowerr;
            #endif
            value.uint8 = val_dwint;

            value_realsize = sizeof(value.uint8);
        goto filloptval;

        // =============================================================================
        
        filloptval:
            __filloutopt(&value, value_realsize, optval, optsize);
        return NError_Success;

        // =============================================================================
        
        varoverflowerr:
            alert("Got internal size overflow in socket_getopt with params: level=%i, optname=%i.", level, optname);
        return NError_InternalVariableOverflow;
    }
}

static NError __getsockopt(SOCKETDESCRIPTOR desc, int level, int optname, void *optval, socklen_t optlen)
{
    socklen_t realoptlen = optlen;
    if (getsockopt(desc, level, optname, optval, &realoptlen)) return GETLASTTRANSLATEDSYSERR();
    if (realoptlen != optlen)
    {
        alert("Got internal size mismatch in __getsockopt with params: level=%i, optname=%i.", level, optname);
        return NError_InternalSizeMismatch;
    }

    return NError_Success;
}

static inline void __filloutopt(const void *value, size_t size, void *optval, size_t *optsize)
{
    if (*optsize > 0) memcpy(optval, value, (*optsize > size) ? size : *optsize);
    *optsize = size;
}
