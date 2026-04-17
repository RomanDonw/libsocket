#ifndef UTIL_H
#define UTIL_H

#include "libsocket.h"

#ifndef OS_WINDOWS
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

int fillsockaddrstruct(struct sockaddr *out_sockaddr, SocketAddressFamily af, const char *addr, unsigned short port);

#endif