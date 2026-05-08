#ifndef ERR_H
#define ERR_H

#include "libsocket.h"

#ifdef LIBSOCKET_OS_WINDOWS
    #define GETLASTERROR() (WSAGetLastError())
#else
    #include <errno.h>

    #define GETLASTERROR() (errno)
#endif

SocketError translateerror(int err);

#define RETURNWITHSUCCESS(return_value) { socket_lasterror = Success; return return_value; }
#define RETURNWITHERROR(errorcode, return_value) { socket_lasterror = errorcode; return return_value; }
#define RETURNWITHSYSERR(return_value) { socket_lasterror = translateerror(GETLASTERROR()); return return_value; }

#endif
