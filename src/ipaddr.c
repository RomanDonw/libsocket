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
    ENSURE_INIT;

    int ret = inet_pton(af, straddr, addr);
    if (ret == 0) SETLASTERROR(SOCKERR_PARSEADDRFAIL);
    return ret == 1;
}

bool socket_addrtostr(const IPAddressInterface *addr, SocketAddressFamily af, char *straddr, socklen_t size)
{
    ENSURE_INIT;

    bool res = inet_ntop(af, addr, straddr, size);
    #ifdef LIBSOCKET_OS_WINDOWS
        if (!res && GETLASTERROR() == SOCKERR_INVAL) SETLASTERROR(SOCKERR_NOSPC);
    #endif
    return res;
}