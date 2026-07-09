/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"

#include <stdlib.h>
#include <string.h>

#include "init.h"
#include "err.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

NError socket_parseipaddr(IPAddressInterface *addr, SocketAddressFamily af, const char *straddr)
{
    ENSURE_INIT;
    int ret = inet_pton(af, straddr, addr);
    if (ret == 0) return NError_ParsingAddressFailed;
    if (ret == -1) return GETLASTTRANSLATEDSYSERR();
    return NError_Success;
}

NError socket_ipaddrtostr(const IPAddressInterface *addr, SocketAddressFamily af, char *straddr, socklen_t size)
{
    ENSURE_INIT;

    if (!inet_ntop(af, addr, straddr, size))
    {
        int err = GETLASTERROR();
        #ifdef LIBSOCKET_OS_WINDOWS
            if (err == SOCKERR_INVAL) return NError_NoSpaceLeft;
        #endif
            return translateerror(err);
    }

    return NError_Success;
}