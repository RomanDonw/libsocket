#include "libsocket.h"

#include <stdlib.h>
#include <string.h>

#include "init.h"
#include "err.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

bool socket_parseaddr(IPAddressInterface *addr, SocketAddressFamily af, const char *straddr)
{
    ENSURE_INIT(false);
    int ret = inet_pton(af, straddr, addr);
    if (ret == 0) RETURNWITHERROR(SocketError_ParsingAddressFailed, false);
    if (ret == -1) RETURNWITHSYSERR(false);
    RETURNWITHSUCCESS(true);
}

bool socket_addrtostr(const IPAddressInterface *addr, SocketAddressFamily af, char *straddr, socklen_t size)
{
    ENSURE_INIT(false);

    if (!inet_ntop(af, addr, straddr, size))
    {
        int err = GETLASTERROR();
        #ifdef LIBSOCKET_OS_WINDOWS
            if (err == SOCKERR_INVAL) socket_lasterror = SocketError_NoSpaceLeft;
            else socket_lasterror = translateerror(err);
        #else
            socket_lasterror = translateerror(err);
        #endif
        return false;
    }

    RETURNWITHSUCCESS(true);
}