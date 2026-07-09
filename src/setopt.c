/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "optfunc.h"

NError socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, size_t optsize)
{
    ENSURE_INIT;

    if (!optsize) return NError_IncorrectArgumentValue;
    if (!optval) return NError_Fault;

    switch (level)
    {
        case SocketOptionLevel_Socket:
            switch (optname)
            {
                // readonly options.
                case SocketOptionName_Socket_AcceptConnections:
                case SocketOptionName_Socket_InternalError:
                case SocketOptionName_Socket_Type:
                    return NError_UnsupportedOperation;

                case SocketOptionName_Socket_KeepAliveConnection:
                case SocketOptionName_Socket_Broadcast:
                    goto handle_bool;

                case SocketOptionName_Socket_RecvBufferSize:
                case SocketOptionName_Socket_SendBufferSize:
                    goto handle_onlyposint;

                case SocketOptionName_Socket_Linger:
                {
                    if (optsize < sizeof(SocketLingerOptions)) return NError_IncorrectArgumentValue;

                    const SocketLingerOptions *lingopts = optval;

                    struct linger ling;
                    ling.l_onoff = lingopts->enable;
                    ling.l_linger = lingopts->linger;
                    if (setsockopt(socket->desc, level, optname, (void *)&ling, sizeof(ling))) return GETLASTTRANSLATEDSYSERR();
                    return NError_Success;
                }

                case SocketOptionName_Socket_RecvTimeout:;
                case SocketOptionName_Socket_SendTimeout:;
                {
                    if (optsize < sizeof(uint32_t)) return NError_IncorrectArgumentValue;

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
                    goto handle_bool;

                case SocketOptionName_TCP_ConnectionKeepIdleTime:
                    goto handle_posorzeroint;

                case SocketOptionName_TCP_KeepAliveProbesInterval:
                case SocketOptionName_TCP_MaxKeepAliveProbes:
                    goto handle_onlyposint;

                default:
                    break;
            }
            break;

        case SocketOptionLevel_IP:
            switch (optname)
            {
                case SocketOptionName_IP_TimeToLive:
                    goto handle_uint8;

                default:
                    break;
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

        // =============================================================================

        handle_bool:
            if (optsize < sizeof(bool)) return NError_IncorrectArgumentValue;
            #ifdef LIBSOCKET_OS_WINDOWS
                val_dwint = (*(bool *)optval) ? TRUE : FALSE;
            #else
                val_dwint = (*(bool *)optval) ? true : false;
            #endif
        goto load_dwint;

        handle_posorzeroint:
            if (optsize < sizeof(int) || *(int *)optval < 0) return NError_IncorrectArgumentValue;
            #ifdef LIBSOCKET_OS_WINDOWS
                val_dwint = *(int *)optval;
                goto load_dwint;
            #else
                goto processopt;
            #endif

        handle_onlyposint:
            if (optsize < sizeof(int) || *(int *)optval <= 0) return NError_IncorrectArgumentValue;
            #ifdef LIBSOCKET_OS_WINDOWS
                val_dwint = *(int *)optval;
                goto load_dwint;
            #else
                goto processopt;
            #endif

        handle_uint8:
            if (optsize < sizeof(uint8_t)) return NError_IncorrectArgumentValue;
            val_dwint = *(uint8_t *)optval;
        goto load_dwint;

        // =============================================================================

        load_dwint:
            optval = &val_dwint;
            optsize = sizeof(val_dwint);
        processopt:
            if (setsockopt(socket->desc, level, optname, optval, optsize)) return GETLASTTRANSLATEDSYSERR();
        return NError_Success;
    }
}
